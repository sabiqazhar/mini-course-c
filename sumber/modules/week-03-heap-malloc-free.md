# Minggu 3 — Heap, Dynamic Allocation, Ownership, dan Cleanup

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** bagaimana sebuah objek bisa hidup melampaui satu pemanggilan fungsi?

---

## 1. Mengapa ini penting

Ini minggu pertamamu benar-benar memegang kendali atas umur objek. Di minggu 2, objek lokal mati
begitu fungsi kembali. Di sini kamu belajar meminta memory yang bertahan sampai kamu secara eksplisit
melepasnya. Kekuatan ini datang dengan satu kewajiban: **ownership**. C tidak punya fitur bahasa
untuk melacak siapa yang bertanggung jawab melepas suatu alokasi; itu sepenuhnya kesepakatan yang
kamu tulis dan dokumentasikan. Tanpa kesepakatan yang jelas, lahirlah memory leak, double free, dan
use-after-free.

## 2. Learning objectives

Setelah minggu 3, kamu dapat:

1. Menjelaskan kapan alokasi dinamis diperlukan dan kapan tidak.
2. Memakai `malloc`, `calloc`, dan `free`, serta selalu memeriksa hasil alokasi.
3. Menjelaskan konsep ownership dan menentukan siapa yang memanggil `free`.
4. Menjelaskan dan mengenali dangling pointer, memory leak, double free, dan use-after-free.
5. Menulis pola cleanup yang benar saat terjadi error di tengah jalan.
6. Menjelaskan batas manfaat menginisialisasi pointer ke `NULL`.

## 3. Prasyarat

Kamu perlu pemahaman lifetime dari minggu 2
([`week-02-arrays-structs-lifetime.md`](week-02-arrays-structs-lifetime.md)) — khususnya bahwa objek
lokal mati saat fungsi kembali. Heap adalah jawaban atas keterbatasan itu.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Fungsi perlu mengembalikan data yang ukurannya baru diketahui saat runtime, atau yang harus tetap
ada setelah fungsi kembali. Variabel lokal tidak bisa, karena lifetime-nya habis. Solusinya: minta
memory dari **heap** (allocated storage duration), yang umurnya kamu tentukan sendiri.

### Analogi: menyewa brankas vs laci meja

Variabel lokal seperti laci di meja kerjamu: otomatis tersedia saat kamu duduk (masuk fungsi) dan
lenyap saat kamu pulang (fungsi kembali). Heap seperti menyewa brankas di bank: kamu sewa kapan saja,
isi apa saja, dan ia tetap ada besok. Syaratnya: kamu harus mengembalikan kuncinya (panggil `free`)
atau brankas itu tetap terhitung milikmu selamanya (leak).

### Batas analogi

Di bank, "lupa mengembalikan kunci" cuma masalah administrasi. Di program, leak menumpuk: semakin
lama program jalan, semakin banyak memory terpakai, sampai sistem kehabisan. Dan tidak ada "bank" yang
menagih; OS hanya mengambil semua kembali saat process berakhir. Itu sebabnya leak sering luput di
program pendek uji coba.

### Penjelasan teknis

