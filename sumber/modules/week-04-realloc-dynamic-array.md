# Minggu 4 — realloc, Dynamic Array, dan Alat Deteksi Masalah Memory

**Estimasi durasi:** 6–7 jam
**Pertanyaan utama minggu ini:** bagaimana struktur data berubah ukuran tanpa kehilangan data atau merusak memory?

---

## 1. Mengapa ini penting

`malloc` memberimu ukuran tetap. Dunia nyata sering minta ukuran yang tumbuh: buffer yang membesar saat
data masuk, vektor yang di-push terus. `realloc` adalah alatnya, tetapi ia punya aturan yang mudah
disalahgunakan: blok bisa dipindah, pointer lama bisa jadi invalid, dan perhitungan ukuran bisa
meluap. Minggu ini juga memperkenalkan *tool* deteksi masalah memory secara sistematis — karena
sejak sekarang kodemu cukup kompleks untuk butuh bantuan, bukan sekadar mata telanjang.

## 2. Learning objectives

Setelah minggu 4, kamu dapat:

1. Menjelaskan semantik `realloc`, kemungkinan pemindahan blok, dan aturan penggunaan pointer setelah sukses.
2. Menggunakan pola pointer sementara agar kegagalan `realloc` tidak merusak data.
3. Menghindari overflow pada `count * sizeof(element)` dan memeriksanya saat perlu.
4. Merancang API dynamic array (init, push, get/set, pop, destroy) dengan kontrak keamanan.
5. Memakai compiler warning, AddressSanitizer, UndefinedBehaviorSanitizer, dan (bila tersedia) Valgrind
   untuk menemukan masalah memory.

## 3. Prasyarat

Kamu harus menguasai `malloc`/`free` dan ownership dari minggu 3
([`week-03-heap-malloc-free.md`](week-03-heap-malloc-free.md)).

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Kamu membangun vektor integer. Awalnya kapasitas 2. Saat elemen ke-3 masuk, kapasitas habis. Kamu butuh
ruang lebih tanpa kehilangan 2 elemen pertama.

### Analogi: pindah apartemen

`realloc` seperti pindah ke apartemen lebih besar. Kamu minta pengelola (allocator) untuk menyediakan
ruang. Dua hasil mungkin: (a) kamu tetap di unit lama yang diperluas (alamat sama), atau (b) kamu pindah
unit baru dan barang dipindahkan (alamat berubah). Setelah pindah, alamat lama tidak lagi berlaku —
surat ke alamat lama tidak sampai ke kamu.

### Batas analogi

Di dunia nyata, "pindah" selalu berhasil kalau kamu punya uang. Di `realloc`, ketika memory tidak cukup,
pindahan **gagal dan barang tetap di unit lama**. Itu bedanya dengan `malloc` biasa, dan itulah mengapa
kamu tak boleh langsung menimpa pointer lama.

### Penjelasan teknis

