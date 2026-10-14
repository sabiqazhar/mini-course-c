# Minggu 12 — Proyek Akhir: Antrean Berbatas Aman-Thread (Bounded Thread-Safe Queue)

**Estimasi durasi:** 8–10 jam (baca ±2 jam, eksperimen ±4 jam, latihan ±3 jam)
**Pertanyaan utama minggu ini:** bagaimana menyatukan memory, heap, thread, mutex, dan struktur data menjadi satu komponen yang benar?

---

## 1. Mengapa ini penting

Ini adalah minggu terakhir, dan ia sengaja dibuat sebagai *capstone*: satu proyek kecil yang memaksa kamu memakai hampir semua yang sudah dipelajari. Setelah minggu ini, kamu punya sebuah **antrean (queue) berbatas yang aman untuk multithread** — komponen yang sungguh-sungguh ada di dunia nyata: antrean request di web server, antrean task di thread pool, antrean log antara produser dan penulis file.

Kalau kamu bisa membangun dan membuktikan komponen ini benar, maka kamu sudah melewati rintangan terbesar dalam *systems engineering*: menjahit banyak konsep low-level tanpa memperkenalkan *race condition*, *deadlock*, atau kebocoran memori.

Minggu ini tidak mengenalkan konsep baru. Ia mengikat konsep lama: ring buffer (M2, M9), alokasi + kegagalan (M3, M4), thread (M7), mutex & kondisi (M8), dan ownership/invarian (M9). Itulah sebabnya ini adalah ujian sekaligus fondasi.

## 2. Learning objectives

Setelah minggu 12, kamu dapat:

1. Menjelaskan mengapa antrean berbatas butuh *satu mutex* yang melindungi **semua** state, bukan per-field lock.
2. Menggunakan *dua* condition variable (`not_empty`, `not_full`) agar produser dan konsumer memblokir dengan benar tanpa *busy-wait*.
3. Menjelaskan mengapa `pthread_cond_wait` harus dipanggil di dalam `while`, bukan `if` (spurious wakeup).
4. Merancang *shutdown* yang deterministik lewat *poison-pill flag* + `pthread_cond_broadcast`, sehingga tidak ada thread yang menggantung.
5. Menangani kegagalan `init` (alokasi / primitif gagal) tanpa meninggalkan resource bocor.
6. Membuktikan kebenaran program dengan AddressSanitizer, UBSan, dan ThreadSanitizer.

## 3. Prasyarat

Kamu butuh seluruh minggu 1–11. Yang paling langsung dipakai:

- Ring buffer & invarian (M9): queue kita adalah ring buffer dengan `head`/`tail`/`count`.
- Mutex & race (M7, M8): inilah alat untuk membuat akses aman.
- Kondisi & spurious wakeup (M8): kunci agar produser/konsumer menunggu dengan benar.
- Heap & kegagalan alokasi (M3, M4): buffer internal dialokasikan di `init`.

Semua kode ada di [`labs/week-12/`](../labs/week-12/): `tsqueue.h`, `tsqueue.c`, `test_basic.c`, `test_mp_mc.c`, `Makefile`, dan `README.md`.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Bayangkan sebuah *thread pool*: beberapa *worker thread* (konsumer) mengambil tugas dari satu antrean, sementara *request handler* (produser) memasukkan tugas ke antrean itu. Tiga kebutuhan muncul:

1. **Aman untuk banyak thread.** Produser dan konsumer berlari bersamaan. Tanpa sinkronisasi, `count++` atau `tail = (tail+1)%cap` adalah *data race* (M7).
2. **Berbatas (bounded).** Kalau produser lebih cepat dari konsumer, antrean tak boleh tumbuh tak terbatas sampai *out-of-memory*. Ia harus menampung paling banyak `cap` item; di luar itu, produser *menunggu*.
3. **Shutdown yang bersih.** Saat program dimatikan, semua tugas yang sudah masuk harus selesai diproses, dan tidak ada worker yang menggantung menunggu tugas yang tak akan datang.

### Analogi: loket dan ruang tunggu berbatas

- **Ruangan tunggu** berkapasitas `cap` kursi = buffer kita.
- **Orang antre** = item (disimpan *by value*, kita salin datanya).
- **Satpam** = mutex, menjaga agar hanya satu orang yang bergerak di pintu pada satu waktu.
- **Lampu "ada kursi kosong"** = kondisi `not_full` (produser menunggu lampu ini).
- **Lampu "ada orang"** = kondisi `not_empty` (konsumer menunggu lampu ini).
- **Pengumuman tutup** = flag `shutdown` + broadcast (semua yang menunggu dibangunkan sekaligus).

