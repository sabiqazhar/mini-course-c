# Minggu 8 — Mutex, Critical Section, Deadlock, Atomic, dan Bounded Queue

**Estimasi durasi:** 6–7 jam
**Pertanyaan utama minggu ini:** bagaimana beberapa thread bekerja bersama tanpa merusak invariant data?

---

## 1. Mengapa ini penting

Minggu 7 menunjukkan bahwa shared state tanpa perlindungan menghasilkan data race (undefined behavior).
Minggu ini memberimu alat utama untuk memperbaikinya: **mutex** (mutual exclusion) untuk melindungi
critical section, dan **atomic operation** untuk kasus satu operasi sederhana. Kamu juga akan belajar
tentang deadlock — jebakan sinkronisasi yang tidak akan terlihat sampai production. Di akhir minggu,
kamu merancang bounded queue, fondasi langsung untuk proyek akhir minggu 12.

## 2. Learning objectives

Setelah minggu 8, kamu dapat:

1. Menjelaskan mutex, lock/unlock, dan critical section.
2. Menggunakan `PTHREAD_MUTEX_INITIALIZER` atau inisialisasi eksplisit, serta `pthread_mutex_destroy`.
3. Memeriksa return code fungsi pthread dan tidak menganggap semua error memakai `errno`.
4. Menjelaskan deadlock, lock ordering, dan lock yang tidak dilepas di jalur error.
5. Membedakan mutex (melindungi invariant multi-langkah) dan atomic (satu operasi).
6. Merancang bounded queue dengan sinkronisasi yang benar.

## 3. Prasyarat

Minggu 7 ([`week-07-threads-races.md`](week-07-threads-races.md)) — harus sudah paham data race dan
mengapa `counter++` butuh perlindungan.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Dua thread menambah counter bersama. Bagaimana menjamin hasil akhirnya benar tanpa kehilangan penambahan?

### Analogi: satu pena untuk satu buku

Kembali ke dua kasir satu buku catatan. Solusinya: ada satu pena, dan aturan "hanya boleh menulis kalau
memegang pena". Seorang kasir mengambil pena (lock), membaca-tambah-tulis (critical section), lalu
mengembalikan pena (unlock). Kasir lain harus menunggu giliran. Inilah mutex.

### Batas analogi

Pena nyata tak bisa "dimiliki bersamaan". Mutex di program bisa **deadlock** bila dua thread masing-masing
memegang satu pena dan menunggu pena lawan — analogi "satu pena" tidak menangkap deadlock. Itu sebabnya
kita pelajari lock ordering secara eksplisit.

### Penjelasan teknis

**Mutex** adalah objek sinkronisasi. `pthread_mutex_lock(&m)` menunggu hingga mutex bebas lalu mengunci;
`pthread_mutex_unlock(&m)` melepas. Region antara keduanya adalah **critical section** — hanya satu thread
yang boleh berada di dalamnya sekaligus. Fungsi pthread mengembalikan *error number* (0 = sukses), bukan
`errno` ([POSIX pthread_mutex_lock](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutex_lock.html)).

Tipe mutex menentukan perilaku relock: `PTHREAD_MUTEX_NORMAL` (default di banyak implementasi) → relock =
deadlock; `PTHREAD_MUTEX_ERRORCHECK` → relock mengembalikan `EDEADLK`; `PTHREAD_MUTEX_RECURSIVE` → hitung
lock; `PTHREAD_MUTEX_DEFAULT` adalah yang paling tidak bisa ditebak (bisa seperti salah satu di atas, atau
UB). Default tipe adalah `PTHREAD_MUTEX_DEFAULT`
([pthread_mutexattr_settype](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutexattr_settype.html)).

**Deadlock** terjadi bila thread saling menunggu lock yang dipegang yang lain (cyclical wait). Pencegahan
paling umum: **lock ordering** — selalu kunci mutex dalam urutan alamat/ID yang konsisten di semua thread.

