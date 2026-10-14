# Minggu 1 — Model Mental Memory, Variabel, Alamat, dan Pointer

**Estimasi durasi:** 5–7 jam (baca ±1,5 jam, eksperimen ±3 jam, latihan ±2 jam)
**Pertanyaan utama minggu ini:** apa bedanya nilai, alamat, dan pointer?

---

## 1. Mengapa ini penting

Hampir semua kebingungan dalam low-level programming bermuara pada satu hal: mencampuradukkan
*nilai* sesuatu dengan *tempat* sesuatu itu berada. Kalau model mental ini belum kokoh, maka
`malloc` akan terasa seperti sihir, `free` akan terasa berbahaya, dan `pthread` akan terasa seperti
menebak-nebak.

Minggu ini tidak menambah fungsionalitas apa pun ke programmu. Yang minggu ini lakukan adalah
mengubah cara kamu melihat sebuah baris sederhana seperti `int x = 5;`. Setelah minggu ini, kamu
akan melihat dua hal di sana, bukan satu.

## 2. Learning objectives

Setelah minggu 1, kamu dapat:

1. Menjelaskan bahwa variabel adalah objek yang punya nilai dan alamat, dan bahwa keduanya berbeda.
2. Memakai `&` untuk mendapatkan alamat dan `*` untuk mendeklarasi pointer serta dereference,
   dan menjelaskan perbedaan dua peran `*` itu.
3. Menjelaskan mengapa tipe pointer penting, walaupun semua pointer punya ukuran yang sama.
4. Menjelaskan mengapa pointer harus menunjuk ke objek yang valid sebelum di-dereference.
5. Mencetak alamat dengan `printf` dan `%p` secara benar, termasuk cast ke `(void *)`.
6. Menjelaskan apa itu undefined behavior, dan mengapa ia tidak berarti "program pasti crash".

## 3. Prasyarat

Yang kamu butuhkan: bisa menulis `main`, memanggil `printf`, dan mengompilasi dari terminal.
Kalau [`setup.md`](../setup.md) sudah kamu kerjakan, kamu sudah punya semuanya.

Minggu ini adalah fondasi untuk semua minggu berikutnya. Minggu 2 membicarakan *di mana* objek
hidup (array, struct, stack), minggu 3 membicarakan *bagaimana membuat objek hidup lebih lama*
(heap). Tanpa minggu 1, dua minggu itu akan terasa seperti hapalan.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Kamu punya dua variabel dan ingin sebuah fungsi mengubah nilainya. Kamu tulis fungsi yang
menukar nilai, panggil, lalu cetak — dan nilainya tidak berubah sama sekali. Kenapa?

Jawabannya sederhana dan mengejutkan: fungsi itu menerima **salinan nilai**, bukan **objek aslinya**.
Untuk mengubah objek asli, fungsi perlu tahu **alamat** objek itu.

### Analogi: loker di stasiun

Bayangkan sebuah gedung penuh loker bernomor.

- **Loker** itu sendiri = objek di memory.
- **Nomor loker** = alamat.
- **Isi loker** = nilai.
- **Secarik kertas bertuliskan nomor loker** = pointer.

Kalau kamu memberi tahu teman "isi loker 42 adalah sepatu", itu seperti mengirim nilai.
Temanmu mencatat "sepatu" di kertasnya; ia tidak bisa mengubah isi loker 42.

Kalau kamu memberi tahu "nomornya 42", itu seperti mengirim alamat/pointer. Sekarang temanmu bisa
membuka loker 42 dan mengganti isinya — dan kamu yang datang belakangan akan melihat isi baru.

### Batas analogi (penting)

Analoginya membantu, tetapi ada tiga hal yang **tidak** sama dengan komputer:

1. Di komputer, "secarik kertas" (pointer) itu sendiri juga disimpan di loker, jadi ia juga punya
   nomor loker. Pointer bisa menunjuk ke pointer. Itu sebabnya ada `&p` dan `int **`.
