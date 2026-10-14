# Minggu 7 — Thread, Shared State, dan Race Condition

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** apa yang berubah ketika dua alur eksekusi mengakses data yang sama?

---

## 1. Mengapa ini penting

Sampai minggu 6, programmu pada dasarnya menjalankan satu alur instruksi sekaligus. Dunia nyata sering
butuh paralelisme: server menangani banyak koneksi, komputasi memakai banyak core. Thread memungkinkan
itu dalam satu process. Tetapi thread dalam process yang sama **berbagi ruang alamat** — dan itulah
sumber kekuatan sekaligus sumber bug paling licik: race condition dan data race. Minggu ini kamu melihat
keduanya sendiri, dan belajar bahwa "program tampak benar" bukan bukti keamanan.

## 2. Learning objectives

Setelah minggu 7, kamu dapat:

1. Menjelaskan beda process dan thread, dan bahwa thread dalam process sama berbagi ruang alamat.
2. Membuat thread dengan `pthread_create`, mengirim argumen, mengambil nilai kembalian, dan `pthread_join`.
3. Menjelaskan perbedaan istilah *race condition* dan *data race*.
4. Menjelaskan bahwa `counter++` bukan operasi otomatis atomik.
5. Menjelaskan bahwa data race di C adalah undefined behavior, bukan sekadar "kadang salah".
6. Menggunakan ThreadSanitizer untuk mendeteksi data race (dengan batas dan kompatibilitasnya).

## 3. Prasyarat

Kamu butuh pemahaman process/virtual address space dari minggu 5
([`week-05-process-virtual-memory.md`](week-05-process-virtual-memory.md)) — khususnya bahwa share
address space berarti thread melihat memory yang sama.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Dua thread masing-masing menambah variabel global 100.000 kali. Hasil akhir tidak pernah 200.000.
Kenapa?

### Analogi: dua kasir satu buku catatan

Dua kasir berbagi satu buku catatan total penjualan. Kasir A membaca total (100), menambah (101),
menulis kembali. Bersamaan, Kasir B membaca total *sebelum* A sempat menulis (juga 100), menambah (101),
menulis. Dua penambahan, tetapi total cuma naik 1. Inilah *interleaving*: urutan eksekusi yang saling
selip.

### Batas analogi

Di toko nyata, kita selesaikan dengan aturan "satu kasir tulis bergiliran". Di program, aturannya adalah
**sinkronisasi** (mutex/atomic) yang akan kita pelajari minggu 8. Analogi "kasir" tidak menangkap bahwa
di mesin, `counter++` sendiri terdiri dari *tiga* langkah mesin (baca, tambah, tulis) yang bisa terpotong
di antara dua thread kapan saja.

### Penjelasan teknis

**Process** memiliki ruang alamat sendiri (minggu 5). **Thread** adalah alur eksekusi dalam satu process;
semua thread berbagi ruang alamat, variabel global, dan heap, tetapi masing-masing punya stack sendiri.
Karena berbagi memory, mereka bisa saling mengganggu.

`counter++` untuk `long` bukan atomik. Secara konseptual: (1) baca `counter` dari memory ke register,
(2) tambah di register, (3) tulis kembali. Jika thread A diinterupsi antara (1) dan (3), dan B menulis
versinya, tulisan A kemudian menimpa hasil B → satu penambahan hilang.

Menurut standar C ([cppreference: memory model](https://en.cppreference.com/w/c/language/memory_model),
merujuk C11 5.1.2.4): "If a data race occurs, the behavior of the program is undefined." **Data race**
adalah istilah teknis: dua akses ke lokasi memory yang sama, salah satunya tulis, tanpa urutan yang
disinkronisasi. Berbeda dari **race condition** yang lebih luas: bug logika yang hasilnya bergantung pada
timing — istilah ini tidak didefinisikan oleh standar C secara presisi (tidak ada sumber primer tunggal
yang dikutip di sini).

### Diagram: interleaving yang salah

```
Thread A:  baca counter(=100)  ...terpotong...         tulis(101)
Thread B:                         baca counter(=100)  tambah 101  tulis(101)
Hasil akhir = 101  (seharusnya 102)
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-07/01_pthread_basic.c`](../labs/week-07/01_pthread_basic.c)