**Atomic** ([cppreference: atomic_fetch_add](https://en.cppreference.com/w/c/atomic/atomic_fetch_add),
sejak C11 via `<stdatomic.h>`): operasi tunggal yang tidak bisa diamati "setengah jalan" oleh thread lain.
`atomic_fetch_add(&c, 1)` menambah secara atomik. Penting: atomic membuat *satu* operasi aman; ia **tidak**
membuat rangkaian beberapa langkah (misal cek-lalu-aksi) menjadi atomik — untuk itu tetap butuh mutex.

### Diagram

```
Tanpa mutex:                Dengan mutex:
 T1: baca(tulis)  --+         T1: lock ── baca/tulis ── unlock
                     |  tabrakan   T2:           (menunggu) lock ── baca/tulis ── unlock
 T2: baca(tulis)  --+
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-08/01_mutex_counter.c`](../labs/week-08/01_mutex_counter.c)

```c
static long counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *inc(void *v) {
    (void)v;
    for (int i = 0; i < N; i++) {
        if (pthread_mutex_lock(&mtx) != 0) return NULL;
        counter++;
        if (pthread_mutex_unlock(&mtx) != 0) return NULL;
    }
    return NULL;
}
```

`PTHREAD_MUTEX_INITIALIZER` menginisialisasi mutex secara statis (cocok untuk variabel global/static).
Untuk mutex dinamis, pakai `pthread_mutex_init(&m, NULL)` dan `pthread_mutex_destroy(&m)` saat selesai
(seperti di `04_bounded_queue.c`). **Lock dan unlock harus berpasangan**; jika suatu jalur error melewatkan
`unlock`, thread lain bisa menggantung selamanya.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-08
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 01_mutex_counter 01_mutex_counter.c
./01_mutex_counter
```

### Hasil yang diharapkan

```
counter = 400000  (harusnya 400000)
```

Sekarang hasilnya konsisten dengan invariant: 4 thread × 100.000 = 400.000.

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — perbaiki counter dengan mutex (10 menit)

Jalankan `01_mutex_counter` beberapa kali. Bandingkan dengan `02_race_basic.c` minggu 7: sekarang selalu
400.000. Itu karena mutex menjamin critical section `counter++` tidak tumpang-tindih.

### Eksperimen 2 — atomic sebagai alternatif (10 menit)

File [`03_atomic_counter.c`](../labs/week-08/03_atomic_counter.c) menggunakan `atomic_long` dan
`atomic_fetch_add`.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 03_atomic_counter 03_atomic_counter.c
./03_atomic_counter
```