2. Alamat di komputer tidak permanen. Di Linux, alamat yang kamu cetak bisa berubah setiap kali
   program dijalankan (ada fitur bernama ASLR — kita bahas di minggu 5). Jangan pernah menyimpan
   alamat ke file atau mengirimkannya ke program lain.
3. Di loker fisik, isi loker jelas "ukurannya". Di memory, ukuran yang dibaca ditentukan oleh
   **tipe** yang kamu pakai untuk membacanya. Byte yang sama bisa dibaca sebagai nilai yang
   berbeda-beda. Ini alasan mengapa tipe pointer penting.

### Penjelasan teknis

Dalam bahasa standar C, sebuah **objek** adalah region penyimpanan yang isinya bisa mewakili nilai.
Deklarasi `int x = 5;` menciptakan objek bertipe `int` yang isinya mewakili nilai 5. Objek itu
berada di suatu **alamat**. Ekspresi `&x` menghasilkan alamat itu. Ekspresi `x` menghasilkan nilai
objek itu ([cppreference: object](https://en.cppreference.com/w/c/language/object)).

Sebuah **pointer** adalah objek yang nilainya adalah sebuah alamat:

```c
int  x = 5;    /* objek int, nilai 5            */
int *p = &x;   /* objek pointer, nilai = &x     */
```

Dua peran tanda bintang yang sering membingungkan:

- Dalam **deklarasi**, `*` menandai "ini pointer": `int *p;`
- Dalam **ekspresi**, `*` berarti dereference, yaitu "baca/tulis objek yang ditunjuk": `*p = 10;`

Kontras yang sama berlaku untuk `&`: dalam ekspresi `&x` ia berarti "alamat dari x".

### Diagram

```
      nama      alamat        memory           nilai
   ┌──────────┬──────────┬──────────────┬──────────────┐
   │    x     │ 0x...1c  │   [ 5 ]      │  int         │
   ├──────────┼──────────┼──────────────┼──────────────┤
   │    p     │ 0x...20  │   [ 0x...1c ]│  int *       │
   └──────────┴──────────┴──────────────┴──────────────┘
                              │
                              └── isi p adalah alamat x  ──▶  (panah ke x)
```

Jadi: `p` bernilai `0x...1c`. `*p` bernilai `5`. `&p` bernilai `0x...20`.

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-01/02_lewat_pointer.c`](../labs/week-01/02_lewat_pointer.c)

```c
#include <stdio.h>

int main(void)
{
    int  x = 5;        /* (1) objek int bernama x, nilai 5        */
    int *p = &x;       /* (2) pointer p diisi alamat x            */

    printf("sebelum  : x = %d, *p = %d\n", x, *p);

    *p = 10;           /* (3) tulis 10 ke objek yang ditunjuk p   */

    printf("sesudah  : x = %d, *p = %d\n", x, *p);

    printf("p  = %p\n", (void *)p);    /* (4) nilai p, yaitu alamat x */
    printf("&x = %p\n", (void *)&x);
    printf("&p = %p\n", (void *)&p);   /* (5) alamat si pointer sendiri */

    printf("sizeof p  = %zu byte\n", sizeof p);   /* (6) ukuran pointer  */
    printf("sizeof *p = %zu byte\n", sizeof *p);  /* (7) ukuran objek    */
    return 0;
}
```

Penjelasan bagian yang mudah salah:

**(2)** `int *p = &x;` dibaca sebagai "p adalah `int *` (pointer ke int), diinisialisasi dengan
`&x`". Tanda `*` menempel pada p, bukan pada `int`. Karena itu `int* p, q;` membuat `p` pointer
tetapi `q` tetap `int` — salah satu jebakan klasik. Tulis satu deklarasi per baris sampai terbiasa.

**(3)** `*p = 10;` tidak mengubah pointer `p`. Ia mengubah objek yang ditunjuk `p`. Akibatnya `x`
berubah menjadi 10. Kalau kamu ingin mengubah *ke mana p menunjuk*, tulis `p = &y;` — tanpa bintang.

**(4)** `%p` adalah format khusus untuk mencetak alamat. Format ini mensyaratkan argumen bertipe
`void *`, karena itu kita menulis `(void *)p`. Tanpa cast, `printf` menerima argumen dengan tipe
yang tidak sesuai dengan formatnya, dan standar C tidak menjanjikan hasil yang bisa diandalkan.
Cast di sini bukan "menutupi masalah", melainkan memenuhi kontrak `printf`.

**(6) vs (7)** `sizeof p` adalah ukuran pointer (8 byte di mesin 64-bit ini), sedangkan
`sizeof *p` adalah ukuran objek yang ditunjuk (4 byte untuk `int`). Perhatikan bahwa `sizeof *p`
tidak mengevaluasi `*p`; ia hanya menghitung ukuran tipe hasilnya. Ini aman dilakukan walau
tulisannya mirip dereference.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-01
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_lewat_pointer 02_lewat_pointer.c
./02_lewat_pointer
```

### Hasil yang diharapkan

Satu kali jalan di mesin penulis (Ubuntu 24.04, x86-64, GCC 13.3):

```
sebelum  : x = 5, *p = 5
sesudah  : x = 10, *p = 10

p  (nilai p)      = 0x7ffeb70a2f1c
&x (alamat x)     = 0x7ffeb70a2f1c
&p (alamat p)     = 0x7ffeb70a2f20   <-- p sendiri juga punya alamat

sizeof p  = 8 byte  (ukuran si pointer)
sizeof *p = 4 byte  (ukuran objek yang ditunjuk)
```

**Yang akan berbeda di mesinmu:** semua angka `0x...` — alamat berubah setiap kali program
dijalankan, dan formatnya bergantung pada arsitektur. Pada mesin 32-bit, `sizeof p` akan menjadi 4.
**Yang akan sama:** `x = 10` setelah `*p = 10`, dan fakta bahwa `p` sama dengan `&x`.

---

## 6. Eksperimen terbimbing (20–30 menit)

### Eksperimen 1 — nilai dan alamat dari tiga tipe berbeda (5 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_nilai_dan_alamat 01_nilai_dan_alamat.c
./01_nilai_dan_alamat
```

Hasil di mesin penulis:

```
umur  : nilai = 30,   alamat = 0x7fffe5ac60ac, ukuran = 4 byte
suhu  : nilai = 36.5, alamat = 0x7fffe5ac60b0, ukuran = 8 byte
huruf : nilai = A,   alamat = 0x7fffe5ac60ab, ukuran = 1 byte
```

Amati: `huruf` (1 byte) beralamat lebih rendah daripada `umur` (4 byte), dan `umur` lebih rendah
daripada `suhu`. Tiga objek ini berdekatan, tetapi jaraknya tidak sekadar ukurannya — ada jarak
antar-alamat yang tidak terpakai. Itu **padding/alignment**, topik minggu 2. Untuk sekarang,
catat saja bahwa "berurutan dalam deklarasi" tidak menjamin "berurutan rapat di memory".

### Eksperimen 2 — mengapa tipe pointer penting (10 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_tipe_pointer 03_tipe_pointer.c
./03_tipe_pointer
```

Hasil di mesin penulis:

```
sizeof(int *)    = 8
sizeof(char *)   = 8
sizeof(double *) = 8
sizeof(void *)   = 8

*pi = 42   (dibaca sebagai int)
*pc = 42   (dibaca sebagai char: 1 byte pertama dari representasi int)
```

Semua pointer berukuran 8 byte, jadi "pointer besar" itu tidak ada. Yang berbeda adalah **berapa
banyak byte yang dibaca** saat dereference dan **bagaimana byte itu ditafsirkan**. Program ini
sengaja tidak pernah men-dereference `pd` (pointer double ke objek int), karena itu undefined
behavior. Coba aktifkan barisnya kalau ingin melihat sendiri bahwa hasilnya tidak bisa diprediksi.

### Eksperimen 3 — melihat undefined behavior yang *tidak* crash (10 menit)

File `04_pointer_tanpa_inisialisasi_rusak.c` adalah **contoh rusak**. Ia mendeklarasikan
`int *p;` tanpa inisialisasi lalu menulis `*p = 123;`.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_rusak 04_pointer_tanpa_inisialisasi_rusak.c
./04_rusak
```

Compiler sudah memberi peringatan, dan itu hal terpenting di sini:

```
warning: 'p' is used uninitialized [-Wuninitialized]
```

Di mesin penulis, program **tidak crash**. Dua kali jalan menghasilkan alamat berbeda
(`0x7ffda155ffe8`, lalu `0x7fffcfdfc518`) dan mencetak `*p = 123` dengan exit code 0.

Inilah pelajaran yang paling berharga minggu ini: **undefined behavior bukan jaminan crash.**
Standar C mendefinisikan UB sebagai situasi di mana standar tidak memberi batasan apa pun tentang
hasilnya ([cppreference: UB](https://en.cppreference.com/w/c/language/behavior)). Program bisa crash,
bisa jalan normal, bisa merusak data lain yang kebetulan berada di alamat itu. "Sudah saya coba,
aman kok" bukan bukti apa-apa.

### Eksperimen 4 — prediksi sebelum menjalankan (10 menit)

Lihat [`labs/week-01/05_prediksi.c`](../labs/week-01/05_prediksi.c). **Baca kodenya, jangan
dijalankan dulu.** Tulis tebakan output untuk baris A, B, C, dan D. Baru jalankan.

Hasil di mesin penulis:

```
A: x=1 y=2 *p=1
B: x=1 y=99 *p=99
C: &x=0x7ffdc64ebfd4 &y=0x7ffdc64ebfd8 p=0x7ffdc64ebfd8
D: y=99 y_lagi=7, &y=0x7ffdc64ebfd8 &y_lagi=0x7ffdc64ebfdc
```

Perhatikan baris B: `x` tetap 1 karena `p` sudah dialihkan ke `&y` sebelum penulisan. Pointer
menunjuk ke mana adalah state yang bisa berubah, sama seperti nilai.

---

## 7. Kesalahan umum dan cara mendiagnosisnya

**Lupa `&` saat mengirim alamat.**
Gejala: `warning: passing argument 1 of 'ubah' makes pointer from integer without a cast`.
Artinya kamu mengirim nilai (integer) ke parameter yang berharap pointer. Tambahkan `&`.

**Memakai `%d` untuk mencetak alamat, atau `%p` tanpa cast ke `void *`.**
Gejala: peringatan format, atau output yang tidak masuk akal. Alamat selalu pakai `%p`, dan argumennya
harus bertipe `void *`.

**Mengira `*` dalam deklarasi berlaku untuk semua variabel dalam satu baris.**
`int *p, q;` → `p` pointer, `q` int. Tulis terpisah.

**Men-dereference pointer yang belum ditunjuk ke mana-mana.**
Gejala: bisa crash, bisa tidak. Compiler biasanya sudah memperingatkan dengan
`-Wuninitialized` atau `-Wmaybe-uninitialized`. Jangan abaikan peringatan itu.

**Mengira alamat itu tetap.**
Alamat berubah setiap eksekusi (ASLR, minggu 5). Kalau kamu melihat alamat yang sama persis dua kali
berturut-turut, itu kebetulan — jalankan programmu lima kali sebelum menyimpulkan apa pun.

**Mengira `sizeof` pada pointer memberi ukuran data.**
`sizeof p` untuk `int *p` memberi ukuran pointer (8 byte di sini), bukan ukuran `int`.

---

## 8. Latihan

### Mudah — tiga variabel, tiga alamat

Tulis program yang mendeklarasikan `char`, `int`, dan `double`, lalu mencetak nilai, alamat, dan
`sizeof` masing-masing. Tambahkan satu pointer untuk tiap variabel dan cetak juga nilai pointer itu.
Jalankan tiga kali: apa yang berubah dan apa yang tetap?

### Menengah — swap yang benar dan yang salah

Tulis dua fungsi: `swap_salah(int a, int b)` dan `swap_benar(int *a, int *b)`. Panggil keduanya
dari `main` dan cetak nilai sebelum dan sesudah. Jelaskan dengan kata-katamu sendiri mengapa versi
pertama tidak berpengaruh.

### Tantangan — analisis tiga "tempat tinggal" objek

Tulis `void analisis(const char *label, int *p)` yang mencetak: label, `*p`, `p`, `&p`,
`sizeof p`, dan `sizeof *p`. Panggil dengan alamat variabel lokal, variabel global, dan variabel
`static` lokal. Lalu panggil dengan `NULL` dan pastikan fungsi tidak men-dereference-nya.

Petunjuk bertahap untuk tantangan: (1) tulis dulu versi yang tidak menangani `NULL`; (2) tambahkan
pemeriksaan `if (p == NULL)`; (3) jalankan lima kali dan amati apakah jarak relatif antar-alamat
konsisten; (4) tulis satu kalimat kesimpulan, dan satu kalimat tentang apa yang *tidak* bisa kamu
simpulkan dari eksperimen ini.

Solusi referensi ada di [`solutions/week-01/swap.c`](../solutions/week-01/swap.c) dan
[`solutions/week-01/analisis.c`](../solutions/week-01/analisis.c). Coba dulu minimal 20 menit
sebelum membukanya.

---

## 9. Checklist pemahaman (self-assessment)

Centang hanya kalau kamu bisa menjelaskannya tanpa melihat materi:

- [ ] Saya bisa menjelaskan `int x;`, `&x`, `int *p = &x;`, dan `*p` dengan kata-kata saya sendiri,
      bukan dengan mengulang definisi.
- [ ] Saya tahu bahwa `p` dan `*p` adalah dua hal berbeda, dan bisa menyebutkan mana yang menyimpan
      alamat.
- [ ] Saya tahu `sizeof p` dan `sizeof *p` bisa berbeda dan tahu kapan memakai yang mana.
- [ ] Saya selalu menginisialisasi pointer sebelum men-dereference-nya, dan tahu apa yang terjadi
      kalau tidak.
- [ ] Saya tahu mengapa `%p` perlu cast ke `(void *)`.
- [ ] Saya bisa memberi satu contoh undefined behavior dan menjelaskan mengapa ia tidak harus crash.

Kalau ada satu saja yang belum bisa kamu jelaskan, ulangi eksperimen 3 dan 4. Itu murah dan jauh
lebih cepat daripada menemukan masalah ini lagi di minggu 3.

---

## 10. Ringkasan

Variabel adalah objek. Objek punya nilai dan alamat. `&x` memberi alamat; `int *p` membuat pointer;
`*p` mengakses objek yang ditunjuk. Tipe pointer menentukan berapa byte dibaca dan bagaimana
ditafsirkan, walaupun semua pointer berukuran sama. Pointer yang tidak menunjuk ke objek valid
tidak boleh di-dereference, dan melakukannya adalah undefined behavior — yang tidak berarti crash.

**Glosarium mini minggu ini:** object, value, address, pointer, dereference, null pointer,
undefined behavior. Penjelasan lengkap ada di [`glossary.md`](../glossary.md).

## 11. Referensi minggu ini

- [cppreference — Objects and alignment](https://en.cppreference.com/w/c/language/object) — definisi object, value, dan alignment menurut standar C.
- [cppreference — Undefined behavior](https://en.cppreference.com/w/c/language/behavior) — definisi UB dan contoh nyata bahwa UB tidak berarti crash.
- [Beej's Guide to C Programming](https://beej.us/guide/bgc/) — pengantar C yang ramah untuk intuisi tambahan.

## 12. Jembatan ke minggu 2

Sekarang kamu tahu objek punya alamat. Pertanyaan berikutnya adalah: **berapa lama alamat itu
valid?** Di minggu 2 kita melihat array (banyak objek sejenis berurutan), struct (beberapa objek
dalam satu wadah), dan yang terpenting: perbedaan antara *scope* (di mana nama bisa dipakai) dan
*lifetime* (kapan objek itu ada). Di situlah kita mulai melihat mengapa mengembalikan alamat
variabel lokal adalah kesalahan yang sering sekali terjadi.

Lanjut: [`week-02-arrays-structs-lifetime.md`](week-02-arrays-structs-lifetime.md)