```c
typedef struct { int id; long iter; } arg_t;

static void *pekerja(void *v) {
    arg_t *a = (arg_t *)v;
    long total = 0;
    for (long i = 0; i < a->iter; i++) total += i;
    printf("thread %d selesai, jumlah = %ld\n", a->id, total);
    return NULL;
}

int main(void) {
    pthread_t t;
    arg_t *a = malloc(sizeof *a);   /* argumen di heap agar lifetime cukup */
    a->id = 1; a->iter = 1000;
    if (pthread_create(&t, NULL, pekerja, a) != 0) { /* <- periksa return code! */
        perror("pthread_create"); free(a); return 1;
    }
    pthread_join(t, NULL);          /* tunggu selesai */
    free(a);                        /* baru bebaskan argumen */
    return 0;
}
```

Dua poin krusial:

**(1) Periksa return code `pthread_create`.** Berbeda dengan fungsi libc seperti `open`, pthread
**mengembalikan error number**, bukan mengatur `errno`
([POSIX pthread_create](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_create.html)).
Sukses = 0; gagal = `EAGAIN` (kehabisan resource/thread), `EINVAL` (attr salah), `EPERM` (jadwal). Jangan
cek `errno` untuk fungsi pthread.

**(2) Lifetime argumen.** Kita alokasikan `arg_t` di heap agar tetap valid selama thread berjalan, dan
baru membebaskannya setelah `pthread_join`. Mengirim alamat variabel lokal `main` terkadang aman karena
`main` hidup sampai `join`, tetapi mengirim alamat variabel lokal *di dalam fungsi lain* yang sudah
kembali adalah bug (lihat dangling pointer minggu 2). Kebiasaan heap + free setelah join menghindari
kebingungan.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-07
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 01_pthread_basic 01_pthread_basic.c
./01_pthread_basic
```

### Hasil yang diharapkan

```
thread 1 selesai, jumlah = 499500
```

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — veri aman tanpa shared state (10 menit)

File [`03_private_accumulate.c`](../labs/week-07/03_private_accumulate.c) membuat tiap thread
mengakumulasi ke buffer privat lalu mengembalikannya; `main` menjumlahkan setelah `join`.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 03_private 03_private_accumulate.c
./03_private
```

Hasil: `total = 200000 (harusnya 200000)`. Tanpa shared state, tidak perlu sinkronisasi.

### Eksperimen 2 — race yang sesungguhnya (15 menit)

File `02_race_basic.c` adalah **contoh rusak**: dua thread menambah `counter` global tanpa
sinkronisasi.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 02_race 02_race_basic.c
./02_race        # jalankan beberapa kali
```

Hasil di mesin penulis (dua run):

```
counter = 111127  (kalau aman, harusnya 200000)
counter = 111239  (kalau aman, harusnya 200000)
```

Angkanya berubah-ubah dan selalu kurang dari 200000. Ini data race. **Penting:** bisa jadi suatu run
menghasilkan tepat 200000 — itu *bukan* bukti aman.

### Eksperimen 3 — menangkap race dengan ThreadSanitizer (15 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fsanitize=thread -pthread -o 02_race_tsan 02_race_basic.c
./02_race_tsan
```

TSan melaporkan (cuplikan di mesin penulis):

```
WARNING: ThreadSanitizer: data race (pid=103718)
  Read of size 8 at 0x... by thread T2:
    #0 inc .../02_race_basic.c:24
  Previous write of size 8 at 0x... by thread T1:
    #0 inc .../02_race_basic.c:24
```

TSan mendeteksi data race saat runtime. Batasnya: butuh flag `-fsanitize=thread` saat kompilasi,
berjalan lebih lambat, dan (menurut dokumentasi GCC/Clang) umumnya tidak digabung dengan AddressSanitizer
dalam satu build — klaim penggabungan ini tidak diverifikasi dari sumber primer, jadi jangan asumsikan.
Lihat [GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) dan
[Clang ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html).

