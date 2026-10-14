# Minggu 11 — Debugging, Sanitizer, dan Pemahaman Compiler

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** bagaimana menemukan bug yang tidak selalu terlihat dari output?

---

## 1. Mengapa ini penting

Kamu sudah menulis cukup banyak C untuk tahu bahwa "program jalan, berarti benar" adalah jebakan. Banyak
bug — heap overflow, use-after-free, data race — tidak crash saat kau uji, lalu meledak di production.
Minggu ini memberimu kebiasaan dan alat untuk menangkapnya: peringatan compiler, GDB untuk inspeksi
langsung, dan sanitizer (AddressSanitizer, UndefinedBehaviorSanitizer, ThreadSanitizer) serta Valgrind
untuk deteksi otomatis. Kuncinya: pilih alat berdasarkan *kategori bug*, bukan menjalankan semua alat
secara acak.

## 2. Learning objectives

Setelah minggu 11, kamu dapat:

1. Menjelaskan peran `-Wall -Wextra -Wpedantic`, `-Wconversion`, `-Wshadow`, dan `-g`/`-O0`.
2. Menggunakan GDB dasar: breakpoint, `run`, `next`, `step`, `print`, `backtrace`, periksa memory.
3. Menggunakan AddressSanitizer dan UndefinedBehaviorSanitizer untuk deteksi otomatis.
4. Menggunakan ThreadSanitizer untuk data race (bila kompatibel).
5. Membaca stack trace dan memahami bahwa tiap alat punya batas.
6. Memilih alat berdasarkan kategori bug.

## 3. Prasyarat

Kamu sudah mengalami berbagai bug di minggu 3–10 (leak, UAF, race). Minggu ini memberi alat untuk
menemukannya secara sistematis.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Program selesai tanpa crash, tetapi hasilnya sesekali salah. Mata telanjang tidak cukup. Bagaimana
menemukannya tanpa menebak?

### Pendekatan: pilih alat dari gejala

Bukan "jalankan semua alat lalu lihat". Pakai peta ini:

- **Compiler warning** — untuk kecurigaan saat compile (tipe, inisialisasi, konversi). Murah, jalankan dulu.
- **AddressSanitizer** — untuk akses memory di luar batas, use-after-free, double free, leak.
- **UndefinedBehaviorSanitizer** — untuk perilaku tak terdefinisi (overflow, shift, null deref).
- **ThreadSanitizer** — untuk data race.
- **GDB** — untuk melihat state saat runtime ketika alat otomatis butuh konfirmasi manual.
- **Valgrind Memcheck** — deteksi leak/akses invalid (alternatif saat sanitizer tak tersedia).

### Penjelasan teknis

Compiler warning diaktifkan dengan flag
([GCC Warning Options](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)):
`-Wall -Wextra -Wpedantic` sebagai dasar; `-Wconversion` untuk konversi implicitly-narrowing; `-Wshadow`
untuk variabel menutupi nama lain. `-g` menyimpan informasi debug; `-O0`/`-Og` mematikan optimisasi agar
state program mudah ditebak.

**Sanitizer** ([GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)):
`-fsanitize=address` menyuntikkan cek ke tiap akses memory (deteksi out-of-bounds, use-after-free); ia
juga mengaktifkan `-fsanitize-address-use-after-scope`. `-fsanitize=undefined` menangkap UB saat runtime.
`-fsanitize=thread` (Clang: [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)) mendeteksi
data race. Semua butuh flag saat *kompilasi*; hasilnya muncul saat *run*.