[realloc(3)](https://man7.org/linux/man-pages/man3/realloc.3.html) mengubah ukuran blok yang ditunjuk
`ptr` menjadi `size` byte. Ia mengembalikan pointer ke blok baru, atau `NULL` bila gagal. Menurut teks
POSIX ([realloc(3p)](https://man7.org/linux/man-pages/man3/realloc.3p.html)): "isi tetap terjaga sampai
ukuran yang lebih kecil antara ukuran lama dan baru"; dan bila `realloc` gagal, **blok lama tidak
berubah dan tidak dibebaskan**. Inilah aturan emas: gunakan pointer sementara.

```c
int *tmp = realloc(p, new_size);
if (tmp == NULL) {
    /* p tetap valid; tangani kegagalan, jangan timpa p */
    free(p); return 1;
}
p = tmp;   /* hanya setelah sukses */
```

### Diagram

```
Sebelum realloc:         realloc gagal:          realloc sukses (pindah):
┌──────┐                ┌──────┐                ┌──────────────┐
│ p ──▶│ blok lama      │ p ──▶│ blok lama      │ tmp ──▶ blok baru (besar)
└──────┘                └──────┘                └──────────────┘
                           (tetap valid!)          (p LAMA kini invalid; pakai tmp)
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-04/02_realloc_aman.c`](../labs/week-04/02_realloc_aman.c)

```c
size_t n = 3;
int *p = malloc(n * sizeof *p);
/* ...isi p... */

int *tmp = realloc(p, 2 * n * sizeof *p);   /* (1) pakai tmp */
if (tmp == NULL) {                          /* (2) gagal: p tetap valid */
    fprintf(stderr, "realloc gagal; blok lama utuh\n");
    free(p);
    return 1;
}
p = tmp;                                    /* (3) baru setelah sukses */
n = 2 * n;
```

**(1)** Jangan `p = realloc(p, ...)`. Bila gagal, `p` jadi `NULL` dan blok lama bocor (karena kamu
kehilangan satu-satunya pointer ke sana).
**(2)** Saat gagal, `p` masih menunjuk blok lama yang utuh — kamu bisa lanjut atau `free` dengan aman.
**(3)** Hanya setelah `realloc` sukses kamu perbarui `p`.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-04
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_realloc_aman 02_realloc_aman.c
./02_realloc_aman
```

### Hasil yang diharapkan

```
sebelum: p = 0x559286a6b2a0
sesudah : p = 0x559286a6b2a0 (bisa sama atau beda dengan sebelumnya)
p[5] = 10
```

Alamat bisa sama atau beda di run yang berbeda — keduanya benar. Jangan mengandalkan salah satunya.

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — dynamic array lengkap (15 menit)

File [`01_dynamic_array.c`](../labs/week-04/01_dynamic_array.c) mengimplementasikan `IntVec` dengan
`vec_init`, `vec_push` (otomatis resize lewat `realloc` aman), dan `vec_destroy`. Kompilasi **selalu**
dengan sanitizer:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_dynamic_array 01_dynamic_array.c
./01_dynamic_array
```

Hasil di mesin penulis:

```
len=6 cap=8
v[0] = 0
v[1] = 10
v[2] = 20
v[3] = 30
v[4] = 40
v[5] = 50
```

Perhatikan kapasitas membesar dari 2 → 4 → 8 saat `push` melewati batas. `vec_destroy` memanggil
`free` dan men-set `data = NULL` agar tidak ada dangling pointer tertinggal.

### Eksperimen 2 — overflow perhitungan ukuran (15 menit)

File `03_overflow_broken.c` adalah **contoh rusak**. Di mesin 64-bit, `size_t` maksimum ~1.8e19, tetapi
`n * sizeof(int)` bisa meluap jika faktor pertamanya cukup besar:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_overflow 03_overflow_broken.c
./03_overflow
```

Hasil di mesin penulis:

```
n = 4611686018427387904
bytes yang diminta = 0  (seharusnya ~4.6e18, tapi meluap jadi 0!)
malloc(0) mengembalikan pointer bukan-NULL: 0x55c1fcbc02b0
```

`n = 2^62`, lalu `n * 4 = 2^64` yang pada `size_t` 64-bit meluap menjadi 0, sehingga `malloc(0)`
dipanggil — bukan alokasi besar. **Idiom aman**: periksa sebelum kali.

```c
if (n > SIZE_MAX / sizeof(int)) { /* tolak: akan meluap */ }
int *p = malloc(n * sizeof(int));
```

Idiom ini memakai `SIZE_MAX` dari `<stdint.h>`. Selalu gunakan ketika jumlah elemen berasal dari input
eksternal atau perhitungan runtime.

### Eksperimen 3 — menjalankan sanitizer (10 menit)

Jalankan ulang `01_dynamic_array.c` tanpa sanitizer dan dengan sanitizer; bandingkan. Tanpa sanitizer,
program mungkin "bersih" padahal ada bug halus. Dengan `-fsanitize=address,undefined`, ASan menangkap
out-of-bounds/use-after-free dan UBSan menangkap perilaku tak terdefinisi (misal pergeseran/overflow).
Gunakan kombinasi ini sebagai standar saat fase belajar dan saat menulis proyek akhir.

---

## 7. Kesalahan umum

- **`p = realloc(p, size)` langsung.** Bila gagal, `p = NULL` dan blok lama bocor. Selalu pakai tmp.
- **Mengakses `p` lama setelah `realloc` sukses dan blok pindah.** Pointer lama invalid; pakai nilai
  kembalian `realloc`.
- **Lupa `realloc` bisa mengembalikan alamat sama.** Jangan asumsikan berubah; jangan asumsikan tetap.
- **Overflow `n * sizeof(T)`.** Periksa dengan `SIZE_MAX / sizeof(T)` bila `n` tidak terkendali.
- **Double free saat error.** Jalur error sering memanggil `free` dua kali (satu di cleanup, satu di
  caller). Set pointer ke `NULL` setelah `free`, atau gunakan satu titik cleanup tunggal.

---

## 8. Latihan

### Mudah
Modifikasi `01_dynamic_array.c` agar kapasitas awal 1 dan resize faktor 2. Cetak `cap` tiap `push`
untuk melihat pola pertumbuhan.

### Menengah
Lengkapi `IntVec` dengan `vec_get`, `vec_set`, dan `vec_pop`, semuanya menolak akses di luar batas
dengan kode error. Solusi:
[`solutions/week-04/dynamic_array_full.c`](../solutions/week-04/dynamic_array_full.c). Petunjuk
bertahap: (1) `vec_reserve` yang mengalikan kapasitas sampai cukup; (2) `vec_push` panggil `vec_reserve`;
(3) `vec_get` periksa `i >= len`; (4) `vec_pop` periksa `len == 0`; (5) uji dengan sanitizer.

### Tantangan
Tambahkan fungsi `vec_shrink(IntVec*)` yang memanggil `realloc` ke `len` (bukan `cap`) bila `len` jauh
lebih kecil, tangani kegagalan, dan pastikan invariant tetap terjaga. Uji dengan input yang mengecil
drastis.

---

## 9. Checklist pemahaman

- [ ] Saya tahu bahwa `realloc` bisa memindah blok, dan bahwa blok lama invalid setelah sukses.
- [ ] Saya selalu memakai pointer sementara dan hanya menimpa setelah sukses.
- [ ] Saya tahu `realloc` gagal ⇒ blok lama tetap valid.
- [ ] Saya bisa menjelaskan mengapa `n * sizeof(T)` bisa meluap dan bagaimana mencegahnya.
- [ ] Saya menjalankan kode di bawah AddressSanitizer/UBSan secara rutin.

---

## 10. Ringkasan

`realloc` mengubah ukuran blok dan bisa memindahkannya; gunakan selalu pointer sementara agar kegagalan
tidak merusak data. Hindari overflow perhitungan ukuran dengan cek `SIZE_MAX`. Bangun API dynamic array
dengan kontrak keamanan (init/push/get/pop/destroy) dan jadikan AddressSanitizer + UBSan alat wajib.
Valgrind (`valgrind --leak-check=full ./program`) adalah lapisan lain yang bagus bila terpasang, tetapi
tidak tersedia di lingkungan penulisan ini sehingga contohnya diberikan sebagai panduan, belum dijalankan.

**Glosarium mini:** realloc, dynamic array, allocation overflow, AddressSanitizer, UndefinedBehaviorSanitizer, Valgrind. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [realloc(3) — Linux man-pages](https://man7.org/linux/man-pages/man3/realloc.3.html) — semantik dan catatan penggunaan.
- [realloc(3p) — POSIX](https://man7.org/linux/man-pages/man3/realloc.3p.html) — isi terjaga, blok lama utuh bila gagal.
- [GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) — `-fsanitize=address` dan `-fsanitize=undefined`.
- [Valgrind Memcheck](https://valgrind.org/docs/manual/mc-manual.html) — deteksi leak/invalid access (panduan, belum dijalankan di sini).

## 12. Jembatan ke minggu 5

Kamu sekarang bisa meminta, membesarkan, dan melepas memory. Pertanyaan berikutnya: **alamat yang kamu
cetak itu sebenarnya apa?** Apakah ia alamat fisik RAM? Mengapa tiap run berbeda? Di minggu 5 kita
masuk ke process, virtual address space, page, dan ASLR — fondasi untuk memahami `mmap` di minggu 6.
Lanjut: [`week-05-process-virtual-memory.md`](week-05-process-virtual-memory.md)