### Eksperimen 4 — interleaving pada output (10 menit)

File `04_race_print_broken.c` mencetak dari dua thread tanpa sinkronisasi. Jalankan beberapa kali; dua
baris output sering tumpang-tindih. Itu menunjukkan `printf` sendiri bukan operasi atomik lintas thread.

---

## 7. Kesalahan umum

- **Mengira `counter++` atomik.** Ia bestari tiga langkah mesin; butuh mutex atau atomic untuk aman.
- **Menganggap satu run benar = aman.** Race tidak selalu gagal. Uji dengan TSan, bukan dengan mata.
- **Mengecek `errno` untuk pthread.** Pakai return code fungsi itu sendiri.
- **Mengirim alamat variabel lokal yang bisa mati.** Gunakan heap + free setelah join, atau nilai hasil.
- **Menjanjikan output tertentu dari contoh tanpa sinkronisasi.** Jangan; sebutkan hasilnya tidak deterministik.

---

## 8. Latihan

### Mudah
Modifikasi `01_pthread_basic.c` agar membuat 3 thread berbeda `id`, masing-masing mencetak id-nya.
Pastikan semua di-join.

### Menengah
Buat 4 thread, tiap thread menambah `counter` global 50.000 kali *tanpa* sinkronisasi. Jalankan 5 kali
biasa dan 5 kali di bawah TSan. Catat bahwa hasil tidak konsisten dan TSan bersikap konsisten
(memvalidasi ada race).

### Tantangan
Buktikan bahwa "akumulasi privat lalu gabung" (seperti `03_private_accumulate.c`) selalu benar tanpa
mutex. Jelaskan mengapa tidak ada shared write saat thread berjalan. Petunjuk bertahap: (1) tiap thread
`malloc` buffer sendiri; (2) tidak ada variabel global yang ditulis; (3) gabungkan setelah `join`;
(4) simpulkan tidak ada data race karena tidak ada akses bersama saat berjalan.

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan beda process dan thread, dan bahwa thread berbagi ruang alamat.
- [ ] Saya tahu `pthread_create` mengembalikan error number, bukan errno.
- [ ] Saya bisa menjelaskan mengapa `counter++` dari dua thread tidak aman.
- [ ] Saya tahu data race = undefined behavior di C.
- [ ] Saya tidak lagi menganggap satu run yang benar sebagai bukti keamanan.
- [ ] Saya bisa menjelaskan beda race condition dan data race.

---

## 10. Ringkasan

Thread berbagi ruang alamat process, sehingga akses bersama bisa terjadi race. `counter++` bukan atomik;
tanpa sinkronisasi hasilnya tidak deterministik dan merupakan data race (undefined behavior di C).
ThreadSanitizer mendeteksi race saat runtime — alat wajib saat belajar paralelisme. "Tampak benar" bukan
bukti aman; uji dengan alat, bukan dengan kebetulan.

**Glosarium mini:** thread, shared state, interleaving, race condition, data race, critical section,
ThreadSanitizer. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [pthread_create — POSIX Issue 8](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_create.html) — return error number, bukan errno.
- [Memory model — cppreference](https://en.cppreference.com/w/c/language/memory_model) — data race ⇒ undefined behavior (C11 5.1.2.4).
- [Clang ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) — deteksi data race.
- [The Linux Programming Interface — TLPI](https://man7.org/tlpi/) — ch. 29–30 (Threads).

## 12. Jembatan ke minggu 8

Kamu sekarang tahu mengapa shared state berbahaya. Minggu 8 menjawab: **bagaimana membuatnya aman?**
Kita pakai mutex untuk melindungi critical section, pelajari deadlock dan lock ordering, kenalan dengan
atomic operation sebagai alternatif untuk kasus sederhana, dan merancang bounded queue sebagai persiapan
proyek akhir. Lanjut:
[`week-08-mutex-atomics.md`](week-08-mutex-atomics.md)
