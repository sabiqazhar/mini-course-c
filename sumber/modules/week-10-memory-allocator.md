# Minggu 10 — Dasar Memory Allocator

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** apa yang dilakukan allocator untuk memenuhi permintaan memory?

---

## 1. Mengapa ini penting

Selama 7 minggu kamu memanggil `malloc` tanpa tahu apa yang terjadi di baliknya. Memahami allocator
membuat perilaku aneh (fragmentasi, overhead, alokasi besar berbeda dari kecil) menjadi masuk akal, dan
membuatmu menghargai mengapa `malloc` bukan gratis. Minggu ini kamu juga menulis allocator mainan —
bukan untuk dipakai production, tetapi untuk *merasakan* keputusan yang diambil allocator sungguhan.

## 2. Learning objectives

Setelah minggu 10, kamu dapat:

1. Menjelaskan perbedaan `malloc` sebagai API dan allocator di belakangnya.
2. Menjelaskan blok allocated/free, metadata allocator, alignment, splitting, dan coalescing.
3. Menjelaskan strategi first-fit dan best-fit sebagai pengantar.
4. Menjelaskan fragmentasi internal dan eksternal.
5. Mengimplementasikan arena/bump allocator sederhana, dan (bila dikuasai) free-list kecil.
6. Menjelaskan batas allocator mainan dan mengapa ia bukan pengganti `malloc`.

## 3. Prasyarat

Pointer, struct, heap, dan ownership dari minggu 2–9. Khususnya `free` dan kontrak siapa yang membebaskan
(minggu 3, 9).

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Ketika kamu memanggil `malloc(40)`, darimana 40 byte itu datang? Bagaimana `free` tahu ukuran blok untuk
dikembalikan? Mengapa `malloc(1)` sering memakan lebih dari 1 byte?

### Analogi: tata letak lahan parkir

Allocator seperti pengelola lahan parkir. Ia punya sebidang tanah (heap). Saat kamu minta tempat untuk 1
mobil, ia menandai petak berukuran standar (alignment) dan menyimpan catatan kecil "petak ini milik mobil
A, panjangnya sekian" (metadata). Saat mobil pergi (`free`), ia mencoret catatan dan menggabung petak kosong
yang bersebelahan jadi satu (coalescing) supaya mobil besar berikutnya muat. Bila banyak mobil kecil pergi
namun menyisakan celah-celah sempit yang tak cukup untuk mobil besar, itulah fragmentasi.

### Batas analogi

Di lahan parkir nyata, petak bisa bersebelahan secara fisik dan mudah digabung. Di allocator nyata, metadata
disimpan *di dalam* atau *di samping* blok, overhead-nya nyata, dan algoritma coalescing jauh lebih rumit
daripada coret-coret. Jangan anggap allocator mainanmu mirip glibc di detail — ia hanya mengajarkan ide.

### Penjelasan teknis