**GDB** ([dokumentasi](https://sourceware.org/gdb/documentation/)): `break` menyetop di fungsi/baris,
`run` menjalankan sampai breakpoint, `next` melangkah per baris tanpa masuk fungsi, `step` masuk fungsi,
`print` mengevaluasi ekspresi, `backtrace` (`bt`) menampilkan rantai panggilan, `x` memeriksa memory.

**Valgrind Memcheck** ([manual](https://valgrind.org/docs/manual/mc-manual.html)) adalah alternatif deteksi
leak/akses invalid. **Catatan jujur:** Valgrind tidak terpasang di lingkungan penulisan ini, jadi contohnya
diberikan sebagai panduan, belum dijalankan.

---

## 5. Eksperimen terbimbing

### Eksperimen 1 — off-by-one tertangkap ASan (15 menit)

File `01_off_by_one_broken.c` menulis `a[n]` padahal `a` hanya punya `n` elemen.

```bash
cd labs/week-11
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 01_off_by_one 01_off_by_one_broken.c
./01_off_by_one
```

ASan melaporkan `heap-buffer-overflow` dan menghentikan program (`==...==ABORTING`). Tanpa sanitizer, program
ini bisa lulus — itulah bahayanya.

### Eksperimen 2 — use-after-free tertangkap ASan (10 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 02_uaf 02_use_after_free_broken.c
./02_uaf
```

ASan melaporkan `heap-use-after-free`. Sekali lagi: tanpa ASan ia bisa mencetak nilai lama dan tampak benar.

### Eksperimen 3 — pembacaan tak terinisialisasi (10 menit)

File `03_uninitialized_broken.c`:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_uninit 03_uninitialized_broken.c
./03_uninit
```

Di mesin penulis: compiler memperingatkan `'x' is used uninitialized`, dan program mencetak `x = 32766`
(nilai sampah). Peringatan compiler gratis dan sering kali menunjuk ke akar masalah — jangan diabaikan.

### Eksperimen 4 — melacak bug logika dengan GDB (15 menit)

File `04_gdb_demo.c` menghitung `sum(1..n)` tetapi salah (pakai `i < n` bukan `i <= n`).

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_gdb_demo 04_gdb_demo.c
gdb -batch -ex 'break sum_to_n' -ex run -ex 'print n' -ex 'step' -ex 'print i' -ex 'print s' -ex 'continue' ./04_gdb_demo
```

Transkrip nyata di mesin penulis:

```
Breakpoint 1, sum_to_n (n=5) at 04_gdb_demo.c:11
$1 = 5
12    int i = 1;
$2 = 32667        (nilai i SEBELUM inisialisasi — sampah)
$3 = 0            (s = 0 sudah benar)
sum 1..5 = 10  (harusnya 15)
```

Perhatikan: hasil `10` (= 1+2+3+4) membuktikan loop berhenti satu iterasi terlalu cepat. Step-through
memperlihatkan `i` dimulai dari nilai sampah lalu di-set `1`, dan `s` benar di nol — jadi bug ada di
kondisi `while`, bukan inisialisasi. Itu cara GDB mempersempit lokasi bug.

### Eksperimen 5 — data race dengan TSan (10 menit)

Ulangi `02_race_basic.c` minggu 7 di bawah `-fsanitize=thread` (sudah kamu lakukan di minggu 7); TSan
melaporkan data race pada `counter++`. Gunakan TSan khusus untuk masalah konkurensi.

---

## 6. Kesalahan umum

- **Mengabaikan warning compiler.** Banyak bug level nol tertangkap di sini; jalankan dengan
  `-Wall -Wextra -Wpedantic` sejak hari pertama.
- **Menganggap "lulus di mesin saya" berarti aman.** Itu exactly mengapa kita pakai sanitizer.
- **Menjalankan ASan/TSan tanpa `-g`.** Tanpa `-g`, laporan kehilangan nama fungsi/baris.
- **Menggabung ASan dan TSan dalam satu build tanpa memastikan dukungannya.** Dokumentasi resmi tidak
  konsisten soal ini (klaim "tidak bisa digabung" belum diverifikasi dari sumber primer) — aman memisahkan
  keduanya ke build berbeda.
- **Mengandalkan Valgrind tanpa menguji di mesinmu.** Valgrind lambat dan tidak mendeteksi semua (misal
  stack/static overflow). Sanitizer modern sering lebih tepat sasaran.

---

## 7. Latihan

### Mudah
Kompilasi semua program minggu 11 dengan `-Wall -Wextra -Wpedantic`. Hitung berapa warning muncul dan
jelaskan tiap satu.

### Menengah
Ambil `01_off_by_one_broken.c` dan perbaiki (ubah `i <= n` menjadi `i < n`). Jalankan lagi di bawah ASan —
sekarang harus lulus tanpa laporan. Tulis satu kalimat: apa yang diubah dan mengapa itu menyelesaikan bug.

### Tantangan
Tulis program kecil dengan `int x; int y = x + 1;` (UB), kompilasi dengan `-fsanitize=undefined -g`, dan
amati laporan UBSan. Jelaskan mengapa UBSan bisa (atau tidak bisa) menangkap pembacaan tak terinisialisasi
pada kasus ini, dan bandingkan dengan peringatan `-Wuninitialized`.

---

## 8. Checklist pemahaman

- [ ] Saya selalu mengompilasi dengan `-Wall -Wextra -Wpedantic -g -O0`.
- [ ] Saya bisa memilih alat berdasarkan kategori bug (ASan vs UBSan vs TSan vs GDB).
- [ ] Saya bisa membaca stack trace minimal dan menemukan baris pelakunya.
- [ ] Saya tahu bahwa "lulus di mesin saya" bukan bukti kebenaran.
- [ ] Saya memahami batas tiap alat (misal ASan tidak menangkap semua, Valgrind lambat).

---

## 9. Ringkasan

Debugging modern = kombinasi disiplin dan alat. Nyalakan peringatan compiler sejak awal; pakai ASan untuk
masalah memory, UBSan untuk UB, TSan untuk data race, dan GDB untuk inspeksi langsung. Pilih alat dari
gejala, bukan secara acak. "Tampak benar" bukan bukti — biarkan sanitizer yang membuktikan.

**Glosarium mini:** compiler warning, debugger, sanitizer, stack trace, invariant. Lengkap di
[`glossary.md`](../glossary.md).

## 10. Referensi

- [GCC Warning Options](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html) — daftar flag peringatan.
- [GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) — `-fsanitize=address/undefined/thread`.
- [Clang ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) — deteksi data race.
- [Valgrind Memcheck](https://valgrind.org/docs/manual/mc-manual.html) — alternatif deteksi leak (panduan, belum dijalankan di sini).
- [GDB documentation](https://sourceware.org/gdb/documentation/) — `x` memeriksa memory, `watch` watchpoint.

## 11. Jembatan ke minggu 12

Kamu kini punya semua alat untuk membuktikan kode benar. Satu ujian akhir: gabungkan semuanya — pointer,
heap, ownership, threads, sinkronisasi — ke dalam satu proyek utuh. Lanjut ke proyek akhir:
[`week-12-final-project.md`](week-12-final-project.md)