Yang penting: produser yang melihat ruangan penuh **duduk menunggu lampu**, bukan mondar-mandir mengecek terus (itu *busy-wait*, membuang CPU). Begitu seorang konsumer mengambil orang, ia menyalakan lampu "ada kursi kosong" dan satu produser terbangun.

### Penjelasan teknis

Kita melindungi **semua** state (`buf`, `cap`, `head`, `tail`, `count`, `shutdown`) dengan **satu** mutex. Mengapa tidak satu lock per field? Karena banyak operasi butuh melihat beberapa field sekaligus (misal: "apakah `count < cap`?" sebelum menulis). Satu mutex menyederhanakan dan sudah cukup untuk ukuran ini.

Kita pakai **dua** condition variable agar sinyalnya tepat sasaran:
- `not_empty`: di-signal oleh produser setelah menaruh item, dibangunkan oleh konsumer.
- `not_full`: di-signal oleh konsumer setelah mengambil item, dibangunkan oleh produser.

Mengapa `while`, bukan `if`, untuk mengecek kondisi sebelum `pthread_cond_wait`? Karena `pthread_cond_wait` bisa terbangun tanpa sinyal nyata (*spurious wakeup*), atau terbangun karena sinyal lain namun kondisi belum terpenuhi. Maka kondisi **harus** dites ulang di `while`. Ini pelajaran M8 yang paling sering dilanggar.

### Diagram state

```
 produser ──enqueue──▶ [ mutex ] ──▶ ring buffer (cap slot)
                            │
 konsumer ──dequeue─▶ [ mutex ] ◀── not_empty / not_full (cond var)
                            │
                   shutdown flag (poison pill) ──▶ broadcast ke semua
```

---

## 5. Contoh kode minimal dan penjelasannya

API lengkap ada di [`labs/week-12/tsqueue.h`](../labs/week-12/tsqueue.h). Berikut intinya:

```c
typedef struct {
    int             *buf;        /* buffer internal, milik queue (dialokasikan di init) */
    size_t          cap;         /* kapasitas: banyak slot */
    size_t          head;        /* indeks dequeue berikutnya */
    size_t          tail;        /* indeks enqueue berikutnya */
    size_t          count;       /* banyak item valid saat ini */
    pthread_mutex_t mtx;         /* SATU mutex melindungi semua field di atas */
    pthread_cond_t  not_empty;   /* sinyal ke konsumer: ada item baru */
    pthread_cond_t  not_full;    /* sinyal ke produser: ada slot kosong */
    int             shutdown;    /* poison-pill flag; 1 => stop terima item baru */
} TSQueue;

int  tsqueue_init(TSQueue *q, size_t cap);
int  tsqueue_enqueue(TSQueue *q, int v);
int  tsqueue_dequeue(TSQueue *q, int *out);
void tsqueue_shutdown(TSQueue *q);
void tsqueue_destroy(TSQueue *q);
```

Fungsi inti — perhatikan `while` dan urutan *signal-vs-unlock*:

```c
int tsqueue_enqueue(TSQueue *q, int v)
{
    pthread_mutex_lock(&q->mtx);

    while (q->count == q->cap && !q->shutdown)          /* while, bukan if */
        pthread_cond_wait(&q->not_full, &q->mtx);

    if (q->shutdown) {                                  /* queue penuh & tutup -> tolak */
        pthread_mutex_unlock(&q->mtx);
        return -1;
    }

    q->buf[q->tail] = v;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;

    pthread_cond_signal(&q->not_empty);                 /* bangunkan SATU konsumer */
    pthread_mutex_unlock(&q->mtx);
    return 0;
}

int tsqueue_dequeue(TSQueue *q, int *out)
{
    pthread_mutex_lock(&q->mtx);

    while (q->count == 0 && !q->shutdown)               /* while, bukan if */
        pthread_cond_wait(&q->not_empty, &q->mtx);

    if (q->count == 0) {                                /* shutdown & kosong -> keluar */
        pthread_mutex_unlock(&q->mtx);
        return -1;
    }

    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;

    pthread_cond_signal(&q->not_full);                  /* bangunkan SATU produser */
    pthread_mutex_unlock(&q->mtx);
    return 0;
}
```

