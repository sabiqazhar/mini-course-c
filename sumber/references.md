# Referensi dan Sumber Primer

Materi kursus ini sengaja menyandarkan klaim teknis pada sumber primer bila memungkinkan. Halaman
ini mengumpulkan sumber yang dipakai, dengan catatan singkat tentang *untuk apa* masing-masing
sumber berguna dan *level abstraksi* apa yang diwakilinya — C, POSIX, Linux, atau tool.

Catatan penting: halaman manual dan standar bisa berubah antar versi. Versi yang dipakai saat
materi ini ditulis: Linux man-pages 6.x (2026), POSIX Issue 8 (IEEE Std 1003.1-2024), GCC 13.3.

## Dokumentasi bahasa C

- [cppreference — C reference](https://en.cppreference.com/w/c) — referensi praktis harian:
  sintaks, perilaku fungsi library, dan kutipan bagian standar yang relevan. Sangat baik untuk
  memeriksa "apakah ini dijamin standar?".
- [cppreference — Objects and alignment](https://en.cppreference.com/w/c/language/object) —
  definisi object, value, alignment, padding, dan trap representation, lengkap dengan rujukan
  ke pasal C11.
- [cppreference — Undefined behavior](https://en.cppreference.com/w/c/language/behavior) —
  definisi UB beserta contoh nyata hasil kompilasi, berguna untuk menunjukkan bahwa UB tidak
  berarti crash.
- [cppreference — Memory model](https://en.cppreference.com/w/c/language/memory_model) —
  menjelaskan data race dan menyatakan bahwa data race menghasilkan undefined behavior (C11 5.1.2.4).
- [WG14 — N1570, draft C11](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf) —
  draft publik yang paling sering dikutip untuk C11. **Ini draft, bukan standar final ISO.**
  Kalau kamu butuh kepastian hukum untuk proyek nyata, pakai edisi ISO resmi.
- [cppreference — atomic_fetch_add](https://en.cppreference.com/w/c/atomic/atomic_fetch_add) dan
  [ATOMIC_*_LOCK_FREE](https://en.cppreference.com/w/c/atomic/ATOMIC_LOCK_FREE_consts) —
  referensi `<stdatomic.h>`.

## Linux manual pages (man7.org)

Semua tersedia juga lewat perintah `man` di sistem kamu.

- [malloc(3)](https://man7.org/linux/man-pages/man3/malloc.3.html) — `malloc`, `free`, `calloc`,
  `realloc`: nilai kembalian, perilaku saat gagal, dan catatan spesifik Linux/glibc.
- [realloc(3p)](https://man7.org/linux/man-pages/man3/realloc.3p.html) — versi teks POSIX untuk
  `realloc`, termasuk jaminan "isi tetap terjaga sampai ukuran yang lebih kecil" dan
  "blok asli tidak berubah bila gagal".
- [free(3p)](https://man7.org/linux/man-pages/man3/free.3p.html) — teks POSIX: "`free()` dengan
  null pointer tidak melakukan apa-apa".
- [mmap(2)](https://man7.org/linux/man-pages/man2/mmap.2.html) — pemetaan memory: argumen,
  `MAP_FAILED` yaitu `(void *) -1`, `errno`, `offset` harus kelipatan ukuran page, dan `mmap`
  gagal dengan `EINVAL` bila `length` nol.
- [munmap(2)](https://man7.org/linux/man-pages/man2/munmap.2.html) — melepas pemetaan.
- [proc_pid_maps(5)](https://man7.org/linux/man-pages/man5/proc_pid_maps.5.html) — format kolom
  `/proc/self/maps`. Perhatikan: deskripsi ini sekarang hidup di halaman `proc_pid_maps(5)`,
  terpisah dari `proc(5)`.
- [proc(5)](https://man7.org/linux/man-pages/man5/proc.5.html) — gambaran besar filesystem `/proc`,
  termasuk `/proc/self`.
- [open(2)](https://man7.org/linux/man-pages/man2/open.2.html),
  [read(2)](https://man7.org/linux/man-pages/man2/read.2.html),
  [close(2)](https://man7.org/linux/man-pages/man2/close.2.html),
  [fstat(2)](https://man7.org/linux/man-pages/man2/fstat.2.html) — file descriptor dan I/O dasar.
- [pthread_create(3)](https://man7.org/linux/man-pages/man3/pthread_create.3.html) —
  signature, error (`EAGAIN`, `EINVAL`, `EPERM`), dan keterangan pustaka `-lpthread`.
- [pthreads(7)](https://www.man7.org/linux/man-pages/man7/pthreads.7.html) dan
  [nptl(7)](https://www.man7.org/linux/man-pages/man7/nptl.7.html) — gambaran implementasi
  pthreads di Linux (NPTL).

## POSIX

- [POSIX Issue 8 (IEEE Std 1003.1-2024)](https://pubs.opengroup.org/onlinepubs/9799919799/) —
  indeks utama.
- [pthread_create](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_create.html) —
  menyatakan fungsi mengembalikan *error number*, bukan mengatur `errno`.
- [pthread_join](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_join.html) —
  termasuk anjuran agar setiap thread yang joinable akhirnya di-join atau di-detach.
- [pthread_mutex_lock](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutex_lock.html) —
  tabel perilaku tipe mutex: NORMAL (relock = deadlock), ERRORCHECK (relock = error), RECURSIVE,
  DEFAULT.
- [pthread_mutexattr_settype](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_mutexattr_settype.html) —
  nilai default tipe mutex adalah `PTHREAD_MUTEX_DEFAULT`.
- [pthread_cond_broadcast / pthread_cond_signal](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_cond_broadcast.html) —
  penjelasan resmi tentang spurious wakeup dan kewajiban loop uji kondisi.

## Buku (sebagai rujukan konsep, bukan untuk disalin)

Materi ini **tidak** menyalin isi buku berbayar. Buku berikut dipakai sebagai rujukan konseptual
dan sangat disarankan sebagai bacaan pendamping.

- [The Linux Programming Interface (TLPI) — man7.org/tlpi](https://man7.org/tlpi/) — rujukan utama
  untuk process, memory, file, dan thread di Linux. Bab yang relevan: 6 (Processes), 7 (Memory
  Allocation), 24–27 (Process Creation/Termination/Monitoring/Execution), 49 (Memory Mappings),
  50 (Virtual Memory Operations). Daftar bab terverifikasi di
  [TLPI short TOC](https://man7.org/tlpi/toc-short.html).
- [Computer Systems: A Programmer's Perspective (CS:APP)](https://csapp.cs.cmu.edu/) — representasi
  data, memory hierarchy, linking, dan virtual memory dari sudut pandang programmer.
- [Beej's Guide to C Programming](https://beej.us/guide/bgc/) — pengantar C yang ramah pemula.
  Penulisnya sendiri menyebut dokumennya "beta-quality", jadi pakai untuk intuisi, bukan sebagai
  rujukan normatif. Bagian referensi library ada di [Beej's C Library Reference](https://beej.us/guide/bgclr/).

## Tool: compiler, sanitizer, debugger

- [GCC — Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) —
  `-fsanitize=address`, `-fsanitize=undefined`, `-fsanitize=thread`, `-fsanitize-address-use-after-scope`,
  dan variabel lingkungan `ASAN_OPTIONS`.
- [GCC — Warning Options](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html) —
  `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, `-Wshadow`.
- [Clang — AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) — penjelasan
  mendalam cara kerja ASan dan opsi runtime-nya.
- [Clang — ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) — deteksi data race.
- [Valgrind — Memcheck manual](https://valgrind.org/docs/manual/mc-manual.html) — pemakaian dan
  batasan Memcheck.
- [GDB documentation](https://sourceware.org/gdb/documentation/) — manual resmi;
  bagian [Memory](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Memory.html) menjelaskan
  perintah `x` untuk memeriksa memory, dan
  [Set Watchpoints](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Set-Watchpoints.html)
  menjelaskan `watch`.

## Catatan tentang sumber yang belum terverifikasi

Beberapa klaim populer di tutorial pihak ketiga **tidak** dipakai sebagai fakta di materi ini
karena tidak berhasil diverifikasi dari sumber primer saat penulisan:

- Angka pasti perlambatan eksekusi akibat Valgrind.
- Klaim bahwa ThreadSanitizer tidak bisa digabung dengan AddressSanitizer dalam satu build
  (sering disebut, tetapi tidak dikonfirmasi dari dokumen resmi dalam penulisan ini).
- Kutipan pasti tentang `PTHREAD_CREATE_JOINABLE` sebagai nilai default detachstate.

Modul yang bersangkutan menyebutkan hal ini secara eksplisit bila relevan, supaya kamu tahu mana
yang sudah pasti dan mana yang perlu kamu cek sendiri.