Hasil: `counter = 400000`. Untuk sekadar menambah counter, atomic lebih ringan daripada mutex. Catatan:
`ATOMIC_LONG_LOCK_FREE` (0/1/2 = tidak pernah/terkadang/selalu lock-free) memberi tahu apakah atomic itu
benar-benar lock-free di platform ini
([cppreference](https://en.cppreference.com/w/c/atomic/ATOMIC_LOCK_FREE_consts)).

### Eksperimen 3 — deadlock dan lock ordering (15 menit)

File `02_deadlock_broken.c` adalah **contoh rusak**: thread A mengunci `m1` lalu `m2`, thread B mengunci
`m2` lalu `m1` — urutan berlawanan. Jalankan dengan timeout agar tidak menggantung shell:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 02_deadlock 02_deadlock_broken.c
timeout 5 ./02_deadlock
```

Di mesin penulis satu run *selesai normal* (`exit=0`) — itu sendiri pelajaran: deadlock adalah
*kemungkinan*, bukan kepastian setiap run. Penguncian dua mutex tanpa urutan konsisten bisa macet, bisa
juga lulus berkali-kali lalu tiba-tiba menggantung di production. Selalu pakai lock ordering: kunci
selalu dalam urutan alamat pointer atau id yang disepakati.

### Eksperimen 4 — bounded queue (15 menit)

File [`04_bounded_queue.c`](../labs/week-08/04_bounded_queue.c) mengimplementasikan ring buffer berbasis
mutex. `enqueue` mengembalikan `-1` bila penuh; `dequeue` `-1` bila kosong (tanpa memblokir).

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 04_bounded_queue 04_bounded_queue.c
./04_bounded_queue
```

Hasil di mesin penulis:

```
enqueue 10 : 0
enqueue 20 : 0
enqueue 30 : 0
enqueue 40 (penuh?) : -1
dequeue -> 10 (count sekarang)
dequeue -> 20 (count sekarang)
dequeue -> 30 (count sekarang)
```

Perhatikan invariant terjaga: `0 <= count <= capacity`, dan head/tail bergerak melingkar. Versi memblokir
(producer menunggu saat penuh, consumer menunggu saat kosong) adalah proyek minggu 12.

---

## 7. Kesalahan umum

- **Lock/unlock tidak berpasangan.** Jalur error yang lupa `unlock` menyebabkan thread lain hang.
  Pastikan `unlock` selalu tercapai (misal lewat pola di mana `unlock` ada di akhir fungsi).
- **Mengunci mutex berbeda urutan di thread berbeda.** Resep deadlock; tetapkan lock ordering.
- **Mengira atomic bisa menggantikan mutex untuk invariant multi-langkah.** Atomic hanya untuk satu operasi.
- **Mengecek `errno` untuk pthread.** Periksa return code fungsi.
- **Mengunci terlalu luas (coarse-grained).** Mengurangi paralelisme; pelajari batas kritis sesempit perlu.
- **Mengira mutex selalu gratis.** Ada contention; terlalu banyak lock bersaing memperlambat program.

---

## 8. Latihan

### Mudah
Tulis program dengan mutex yang menjaga counter, lalu cetak apakah hasilnya selalu sama setelah 10 run.

### Menengah
Empat thread masing-masing menambah counter 50.000 kali. Bandingkan waktu/waktu nyata antara mutex dan
atomic (pakai `clock()` atau ukur secara kasar). Simpulkan kapan atomic lebih cocok.

### Tantangan
Modifikasi `04_bounded_queue.c` agar `enqueue`/`dequeue` **memblokir** dengan condition variable:
producer menunggu saat penuh, consumer menunggu saat kosong. Gunakan `pthread_cond_wait` di dalam loop
pengujian kondisi (karena spurious wakeup diizinkan oleh POSIX). Petunjuk bertahap: (1) tambah
`pthread_cond_t not_full, not_empty`; (2) di `enqueue`, `while (count == cap) cond_wait(&not_full, &mtx)`;
(3) setelah enqueue, `cond_signal(&not_empty)`; (4) pasangan simetris di `dequeue`; (5) `cond_broadcast`
atau `cond_signal` saat state berubah. (Pengerjaan penuh ada di minggu 12.)

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan critical section dan mengapa lock/unlock harus berpasangan.
- [ ] Saya memeriksa return code `pthread_mutex_lock`, bukan `errno`.
- [ ] Saya bisa menjelaskan deadlock dan mengapa lock ordering mencegahnya.
- [ ] Saya tahu perbedaan mutex (invariant multi-langkah) dan atomic (satu operasi).
- [ ] Saya bisa menjelaskan data apa yang dilindungi sebuah lock.
- [ ] Saya paham bahwa deadlock bisa lulus berkali-kali lalu tiba-tiba terjadi.

---

## 10. Ringkasan

Mutex melindungi critical section: satu thread di dalam pada satu waktu, sehingga invariant terjaga.
Selalu pasangkan lock/unlock, periksa return code (bukan errno), dan gunakan lock ordering untuk mencegah
deadlock. Atomic cocok untuk satu operasi seperti `counter++`, tetapi tidak untuk urutan langkah majemuk.
Bounded queue berbasis mutex (minggu 8) adalah fondasi proyek akhir minggu 12.

**Glosarium mini:** mutex, critical section, deadlock, lock ordering, atomic operation, condition variable,
invariant. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [pthread_mutex_lock — POSIX Issue 8](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutex_lock.html) — tabel perilaku tipe mutex, return error number.
- [pthread_mutexattr_settype — POSIX Issue 8](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutexattr_settype.html) — default = `PTHREAD_MUTEX_DEFAULT`.
- [pthread_cond_broadcast — POSIX Issue 8](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_cond_broadcast.html) — spurious wakeup diizinkan, butuh loop uji kondisi.
- [atomic_fetch_add — cppreference](https://en.cppreference.com/w/c/atomic/atomic_fetch_add) — sejak C11, `<stdatomic.h>`.
- [The Linux Programming Interface — TLPI](https://man7.org/tlpi/) — ch. 30 (Mutexes), ch. 31 (Condition Variables), ch. 32 (Deadlocks).

## 12. Jembatan ke minggu 9

Sekarang kamu bisa mengamankan shared state. Langkah berikutnya: **membangun struktur data yang lebih
kaya dengan pointer** — linked list, dan bagaimana kontrak ownership menjaga invariant saat node
ditambah/dilepas. Lanjut:
[`week-09-data-structures-ownership.md`](week-09-data-structures-ownership.md)