[`malloc(3)`](https://man7.org/linux/man-pages/man3/malloc.3.html) adalah **API**; di baliknya, allocator
(glibc ptmalloc secara default di Linux) mengelola pool memory yang didapat dari kernel lewat `brk`/`mmap`.
Beberapa fakta yang diverifikasi:

- `malloc` dengan argumen `0` mengembalikan `NULL` *atau* pointer yang bisa dilewatkan ke `free`
  (implementasi-defined); aman menganggap hasilnya bisa di-`free`.
- `free(NULL)` adalah no-op; `free` yang sah butuh ukuran blok — allocator menyimpannya sebagai **metadata**
  (sering di header kecil tepat sebelum pointer yang kamu terima).
- Di glibc/Linux, alokasi di atas ambang (`MMAP_THRESHOLD`, default 128 kB) ditangani lewat `mmap` langsung,
  dan allocator bisa disetel dengan `mallopt`. Ini detail Linux/glibc, **bukan** aturan bahasa C.

**Alignment**: allocator mengembalikan pointer yang cukup terata (minimal `max_align_t`, biasanya 16 byte di
x86-64), sehingga `malloc` untuk `int`, `double`, atau struct sekalipun valid. [`aligned_alloc`
(cppreference)](https://en.cppreference.com/w/c/memory/aligned_alloc) memberi perataan eksplisit saat kamu
butuh lebih (dan `size` harus kelipatan `alignment`).

**Splitting**: bila blok bebas lebih besar dari permintaan, allocator membelahnya, menyisakan sisa sebagai
blok bebas baru (mencegah pemborosan). **Coalescing**: saat `free`, blok bebas bersebelahan digabung agar
alokasi besar bisa dilayani.

**Fragmentasi**: *internal* = blok dialokasikan lebih besar dari permintaan (padding/metadata); *eksternal* =
banyak celah kecil yang tak terpakai di antara blok terpakai, sehingga total kosong cukup tetapi tak ada
satu celah yang cukup untuk permintaan besar.

**Strategi pencarian** (pengantar): *first-fit* mengambil blok bebas pertama yang cukup (cepat, tapi bisa
menyisakan celah di awal); *best-fit* mencari blok terkecil yang cukup (kurangi sisa, tapi lebih lambat dan
bisa memecah blok kecil-kecil = fragmentasi eksternal lebih parah).

### Diagram

```
Heap (sederhana):
[ header | blok A dipakai ][ header | blok bebas ][ header | blok B dipakai ][ blok bebas ]
                  ^pA                  ^bisa di-split/coalesce            ^pB
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-10/01_arena.c`](../labs/week-10/01_arena.c) — bump allocator (arena).

```c
typedef struct { char *buf; size_t cap; size_t off; } Arena;

static void *arena_alloc(Arena *a, size_t n) {
    if (a->off + n > a->cap) return NULL;   /* tidak cukup -> gagal jelas */
    void *p = a->buf + a->off;
    a->off += n;                              /* geser "break" internal */
    return p;
}
```

Arena tidak menyimpan metadata per-objek dan **tidak bisa `free` satu objek** — hanya `arena_reset`
(kembalikan `off=0`) atau `arena_free` (bebaskan seluruh buffer). Itu kelebihan (sangat cepat, deterministik)
sekaligus batas utamanya.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-10
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_arena 01_arena.c
./01_arena
```

### Hasil yang diharapkan

```
x=10 y=20  offset sekarang=8
alokasi 100 byte (lewati kapasitas) -> NULL (benar)
setelah reset, offset=0 (bisa dipakai lagi dari awal)
```

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — arena (10 menit)

Jalankan `01_arena`. Amati: dua `int` menghabiskan 8 byte (2 × 4) dan overflow ditolak dengan `NULL`. Arena
cocok untuk "alokasikan banyak objek lalu buang semua sekaligus" (misal parsing satu request HTTP).

### Eksperimen 2 — free-list allocator (15 menit)

File [`02_free_list.c`](../labs/week-10/02_free_list.c) mengimplementasikan pool blok ukuran tetap.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 02_free_list 02_free_list.c
./02_free_list
```

Hasil di mesin penulis:

```
a=1 b=2 c=3
setelah free(b), alokasi baru d -> 99 (alamat sama? ya)
```

Ini menunjukkan *reuse*: blok yang dikembalikan ke daftar bebas dipakai lagi saat alokasi berikutnya —
inti dari `free` pada allocator sesungguhnya (bedanya: allocator nyata mendukung ukuran bervariasi, splitting,
dan coalescing).

### Eksperimen 3 — ukur overhead (10 menit)

Alokasikan 1000 blok `int` lewat `malloc` biasa vs arena. Hitung berapa byte total tiap cara butuhkan
(arena = persis n×4 plus buffer; malloc = n×4 + overhead metadata per blok). Kesimpulan: allocator umum
memiliki overhead per alokasi.

---

## 7. Kesalahan umum (dan cara menghindarinya dengan mainan)

- **Mengira arena bisa `free` satu objek.** Tidak — gunakan `reset`/`free` seluruhnya, atau beralih ke
  free-list bila butuh reuse per-objek.
- **Lupa alignment pada free-list.** Blok harus diawali alamat yang cukup terata untuk tipe yang disimpan;
  di mainan ini ukuran blok tetap (`BLK_SZ = 32`) sudah cukup untuk `int`/`double`.
- **Menganggap allocator mainan aman untuk production.** Ia tidak menangani ukuran arbitrer, thread-safety,
  atau `realloc`. Jangan menggantikan `malloc` di kode nyata.
- **Double free pada free-list.** Memanggil `pool_free` dua kali untuk pointer sama akan merusak rantai.
  Production ptmalloc punya guard, tetapi mainan kita tidak.

---

## 8. Latihan

### Mudah
Tulis fungsi yang mengalokasikan 5 `int` dari arena, isi 0..4, cetak, lalu `arena_reset`, lalu alokasikan 2
`int` lagi — buktikan alamat kembali ke awal.

### Menengah
Tambahkan ke `02_free_list.c` fungsi `pool_alloc_zero` yang mengosongkan blok sebelum mengembalikan (seperti
`calloc`). Apa trade-off-nya?

### Tantangan
Ubah arena menjadi "arena berukuran variabel tanpa free per-objek" yang mencatat setiap alokasi ke dalam
array kecil agar nanti bisa dibebaskan berurutan. Bandingkan kemudahan vs bump allocator polos. Jelaskan
mengapa ini masih bukan pengganti `malloc`.

---

## 9. Checklist pemahaman

- [ ] Saya bisa membedakan `malloc` (API) dan allocator (implementasi).
- [ ] Saya tahu allocator menyimpan metadata dan mengembalikan pointer terata.
- [ ] Saya bisa menjelaskan splitting, coalescing, dan fragmentasi internal/eksternal.
- [ ] Saya paham batas arena dan free-list mainan, dan mengapa bukan pengganti `malloc`.
- [ ] Saya bisa menjelaskan trade-off first-fit vs best-fit.

---

## 10. Ringkasan

Di balik `malloc` ada allocator yang mengelola pool, metadata, alignment, splitting, dan coalescing. Arena
(bump) sangat cepat tetapi hanya bisa dibersihkan sekaligus; free-list mengizinkan reuse blok dengan
overhead terbatas. Keduanya mainan: allocator production menangani ukuran arbitrer, thread-safety, dan
`realloc` — jauh lebih rumit. Memahami lapisan ini membuat `malloc` berhenti terasa seperti sihir.

**Glosarium mini:** allocator, metadata, alignment, splitting, coalescing, fragmentation (internal/eksternal),
first-fit, best-fit. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [malloc(3) — Linux man-pages](https://man7.org/linux/man-pages/man3/malloc.3.html) — API, NULL saat gagal, catatan glibc/MMAP_THRESHOLD.
- [aligned_alloc — cppreference](https://en.cppreference.com/w/c/memory/aligned_alloc) — alokasi ber-alignment eksplisit.
- [The Linux Programming Interface — TLPI](https://man7.org/tlpi/) — ch. 7 (Memory Allocation).

## 12. Jembatan ke minggu 11

Sekarang kamu tahu bagaimana memory diatur. Pertanyaan terakhir praktis: **bagaimana menemukan bug yang
tidak terlihat dari output?** Minggu 11 memperkenalkan GDB dan sanitizer sebagai alat rutin debugging.
Lanjut: [`week-11-debugging-tools.md`](week-11-debugging-tools.md)
