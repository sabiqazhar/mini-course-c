# Solusi Referensi Minggu 12 — Bounded Thread-Safe Queue

Ini adalah *answer key* lengkap untuk proyek akhir. Semua file di direktori ini sudah
**terverifikasi**:

- Kompilasi bersih dengan `gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread` (nol peringatan).
- `make test` → FIFO benar, MPMC `diproduksi = 40000, dikonsumsi = 40000, cocok = YA`.
- `make asan` → AddressSanitizer + UBSan: **tidak ada** leak atau UB.
- `make tsan` → ThreadSanitizer: **tidak ada** data race.

| File            | Isi                                                          |
|-----------------|--------------------------------------------------------------|
| `tsqueue.h`     | API + definisi struct terbuka                                |
| `tsqueue.c`     | Implementasi (ring buffer + 1 mutex + 2 cond var)            |
| `test_basic.c`  | Tes single-thread: FIFO & ring-buffer wrap                   |
| `test_mp_mc.c`  | Tes multi-producer/multi-consumer + shutdown deterministik   |
| `Makefile`      | `make test` / `make asan` / `make tsan`                      |

## Rubrik penilaian (Definition of Done)

Centang tiap butir untuk memastikan implementasimu setara solusi:

- [ ] `tsqueue_init` menangani `cap == 0`, overflow, dan kegagalan `malloc`/primitif tanpa bocor.
- [ ] `enqueue` memblokir saat penuh, membangunkan **satu** konsumer (`signal not_empty`) setelah menaruh item.
- [ ] `dequeue` memblokir saat kosong, membangunkan **satu** produser (`signal not_full`) setelah mengambil item.
- [ ] Kedua `cond_wait` dipanggil di dalam `while` (bukan `if`) — anti spurious wakeup.
- [ ] `shutdown` memakai flag + `pthread_cond_broadcast` ke **kedua** kondisi.
- [ ] Urutan di `test_mp_mc`: join produser → `shutdown` → join konsumer; hasil `cocok = YA`.
- [ ] `tsqueue_destroy` dipanggil SETELAH semua thread di-join; tidak ada leak di ASan.
- [ ] TSan lulus (tidak ada data race pada `count`/`head`/`tail`/`buf`).
- [ ] Items disimpan *by value*; ownership `int` tetap di pemanggil (tidak ada pointer dibiarkan menggantung).

## Dua bug nyata yang sudah diperbaiki saat menyusun materi

1. **Urutan evaluasi argumen `printf`** — menulis
   `printf("...%zu...", tsqueue_enqueue(&q,10), q.count)` membuat `q.count`
   kadang dibaca *sebelum* enqueue berjalan (C tidak menjamin urutan). Diperbaiki
   dengan memisahkan panggilan ke pernyataan sendiri.
2. **Deadlock single-thread** — `while (tsqueue_dequeue(...)==0)` di tes single-thread
   menggantung karena `dequeue` menunggu sinyal yang tak akan datang. Diperbaiki
   dengan *drain* sejumlah item yang diketahui persis ada.

Kerjakan dulu minimal 30 menit sebelum membuka file di sini. Materi lengkap ada di
[`../../modules/week-12-final-project.md`](../../modules/week-12-final-project.md).
