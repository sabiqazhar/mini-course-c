# Proyek Akhir Minggu 12 — Antrean Berbatas Aman-Thread (Bounded Thread-Safe Queue)

Ini adalah *capstone* yang menyatukan seluruh kursus: pointer & memory (M1),
array/struct & lifetime (M2), heap & malloc (M3–M4), virtual memory & proses (M5–M6),
thread & race (M7), mutex & atomik (M8), struktur data & ownership (M9),
allocator (M10), dan debugging (M11).

## Yang dibangun

Sebuah antrean (queue) berbatas (*bounded*) yang bisa dipakai bersama oleh banyak
produser dan banyak konsumer secara aman, tanpa *busy-wait*, dengan *shutdown*
yang deterministik.

| File            | Isi                                                         |
|-----------------|-------------------------------------------------------------|
| `tsqueue.h`     | API + definisi struct (terbuka untuk inspeksi pembelajaran) |
| `tsqueue.c`     | Implementasi: ring buffer + 1 mutex + 2 condition variable  |
| `test_basic.c`  | Tes single-thread: FIFO & perilaku ring buffer (tail wrap)  |
| `test_mp_mc.c`  | Tes multi-producer/multi-consumer + shutdown deterministik  |
| `Makefile`      | `make`, `make test`, `make asan`, `make tsan`, `make clean` |

## Cara menjalankan

```bash
cd labs/week-12

# build + jalankan kedua tes (harus cetak FIFO benar & "cocok = YA")
make test

# jalankan di bawah AddressSanitizer + UBSan (cek leak / UB)
make asan

# jalankan di bawah ThreadSanitizer (cek data race)
make tsan
```

## Invarian & kontrak (baca sebelum mengubah kode)

- Invarian (selalu benar saat mutex terkunci): `0 <= count <= cap`; `head`/`tail`
  bergerak melingkar modulo `cap`.
- Items disimpan *by value* (salinan `int`). Pemanggil tetap pemilik `int` yang
  dikirim; queue tidak menyimpan pointer pemanggil.
- `enqueue` memblokir sampai ada slot kosong **atau** `shutdown` dipanggil.
- `dequeue` memblokir sampai ada item **atau** (`shutdown` **dan** queue kosong).
- `tsqueue_shutdown` aman dipanggil berkali-kali; ia hanya membangunkan thread.
- `tsqueue_destroy` **harus** dipanggil SETELAH semua thread di-join.

## Mengapa pakai `while`, bukan `if`, di cond_wait?

`pthread_cond_wait` bisa terbangun karena *spurious wakeup* (tanpa sinyal nyata),
atau karena sinyal lain yang membuat kondisi belum terpenuhi. Maka kondisi
harus selalu diuji ulang di dalam `while`. Ini pelajaran kunci minggu 8.

## Tantangan tambahan (opsional)

1. Tambahkan batas waktu pada enqueue/dequeue pakai `pthread_cond_timedwait`.
2. Ganti penyimpanan `int` menjadi pointer `void *` agar queue generik, lengkap
   dengan dokumentasi ownership siapa yang bertanggung jawab membebaskan objek.
3. Ukur throughput dengan berbagai `cap` dan jumlah thread; jelaskan pola yang
   kamu amati (petunjuk: kontensi mutex vs ukuran buffer).