**Urutan signal vs unlock.** Kita `signal` *sebelum* `unlock`. Itu sah dan sering sedikit lebih efisien (thread yang dibangunkan tidak langsung rebut mutex yang masih dipegang). Boleh juga dibalik; keduanya benar. Yang tidak boleh: mengakses `q` *setelah* unlock.

**Shutdown deterministik.** `tsqueue_shutdown` mengunci, menyalakan flag, lalu `broadcast` ke **kedua** kondisi agar semua thread yang menunggu terbangun sekaligus:

```c
void tsqueue_shutdown(TSQueue *q)
{
    pthread_mutex_lock(&q->mtx);
    q->shutdown = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mtx);
}
```

Produser yang sedang menunggu slot (`not_full`) terbangun, lihat `shutdown` aktif, lalu kembalikan `-1` (item tidak jadi ditambah). Konsumer yang sedang menunggu item (`not_empty`) terbangun, kalau masih ada item ia ambil dulu, kalau sudah kosong ia kembalikan `-1` dan keluar. Hasil akhir selalu konsisten.

**Penanganan kegagalan `init`.** [`tsqueue.c`](../labs/week-12/tsqueue.c) memeriksa `cap == 0` dan overflow (`cap > SIZE_MAX / sizeof(int)`), lalu tiap langkah `malloc`/`pthread_*_init` yang gagal membersihkan apa yang sudah dibuat agar **tidak bocor**. Ini langsung mempraktikkan kontrak ownership dari M3/M4.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-12
make test     # build + jalankan kedua tes (FIFO benar & "cocok = YA")
make asan     # AddressSanitizer + UBSan (cek leak / UB)
make tsan     # ThreadSanitizer (cek data race)
```

### Hasil yang diharapkan

`make test` (di mesin penulis, Ubuntu 24.04, GCC 13.3, x86-64):

```
enqueue 10 -> 0  (count=1, head=0, tail=1)
enqueue 20 -> 0  (count=2, head=0, tail=2)
enqueue 30 -> 0  (count=3, head=0, tail=0)

dequeue -> 10  (rc=0, count=2, head=1, tail=0)
enqueue 40 -> 0  (count=3, head=1, tail=1)
isi buffer mentah (bukan urutan FIFO): buf[0]=40 buf[1]=20 buf[2]=30

drain sisa (harus FIFO: 20, 30, 40):
dequeue -> 20
dequeue -> 30
dequeue -> 40

selesai. tidak ada leak (cek dengan -fsanitize=address).