`malloc(size)` meminta blok memory sebesar `size` byte dari allocator (biasanya glibc di Linux).
Ia mengembalikan `void *` ke blok itu, atau `NULL` bila gagal. `free(ptr)` mengembalikan blok ke
allocator. Menurut man page Linux
([malloc(3)](https://man7.org/linux/man-pages/man3/malloc.3.html)): `malloc` mengembalikan `NULL` saat
error; `free(NULL)` adalah *no-op* (POSIX teks di
[free(3p)](https://man7.org/linux/man-pages/man3/free.3p.html) menyatakannya sebagai "no action").
`calloc(n, size)` mirip `malloc(n * size)` namun mengosongkan (nol) isinya.

Tiga level abstraksi yang wajib dipisahkan:

- **Standar C**: menjamin `malloc`/`free` ada, mengembalikan `void *`, `free(NULL)` aman. Tidak
  menentukan *bagaimana* allocator bekerja.
- **POSIX/Linux (glibc)**: detail seperti `malloc` yang memakai `mmap` untuk blok besar (di atas
  MMAP_THRESHOLD default 128 kB) dan bisa disetel via `mallopt`. Ini bukan aturan C.
- **Compiler/arsitektur**: tata letak heap di memory adalah urusan implementasi.

### Diagram

```
   Stack (otomatis)              Heap (allocated)
┌──────────────┐            ┌────────────────────────┐
│ main:        │            │  [ int ][ int ][ int ] │  <- dialokasikan malloc
│  p ──────────┼──────────▶ │   ^ptr                 │
│  (pointer)   │            │                        │
└──────────────┘            └────────────────────────┘
   p hidup saat main         blok hidup sampai free(p)
   berjalan; blok heap       dipanggil, lepas dari
   hidup lebih lama.         lifetime main.
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-03/01_malloc_int.c`](../labs/week-03/01_malloc_int.c)

```c
int *p = malloc(sizeof *p);   /* (1) alokasikan tempat untuk satu int */
if (p == NULL) {              /* (2) SELALU periksa hasil */
    fprintf(stderr, "alokasi gagal\n");
    return 1;
}
*p = 7;
printf("nilai = %d, alamat = %p\n", *p, (void *)p);
free(p);                      /* (3) kembalikan memory */
```

**(1)** `sizeof *p` lebih baik daripada `sizeof(int)`: kalau tipe `p` berubah, ukuran ikut berubah
otomatis. Tidak ada cast pada hasil `malloc` — di C, `void *` dikonversi otomatis ke tipe pointer
tujuan, dan cast yang tidak perlu bisa menutupi peringatan bila header `<stdlib.h>` lupa di-include.

**(2)** Memeriksa `NULL` bukan sopan santun; ini wajib. Mengabaikannya berarti berpotensi
mendereference `NULL` di bawah, yaitu undefined behavior.

**(3)** `free(p)` mengakhiri *allocated storage duration* blok itu. Setelah baris ini, `p` menjadi
**dangling** — nilainya masih alamat lama, tetapi objek yang ditunjuk sudah tidak valid.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-03
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_malloc_int 01_malloc_int.c
./01_malloc_int
```

### Hasil yang diharapkan

```
nilai = 7, alamat = 0x5620246582a0
```

Alamat akan berbeda tiap run (ASLR).

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — array dinamis dengan calloc (5 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_array_dinamis 02_array_dinamis.c
./02_array_dinamis
```

Hasil:

```
a[0] = 10
a[1] = 20
a[2] = 30
a[3] = 40
```

Satu `free(a)` cukup untuk seluruh array, berapapun ukurannya. Inilah daya heap: ukuran runtime.

### Eksperimen 2 — ownership yang jelas (10 menit)

File [`03_ownership_fungsi.c`](../labs/week-03/03_ownership_fungsi.c) menunjukkan pola: fungsi
mengalokasikan dan mengembalikan pointer, `main` membebaskannya. Kontrak ditulis di komentar — bukan
hanya di kepala. Hasil: `data[1] = 2`.

### Eksperimen 3 — melihat memory leak dengan ASan (15 menit)

File `04_leak_broken.c` adalah **contoh rusak**: `malloc` di dalam loop tanpa `free`. Kompilasi
dengan AddressSanitizer (LeakSanitizer ikut aktif):

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 04_leak 04_leak_broken.c
./04_leak
```

Hasil di mesin penulis:

```
=================================================================
==63780==ERROR: LeakSanitizer: detected memory leaks
...
SUMMARY: AddressSanitizer: 20 byte(s) leaked in 5 allocation(s).
```

Tanpa sanitizer, program selesai "diam" karena OS mengambil memory saat process berakhir. ASan
menunjukkan betapa mudahnya leak luput — dan betapa pentingnya selalu menjalankan kode di bawah
sanitizer selama fase belajar.

### Eksperimen 4 — use-after-free (15 menit)

File `05_uaf_broken.c` adalah **contoh rusak**: menulis/membaca setelah `free`. Kompilasi dengan ASan:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 05_uaf 05_uaf_broken.c
./05_uaf
```

ASan melaporkan `heap-use-after-free` dan menghentikan program. Di run normal tanpa ASan, *bisa saja*
program mencetak 100 lagi atau nilai lain — itulah undefined behavior. Jangan pernah menganggap
"berhasil" berarti aman.

### Eksperimen 5 — free(NULL) aman (3 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 06_free_null 06_free_null.c
./06_free_null
```

Mencetak `free(NULL) aman: tidak ada yang terjadi`. Ini berguna untuk pola: bebaskan di awal fungsi
atau di jalur error tanpa takut memanggil `free` dua kali pada pointer yang belum dialokasikan.

---

## 7. Kesalahan umum

- **Tidak memeriksa `NULL` dari `malloc`.** Selalu periksa. Di mesin ini alokasi kecil jarang gagal,
  tetapi ketika gagal dan kamu abaikan, dereference `NULL` = UB.
- **Double free.** Memanggil `free(p)` dua kali. UB, sering merusak metadata allocator. Setelah `free`,
  set `p = NULL` agar free kedua menjadi no-op (tetapi ingat: alias pointer lain tetap dangling).
- **Use-after-free.** Mengakses `p` setelah `free(p)`. Set `p = NULL` segera setelah free sebagai
  kebiasaan, dan jangan biarkan pointer lain menunjuk ke blok yang sama tanpa dilacak.
- **Mengira `p = NULL` memperbaiki semua alias.** Men-set satu pointer ke `NULL` tidak mengubah pointer
  lain yang masih menunjuk ke objek yang sama. Ownership harus dikelola secara sadar, bukan sekadar
  assign `NULL`.
- **Mencampur malloc hasil fungsi tanpa kontrak.** Kalau fungsi mengembalikan pointer, tulis (di
  komentar/dokumentasi) siapa yang berhak `free`. Tanpa itu, leak atau double free hampir pasti terjadi.

---

## 8. Latihan

### Mudah
Alokasikan array `double` berukuran `n` (minta `n` dari `scanf`), isi dengan pangkat dua indeksnya,
cetak, lalu `free`. Pastikan `n` divalidasi (> 0).

### Menengah
Tulis fungsi `int *buat_array(size_t n)` yang mengalokasikan dan mengembalikan array, atau `NULL` bila
gagal. Di `main`, panggil, isi, cetak, dan bebaskan. Dokumentasikan kontrak ownership-nya. (Solusi
ada di [`solutions/week-03/buat_string.c`](../solutions/week-03/buat_string.c) sebagai pola serupa.)

### Tantangan
Tulis fungsi yang mengembalikan string hasil penyusunan `"Halo, " + nama` yang dialokasikan sendiri;
pemanggil bertanggung jawab `free`. Solusi:
[`solutions/week-03/buat_string.c`](../solutions/week-03/buat_string.c). Petunjuk bertahap: (1)
hitung `strlen(awalan) + strlen(nama) + 1`; (2) `malloc` ukur itu; (3) periksa `NULL`; (4) `snprintf`;
(5) kembalikan; (6) di `main`, cetak lalu `free`.

---

## 9. Checklist pemahaman

- [ ] Saya selalu memeriksa `NULL` hasil `malloc`/`calloc`.
- [ ] Saya bisa menjelaskan siapa pemilik suatu alokasi dalam kode saya, dan kapan ia boleh dilepas.
- [ ] Saya tahu bahwa `free(p)` membuat `p` dangling, bukan "kosong".
- [ ] Saya bisa menjelaskan beda leak, double free, dan use-after-free, serta mana yang dideteksi
      oleh AddressSanitizer.
- [ ] Saya tahu batas `p = NULL` setelah free (tidak memperbaiki alias lain).

---

## 10. Ringkasan

Heap memberimu objek dengan umur yang kamu tentukan lewat `malloc`/`free`. Kekuatan itu menuntut
ownership yang eksplisit: tulis siapa yang `free`. Selalu periksa `NULL`, hindari double free dan
use-after-free, dan jadikan sanitizer teman saat belajar. `p = NULL` setelah `free` adalah kebiasaan
baik, tetapi bukan sihir yang memperbaiki semua pointer yang menunjuk ke objek sama.

**Glosarium mini:** allocation, deallocation, ownership, aliasing, memory leak, dangling pointer,
use-after-free, double free. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [malloc(3) — Linux man-pages](https://man7.org/linux/man-pages/man3/malloc.3.html) — perilaku, `NULL` saat gagal, detail glibc.
- [free(3p) — POSIX](https://man7.org/linux/man-pages/man3/free.3p.html) — `free(NULL)` adalah no-op.
- [AddressSanitizer — GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) — flag `-fsanitize=address`.

## 12. Jembatan ke minggu 4

Sekarang kamu bisa mengalokasikan dan membebaskan. Langkah berikutnya: objek yang ukurannya harus
**berubah di tengah jalan** — misalnya dynamic array yang tumbuh. Di situ `realloc` masuk, beserta
cara amannya dan bagaimana mendeteksi overflow perhitungan ukuran. Lanjut:
[`week-04-realloc-dynamic-array.md`](week-04-realloc-dynamic-array.md)
