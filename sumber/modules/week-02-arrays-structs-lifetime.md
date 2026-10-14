# Minggu 2 — Array, Struct, Scope, Lifetime, dan Layout Data

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** di mana sebuah objek hidup, dan kapan ia tidak lagi valid?

---

## 1. Mengapa ini penting

Minggu 1 memberimu model mental bahwa objek punya nilai dan alamat. Minggu ini menambahkan satu
dimensi waktu: **umur**. Menulis `int x;` di satu tempat menghasilkan objek yang hidup berbeda dari
`int x;` di tempat lain, dan memahami perbedaannya adalah kunci untuk tidak menciptakan *dangling
pointer* — masalah nomor satu yang akan kamu temui saat beralih ke heap di minggu 3.

Selain waktu, minggu ini membahas **bentuk** objek: array (banyak objek sejenis berurutan) dan
struct (beberapa objek berbeda dalam satu wadah), serta mengapa ukuran sebuah struct tidak selalu
sama dengan jumlah ukuran field-nya.

## 2. Learning objectives

Setelah minggu 2, kamu dapat:

1. Menjelaskan bahwa array adalah deretan objek bertipe sama yang berurutan, dan bahwa nama array
   *bukan* pointer meskipun sering berubah menjadi pointer dalam ekspresi.
2. Menjelaskan hubungan antara array dan pointer tanpa menyamakan keduanya.
3. Mengakses field struct dan memahami mengapa `sizeof(struct)` bisa lebih besar dari jumlah field.
4. Membedakan **scope** (di mana nama bisa dipakai), **storage duration/lifetime** (kapan objek ada),
   dan **lokasi penyimpanan** (stack/heap/static), serta tidak mencampuradukkannya.
5. Menjelaskan secara konseptual mengapa mengembalikan alamat variabel lokal adalah salah.
6. Menggunakan `sizeof`, `offsetof`, dan memahami alignment/padding sebagai detail implementasi,
   bukan hukum universal.

## 3. Prasyarat

Kamu harus sudah nyaman dengan `&`, `*`, dan perbedaan nilai vs alamat dari minggu 1
([`week-01-memory-and-pointers.md`](week-01-memory-and-pointers.md)).

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Kamu menulis fungsi yang membuat sebuah angka di dalamnya, lalu mengembalikan alamatnya. Di luar
fungsi, kamu mencetak angka itu dan kadang benar, kadang rusak. Kenapa "kadang"?

### Analogi: kamar hotel

Bayangkan setiap objek sebagai kamar hotel. **Scope** = "di mana nama kamar itu tertulis di buku
petunjuk" (hanya di dalam satu lantai/fungsi). **Lifetime** = "berapa lama kamar itu disewa".
Ketika fungsi kembali, penyewaan berakhir dan kamar itu "dibersihkan" untuk tamu berikutnya (stack
frame ditimpa). Mengembalikan alamat kamar itu lalu menggunakan kamar tersebut dari luar adalah seperti
memberi teman nomor kamar yang sudah kamu tinggalkan — nomor itu mungkin sudah dipakai tamu lain
sekarang.

### Batas analogi

Di hotel nyata, kamar tidak berubah isinya sendirinya. Di stack, isi memori *bisa* ditimpa oleh
panggilan fungsi berikutnya, sehingga membaca "kamar lama" bisa memunculkan sampah atau (jika
kebetulan belum tertimpa) angka lama. Itu sebabnya bug ini *kadang* tampak benar. Jangan pernah
mengandalkan kebetulan itu.

### Penjelasan teknis