diproduksi = 40000, dikonsumsi = 40000, cocok = YA
```

Perhatikan `buf[]` mentahnya **tidak** berurutan (40, 20, 30) tetapi *dequeue* tetap mengeluarkan 20, 30, 40 — bukti ring buffer menyimpan melingkar namun retrieval tetap FIFO. Bagian `diproduksi = 40000, dikonsumsi = 40000` berasal dari tes MPMC (4 produser × 10.000 item, 4 konsumer) yang berjalan bersamaan. `make asan` dan `make tsan` selesai **tanpa satu pun laporan** (tidak ada leak, tidak ada UB, tidak ada data race).

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — lihat ring buffer memutar (10 menit)

```bash
cd labs/week-12
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o tb tsqueue.c test_basic.c
./tb
```

Amati `tail` setelah `enqueue 30` kembali ke `0` (karena `3 % 3 == 0`). Coba ubah kapasitas di `test_basic.c` menjadi `4` dan `5`, jalankan lagi, dan pastikan FIFO tetap benar saat `tail` memutar di tengah.

### Eksperimen 2 — buktikan tidak ada data race (15 menit)

```bash
make tsan
```

Jalankan beberapa kali. ThreadSanitizer tidak akan mencetak apa-apa dan program tetap `cocok = YA`. Sekarang, **untuk belajar**, hapus `pthread_mutex_lock`/`unlock` di `tsqueue_enqueue` (biarkan `count++` tanpa proteksi) lalu jalankan `make tsan` lagi. TSan akan melaporkan *data race* pada `count`. Kembalikan kode ke semula. Ini memberimu "rasa" nyata mengapa minggu 7–8 penting.

### Eksperimen 3 — buktikan tidak ada leak (10 menit)

```bash
make asan
```

ASan/UBSan selesai tanpa "LeakSanitizer" dan tanpa "runtime error". Coba lupakan `tsqueue_destroy(&q)` di `test_mp_mc.c`, jalankan `make asan`, dan lihat LeakSanitizer melaporkan buffer yang bocor. Kembalikan.

### Eksperimen 4 — ubah `while` jadi `if` dan amati (15 menit)

Ganti `while (q->count == q->cap && !q->shutdown)` menjadi `if (...)`. Jalankan `make tsan` berulang. Pada beban ringan mungkin tampak "aman", tetapi di bawah TSan dengan 4 produser/4 konsumer, *spurious wakeup* atau urutan sinyal bisa membuat thread menulis ke buffer saat sebenarnya penuh — korupsi data atau crash. Kembalikan ke `while`; itulah aturan emas kondisi variabel.

---

## 7. Kesalahan umum dan cara mendiagnosisnya

**Memakai `if` bukan `while` di `cond_wait`.**
Gejala: kadang benar, kadang korupsi/panic di bawah beban. Selalu `while`. TSan + beban tinggi akan mengeksposnya.

**Lupa `broadcast` saat shutdown, hanya `signal`.**
Gejala: program "hang" di akhir — ada konsumer yang masih menunggu `not_empty` padahal queue sudah kosong dan tutup. `tsqueue_shutdown` harus `broadcast` ke **kedua** kondisi.

**Memanggil fungsi ber-efek-samping sebagai argumen `printf` bersama field yang diubahnya.**
Gejala: nilai yang tercetak "ketinggalan satu" (misal `count` tampak 0 setelah enqueue pertama). Penyebabnya: urutan evaluasi argumen di C tidak ditentukan, sehingga `q.count` bisa dibaca *sebelum* `tsqueue_enqueue` berjalan. Pisahkan ke pernyataan sendiri: `rc = tsqueue_enqueue(...); printf(..., rc, q.count);`. Ini persis bug yang kami temukan dan perbaiki saat menulis tes dasar minggu ini.

**Menunggu queue kosong di thread tunggal (single-thread).**
Gejala: *deadlock* — `dequeue` memanggil `pthread_cond_wait` dan tak pernah terbangun karena tak ada produser lain. Di tes single-thread, *drain* sejumlah item yang kamu tahu persis ada, jangan `while (dequeue()==0)`.

**Mengira `signal` membangunkan semua.**
`pthread_cond_signal` membangunkan **satu** thread; `pthread_cond_broadcast` membangunkan **semua**. Untuk shutdown pakai `broadcast`. Untuk operasi biasa, `signal` cukup (dan lebih efisien).

**Membersihkan sebagian saat `init` gagal.**
Gejala: LeakSanitizer melaporkan kebocoran bila `malloc` sukses tapi `pthread_mutex_init` gagal. Pastikan tiap cabang kegagalan membebaskan yang sudah dialokasikan.

**Menyalakan `shutdown` sebelum produser selesai.**
Gejala: item hilang (`dikonsumsi < diproduksi`). Shutdown harus dipanggil **setelah** semua produser di-join, seperti di `test_mp_mc.c`.

---

## 8. Latihan

### Mudah — hitung sisa dengan aman

Tulis fungsi `size_t tsqueue_size(TSQueue *q)` yang mengunci mutex, membaca `count`, membuka, lalu mengembalikannya. Ingat: nilai yang dikembalikan hanya *snapshot*; jangan mengandalkannya untuk keputusan concurrency tanpa mutex.

### Menengah — batas waktu (timed wait)

Tambahkan `tsqueue_enqueue_timed` dan `tsqueue_dequeue_timed` yang menerima `struct timespec` dan memakai `pthread_cond_timedwait`. Kembalikan kode khusus (misal `-2`) bila waktu habis sebelum kondisi terpenuhi. Uji dengan produser lambat.

### Tantangan — queue generik lewat `void *`

Ubah penyimpanan dari `int` menjadi `void *` agar queue membawa pointer ke objek sembarang. Dokumentasikan kontrak ownership: **siapa** yang bertanggung jawab `free` objek yang sudah di-dequeue? (Saran: produser tetap pemilik; queue hanya menyalurkan pointer. Atau queue mengambil alih dan memanggil *callback* destroy.) Jalankan `make asan` dan `make tsan` untuk membuktikan tidak ada use-after-free.

Solusi referensi lengkap ada di [`solutions/week-12/`](../solutions/week-12/). Coba dulu minimal 30 menit sebelum membukanya.

---

## 9. Checklist pemahaman (self-assessment)

Centang hanya kalau kamu bisa menjelaskannya tanpa melihat materi:

- [ ] Saya bisa menjelaskan mengapa kita pakai **satu** mutex untuk semua field, bukan per-field.
- [ ] Saya tahu perbedaan `not_empty` dan `not_full`, dan kapan masing-masing di-signal.
- [ ] Saya bisa menjelaskan mengapa `cond_wait` harus di `while`, dengan menyebut *spurious wakeup*.
- [ ] Saya bisa menjelaskan alur `shutdown`: flag + `broadcast` + cara konsumer/produser keluar.
- [ ] Saya tahu mengapa `init` yang gagal di tengah jalan harus membebaskan resource yang sudah dibuat.
- [ ] Saya pernah menjalankan `make asan` dan `make tsan` dan mengerti bahwa "tanpa laporan" = lulus.
- [ ] Saya bisa menjelaskan mengapa dequeue di thread tunggal tidak boleh menunggu sampai kosong.

Kalau ada satu saja yang belum, ulangi Eksperimen 2 dan 4. Itu langsung memperlihatkan konsekuensi nyatanya.

---

## 10. Ringkasan

Antrean berbatas aman-thread = ring buffer (M2/M9) + satu mutex yang melindungi seluruh state + dua condition variable (`not_empty`, `not_full`) agar produser dan konsumer memblokir tanpa *busy-wait*. `cond_wait` selalu di `while` (spurious wakeup). Shutdown deterministik dicapai dengan flag *poison-pill* + `pthread_cond_broadcast` ke semua thread yang menunggu, dipanggil **setelah** produser di-join. `init` yang gagal di tengah jalan membebaskan resource agar tak bocor. Kebenaran dibuktikan, bukan diyakini: ASan/UBSan (tanpa leak/UB) dan TSan (tanpa data race) keduanya lulus pada tes MPMC 40.000 item.

**Glosarium mini minggu ini:** mutex, condition variable, spurious wakeup, broadcast, poison pill, bounded queue, deterministic shutdown. Penjelasan lengkap ada di [`glossary.md`](../glossary.md).

## 11. Referensi minggu ini

- [pthread_cond_wait(3) — man7.org](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html) — semantik `while` + spurious wakeup, dan perbedaan `signal` vs `broadcast`.
- [pthread_mutex_lock(3) — man7.org](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html) — kontrak mutex dan urutan lock/unlock.
- [Beej's Guide to C Programming](https://beej.us/guide/bgc/) — pengantar ramah untuk struct, heap, dan dasar-dasar di atas.
- [The Linux Programming Interface (TLPI), M. Kerrisk](https://man7.org/tlpi/) — bab threads, mutex, dan condition variable (rujukan mendalam).
- [ThreadSanitizer — LLVM/Google](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual) — cara kerja deteksi data race.

## 12. Jembatan ke langkah berikutnya (systems engineering)

Selamat — dua belas minggu sudah kamu tempuh. Kamu sekarang punya fondasi nyata: model memory (M1), array/struct/lifetime (M2), heap (M3–M4), virtual memory & proses (M5–M6), system call & `mmap` (M6), thread & race (M7), mutex/atomik (M8), struktur data & ownership (M9), allocator (M10), dan debugging (M11), berpuncak pada antrean aman-thread ini.

Dari sini, untuk menuju *systems engineering* sungguhan, arah yang wajar:

- **Non-blocking I/O & event loop** (`epoll`/`kqueue`, `select`): bagaimana satu thread melayani ribuan koneksi tanpa satu-thread-per-koneksi.
- **Lock-free & wait-free** structures: mengganti mutex dengan operasi atomik untuk mengurangi kontensi (lanjut dari M8).
- **Jaringan**: socket, protokol, *back-pressure* — antrean berbatas ini persis mekanisme *back-pressure* paling dasar.
- **Concurrency model**: bagaimana bahasa/runner lain (Go goroutine, Rust async) menyembunyikan sebagian dari apa yang kamu tulis manual di sini.

Yang paling penting: kebiasaan yang sudah terbentuk — verifikasi dengan sanitizer, pisahkan level abstraksi (C vs POSIX vs Linux vs compiler), dan jangan percaya "sudah saya coba, aman kok" — itulah inti berpikir sistem yang andal.

Kembali ke [`README.md`](../README.md) untuk peta kursus, atau lihat [`plan.md`](../plan.md) untuk checkpoint dan langkah berikutnya.