Standar C mengenal **storage duration** (lihat
[cppreference: object](https://en.cppreference.com/w/c/language/object)):

- **automatic** — objek lokal; hidup sejak deklarasi sampai blok keluar (biasanya disimpan di stack
  pada kebanyakan implementasi).
- **static** — hidup sepanjang program eksekusi.
- **allocated** — hidup sejak `malloc` sampai `free` (ini topik minggu 3).
- **thread-local** — satu instance per thread.

**Scope** adalah cakupan nama dalam *teks* program (blok, file). Scope dan lifetime sering berbarengan
tetapi bukan hal yang sama: sebuah objek bisa punya scope terbatas namun lifetime statis (misal
`static int x;` di dalam fungsi — nama hanya dikenal di fungsi itu, tapi umurnya sepanjang program).

Tentang **stack**: standar C tidak pernah mewajibkan adanya "stack". Istilah ini menjelaskan
perilaku implementasi yang sangat umum. Materi ini memakai model stack sebagai alat bantu berpikir,
bukan sebagai janji standar. Jangan menganggap semua variabel lokal pasti berada di stack dalam
setiap build.

### Diagram: array dan struct

```
Array int a[5] (5 objek int berurutan, masing-masing 4 byte):

  a[0] a[1] a[2] a[3] a[4]
┌────┬────┬────┬────┬────┐
│ 10 │ 20 │ 30 │ 40 │ 50 │
└────┴────┴────┴────┴────┘
 ▲          ▲
 &a[0]     &a[2] = &a[0] + 8 byte

Struct dengan padding:
struct A { char c; int i; }
┌────┬────┬────┬────┐
│ c  │pad │     i     │   <- padding 3 byte agar i berada di alamat kelipatan 4
└────┴────┴────┴────┘
 0    1..3    4..7
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-02/01_array_alamat.c`](../labs/week-02/01_array_alamat.c)

```c
int a[5] = { 10, 20, 30, 40, 50 };

printf("sizeof a     = %zu byte  (seluruh array)\n", sizeof a);
printf("sizeof a[0]  = %zu byte  (satu elemen)\n", sizeof a[0]);

for (int i = 0; i < 5; i++)
    printf("&a[%d] = %p (jarak dari &a[0] = %td byte)\n",
           i, (void *)&a[i], (char *)&a[i] - (char *)&a[0]);

int *p = a;                       /* array decay menjadi pointer ke elemen 0 */
printf("sizeof p     = %zu byte\n", sizeof p);
printf("p + 1 - p    = %td\n", (p + 1) - p);
```

Perhatikan dua hal yang sering membingungkan:

**`sizeof a` vs `sizeof p`.** `a` adalah array sungguhan, jadi `sizeof a` = 5 × 4 = 20 byte.
Tetapi `p` adalah pointer; di mesin 64-bit `sizeof p` = 8 byte berapapun array yang ditunjuknya.
Inilah bukti bahwa array bukan pointer: kalau `a` adalah pointer, `sizeof a` juga akan 8.

**Pointer arithmetic bertipe.** `(p + 1) - p` menghasilkan 1, bukan 4. Operasi `p + 1` berarti
"pointer ke elemen int berikutnya", yaitu 4 byte lebih jauh. Selisih dua pointer bertipe sama
dihitung dalam satuan *elemen*, bukan byte. Itu sebabnya kita pakai `(char *)&a[i] - (char *)&a[0]`
kalau ingin jarak dalam byte.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-02
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_array_alamat 01_array_alamat.c
./01_array_alamat
```

### Hasil yang diharapkan

Di mesin penulis:

```
sizeof a       = 20 byte  (seluruh array, 5 x int)
sizeof a[0]    = 4 byte  (satu elemen)
sizeof &a[0]   = 8 byte  (sebuah pointer ke int)
&a[0] = 0x7fff5a9eaf10   (jarak dari &a[0] = 0 byte)
&a[1] = 0x7fff5a9eaf14   (jarak dari &a[0] = 4 byte)
&a[2] = 0x7fff5a9eaf18   (jarak dari &a[0] = 8 byte)
&a[3] = 0x7fff5a9eaf1c   (jarak dari &a[0] = 12 byte)
&a[4] = 0x7fff5a9eaf20   (jarak dari &a[0] = 16 byte)

sizeof p       = 8 byte  (ini pointer, beda dari sizeof a)
p + 1 - p      = 1         (satu elemen, bukan satu byte)
```

Alamat persis akan berbeda di mesinmu (ASLR), tetapi `sizeof` dan jarak 4-byte antar elemen sama
di semua mesin 64-bit little-endian tipikal.

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — padding pada struct (10 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_struct_padding 02_struct_padding.c
./02_struct_padding
```

Hasil di mesin penulis:

```
struct A : sizeof = 8, offset c = 0, offset i = 4
struct B : sizeof = 8, offset i = 0, offset c = 4
struct C : sizeof = 8, offset c1 = 0, offset c2 = 1, offset i = 4
```

`struct A { char c; int i; }` memakan 8 byte, bukan 5. Compiler menyisipkan 3 byte padding setelah
`c` agar `i` (butuh perataan 4) berada di alamat kelipatan 4. Urutan field mengubah ukuran: pada
`struct C`, dua `char` berurutan tidak butuh padding di antaranya, sehingga total tetap 8.

Catatan penting: susunan padding ini adalah **detail implementasi/ABI**, bukan aturan standar C.
Di arsitektur lain (misalnya yang butuh perataan berbeda), ukurannya bisa lain. Jangan asumsikan
layout struct portabel tanpa memeriksanya secara eksplisit.

### Eksperimen 2 — mengembalikan alamat lokal (10 menit)

File `03_return_local_broken.c` adalah **contoh rusak**. Compiler langsung memperingatkan:

```
warning: function returns address of local variable [-Wreturn-local-addr]
```

Jalankan pun (*bisa* mencetak 42, bisa mencetak sampah, bisa crash) membuktikan bahwa lifetime objek
lokal berakhir begitu fungsi kembali. Jangan pernah mengembalikan `&variabel_lokal`. Jika kamu
butuh objek bertahan, gunakan static (hati-hati: satu instance bersama) atau heap (minggu 3).

### Eksperimen 3 — prediksi & ukur (10 menit)

Gunakan solusi [`solutions/week-02/analisis_array.c`](../solutions/week-02/analisis_array.c) untuk
melihat bahwa `a[i]`, `*(a + i)`, `*(p + i)`, dan `p[i]` semuanya setara untuk array. Lalu ubah
`struct` di `02_struct_padding.c` dengan menambah field dan amati bagaimana ukuran berubah.

---

## 7. Kesalahan umum

- **Menyamaan array dan pointer.** `int a[5]; int *p = a;` — setelah baris ini `p` dan `a` sering
  dipakai seolah sama, tetapi `sizeof a != sizeof p`. Mereka sama hanya dalam ekspresi di mana array
  "decay" menjadi pointer, yaitu: sebagai argumen fungsi, operand `[]`, operand `+`, dan saat diumpan
  ke dereferensi. Pengecualian di mana array *tidak* decay: operand `sizeof`, `&` (address-of),
  `_Alignof`, dan inisialisasi string literal.
- **Lupa padding.** Mengirim struct lewat jaringan atau membaca field dengan offset manual tanpa
  `offsetof` akan salah karena padding. Gunakan `offsetof` atau serialisasikan field demi field.
- **Mengembalikan alamat lokal.** Sudah dibahas; ini sumber dangling pointer paling klasik.
- **Asumsi stack selalu ada.** Standar tidak menjaminnya. Model stack berguna, tetapi jangan
  menulis kode yang bergantung pada detail stack (misal mengintip stack frame fungsi lain).

---

## 8. Latihan

### Mudah
Buat array `double d[4]` dan cetak alamat tiap elemen beserta jarak dalam byte. Apa beda polanya
dengan array `int`?

### Menengah
Buat struct berisi dua `char` dan satu `int` dengan urutan berbeda, cetak `sizeof` dan `offsetof`
masing-masing, lalu jelaskan di mana padding muncul dan mengapa. Solusi:
[`solutions/week-02/struct_offsets.c`](../solutions/week-02/struct_offsets.c).

### Tantangan
Tulis fungsi yang menerima array `int` dan mencetak elemen lewat pointer arithmetic (boleh pakai
`p[i]`, `*(p + i)`, `*(a + i)`). Solusi:
[`solutions/week-02/analisis_array.c`](../solutions/week-02/analisis_array.c). Petunjuk bertahap:
(1) deklarasikan `int *p = a;`; (2) jalankan loop `for i in 0..n`; (3) cetak keempat bentuk untuk
tiap `i`; (4) tambahkan cetak `&a[0]` dan `a + 5` untuk melihat "batas satu-lebih" yang tak boleh
diakses.

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan perbedaan scope, lifetime, dan lokasi penyimpanan tanpa mencampuradukkannya.
- [ ] Saya tahu mengapa `sizeof a` (array) berbeda dari `sizeof p` (pointer ke array).
- [ ] Saya bisa menjelaskan mengapa `struct { char; int; }` bukan 5 byte.
- [ ] Saya tahu bahwa "stack" adalah detail implementasi, bukan jaminan standar C.
- [ ] Saya bisa menjelaskan mengapa mengembalikan `&x` dari fungsi itu salah.

---

## 10. Ringkasan

Array adalah objek berurutan; nama array *decay* menjadi pointer dalam kebanyakan ekspresi tetapi
bukan pointer. Struct menambahkan padding untuk memenuhi alignment, sehingga ukurannya bukan sekadar
jumlah field. Yang paling penting: pahami *kapan* sebuah objek masih valid. Lifetime lokal berakhir
saat fungsi kembali — menahan alamatnya adalah jebakan dangling pointer yang akan kita hindari
secara sistematis mulai minggu depan dengan heap.

**Glosarium mini:** array, struct, field, scope, storage duration, lifetime, stack, alignment,
padding. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [cppreference — Objects and alignment](https://en.cppreference.com/w/c/language/object) — definisi object, storage duration, alignment, padding menurut standar.
- [cppreference — Arrays](https://en.cppreference.com/w/c/language/array) — penjelasan decay dan perbedaan array vs pointer.
- [Beej's Guide to C Programming](https://beej.us/guide/bgc/) — intuisi tambahan untuk array dan struct.

## 12. Jembatan ke minggu 3

Kamu sekarang tahu bahwa objek lokal mati saat fungsi kembali. Bagaimana kalau kita ingin sebuah
objek bertahan lebih lama — misalnya dikembalikan dari fungsi, atau dipakai di banyak tempat? Jawabannya
adalah **heap**: alokasi dinamis dengan `malloc`/`free`. Di situlah kita belajar tentang ownership,
kebocoran memory, dan use-after-free. Lanjut:
[`week-03-heap-malloc-free.md`](week-03-heap-malloc-free.md)
