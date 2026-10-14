# Low-Level Programming dengan C — Materi Belajar 12 Minggu

Selamat datang. Materi ini ditulis untuk satu tipe pembelajar yang cukup spesifik: kamu sudah
bisa menulis sedikit C, bisa membaca syntax dasar, dan mungkin sudah nyaman sebagai backend
engineer — tetapi setiap kali bertemu `*p`, `malloc`, `free`, stack/heap, atau `pthread`,
rasanya seperti membaca bagian yang hilang dari buku manual.

Kabar baiknya: tidak ada yang mistis di sini. Semua konsep itu bisa dijelaskan, bisa diamati
secara langsung lewat eksperimen kecil, dan yang paling penting — bisa kamu *buktikan sendiri*
di terminal.

## Apa yang akan kamu dapatkan

Setelah 12 minggu (sekitar 5–7 jam per minggu), targetnya adalah:

1. Kamu bisa menjelaskan dengan kata-katamu sendiri perbedaan nilai, objek, alamat, pointer, dan dereference.
2. Kamu paham kapan sebuah objek hidup, kapan ia mati, dan siapa yang bertanggung jawab melepasnya.
3. Kamu bisa memakai `malloc`/`calloc`/`realloc`/`free` dengan benar, termasuk jalur error-nya.
4. Kamu bisa mengenali dan menjelaskan memory leak, use-after-free, double free, out-of-bounds, dangling pointer, dan undefined behavior.
5. Kamu memahami secara konseptual apa yang dilakukan Linux terhadap alamat yang kamu cetak: virtual address space, page, page fault, dan system call.
6. Kamu bisa memakai API Linux yang relevan, termasuk `mmap`, dengan cleanup yang benar.
7. Kamu bisa membuat thread dengan pthreads, menjelaskan race condition, dan melindungi data dengan mutex atau atomic yang tepat.
8. Kamu bisa memakai compiler warning, GDB, AddressSanitizer, UndefinedBehaviorSanitizer, ThreadSanitizer, dan Valgrind untuk mencari bug — bukan menebak-nebak.
9. Kamu menyelesaikan satu proyek akhir: bounded thread-safe queue dengan ownership dan cleanup yang jelas.
10. Kamu terbiasa membaca dokumentasi primer (man page, POSIX, cppreference)而不是 menggantungkan diri pada tutorial pihak ketiga.

## Prasyarat

Yang kamu butuhkan:

- Mesin Linux (materi ini ditulis dan diuji di Linux; sebagian besar kode C murni tetap jalan di sistem lain, tetapi modul 5, 6, dan sebagian 11–12 spesifik Linux).
- `gcc` atau `clang`, `make`, dan `gdb`. Panduan instalasi ada di [`setup.md`](setup.md).
- Kemampuan menulis fungsi, memanggil fungsi, memakai `if`/`for`, dan mengompilasi program dari terminal.
- Kesediaan untuk menebak output program *sebelum* menjalankannya. Ini bagian yang paling penting.

Yang **tidak** perlu kamu kuasai dulu: assembly, kernel, matematika, atau pengalaman systems programming.
Materi ini tidak mengasumsikan itu, dan tidak akan mengujinya.

## Cara memakai materi ini

Urutan yang disarankan:

1. Baca [`setup.md`](setup.md), siapkan toolchain, dan pastikan perintah kompilasi dasar jalan.
2. Baca [`plan.md`](plan.md) untuk melihat peta 12 minggu dan checkpoint-nya.
3. Mulai dari `modules/week-01-memory-and-pointers.md`. Setiap modul punya satu ide utama — jangan lompat.
4. Kerjakan **lab** di `labs/week-NN/`. Jangan hanya membaca kode: ketik ulang, ubah, rusak, perbaiki.
5. Coba latihan dulu sebelum membuka `solutions/`. Solusi sengaja dipisah supaya kamu punya kesempatan untuk salah — salah di sini murah dan berguna.
6. Akhiri setiap minggu dengan *checklist pemahaman* di modul. Kalau lebih dari satu poin belum kamu yakini, ulangi minggu itu. Kalender bukan aturan; pemahaman kamu yang jadi aturan.

### Perintah kompilasi standar

Hampir semua contoh di materi ini dikompilasi dengan pola berikut:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o program program.c
```

Arti singkatnya: `-std=c11` memilih versi bahasa, `-Wall -Wextra -Wpedantic` menyalakan peringatan,
`-g` menyimpan informasi debugging, `-O0` mematikan optimisasi supaya perilaku program lebih mudah
ditebak saat eksperimen.

Tambahan yang dipakai di modul tertentu:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o program program.c          # pthreads
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o p p.c  # deteksi memory bug
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fsanitize=thread -pthread -o p p.c    # deteksi data race
```

Kalau sebuah modul memakai flag yang berbeda, modul tersebut menyebutkannya secara eksplisit.

### Konvensi "contoh rusak"

Kadang kita perlu melihat program yang salah untuk memahami mengapa ia salah. Contoh seperti ini
selalu diberi judul yang mengandung kata **contoh rusak** (atau nama file berakhiran `_broken.c`)
dan tidak pernah dicampur dengan contoh yang aman. Jangan jadikan pola di file tersebut sebagai
template kode sehari-hari.

## Peta kurikulum

| Minggu | File | Pertanyaan utama | Ide inti |
|---|---|---|---|
| 1 | [`week-01-memory-and-pointers.md`](modules/week-01-memory-and-pointers.md) | Apa bedanya nilai, alamat, dan pointer? | Variabel sebagai objek, `&`, `*`, tipe pointer |
| 2 | [`week-02-arrays-structs-lifetime.md`](modules/week-02-arrays-structs-lifetime.md) | Di mana objek hidup, kapan ia tidak valid? | Array, struct, scope vs lifetime, stack, alignment |
| 3 | [`week-03-heap-malloc-free.md`](modules/week-03-heap-malloc-free.md) | Bagaimana objek hidup melampaui satu pemanggilan fungsi? | `malloc`/`calloc`/`free`, ownership, leak, UAF |
| 4 | [`week-04-realloc-dynamic-array.md`](modules/week-04-realloc-dynamic-array.md) | Bagaimana struktur data berubah ukuran tanpa merusak memory? | `realloc`, dynamic array, ASan/UBSan/Valgrind |
| 5 | [`week-05-process-virtual-memory.md`](modules/week-05-process-virtual-memory.md) | Apakah alamat yang dicetak itu alamat fisik RAM? | Process, virtual address space, page, ASLR, `/proc/self/maps` |
| 6 | [`week-06-system-calls-mmap.md`](modules/week-06-system-calls-mmap.md) | Bagaimana program meminta layanan kernel dan membaca file? | System call, file descriptor, `mmap`/`munmap` |
| 7 | [`week-07-threads-races.md`](modules/week-07-threads-races.md) | Apa yang berubah saat dua alur eksekusi memakai data yang sama? | `pthread_create`/`join`, interleaving, data race |
| 8 | [`week-08-mutex-atomics.md`](modules/week-08-mutex-atomics.md) | Bagaimana thread bekerja sama tanpa merusak invariant? | Mutex, critical section, deadlock, atomic, bounded queue |
| 9 | [`week-09-data-structures-ownership.md`](modules/week-09-data-structures-ownership.md) | Bagaimana pointer membangun struktur data dinamis? | Linked list, kontrak ownership, ring buffer |
| 10 | [`week-10-memory-allocator.md`](modules/week-10-memory-allocator.md) | Apa yang dilakukan allocator saat kita minta memory? | Arena/bump allocator, free list, fragmentasi |
| 11 | [`week-11-debugging-tools.md`](modules/week-11-debugging-tools.md) | Bagaimana menemukan bug yang tidak terlihat dari output? | GDB, sanitizer, Valgrind, membaca stack trace |
| 12 | [`week-12-final-project.md`](modules/week-12-final-project.md) | Bisakah kamu menggabungkan semuanya? | Proyek akhir: bounded thread-safe queue |

Checkpoint ada setelah minggu 2, 4, 6, 8, dan 12. Detailnya di [`plan.md`](plan.md).

## Cara materi ini membedakan level abstraksi

Satu hal yang sering bikin bingung pemula: tidak semua yang kamu lihat di Linux itu "aturan C".
Materi ini konsisten memisahkan empat level:

- **Bahasa C (standar)**: apa yang dijamin atau tidak dijamin oleh standar C. Ini yang paling portabel.
- **POSIX**: API seperti pthreads, `mmap`, `open`. Standar untuk sistem mirip Unix, bukan bagian dari bahasa C.
- **Linux / GNU/Linux**: misalnya `/proc/self/maps`, ASLR lewat sysctl, perilaku glibc. Spesifik Linux.
- **Compiler/arsitektur**: misalnya bagaimana GCC menyusun struct di x86-64. Bergantung implementasi.

Kalau sebuah fakta hanya benar di Linux x86-64, materi ini akan mengatakannya. Anggap itu fitur,
bukan gangguan: kebiasaan memisahkan level abstraksi ini justru inti dari systems programming.

## Tentang verifikasi kode

Semua program di folder `labs/` ditulis untuk benar-benar dikompilasi dan dijalankan, bukan sekadar
terlihat masuk akal. Lingkungan yang dipakai saat materi ini ditulis:

- Ubuntu 24.04.3 LTS, kernel 6.6 (x86_64)
- GCC 13.3.0 dan Clang 18.1.3
- GDB 15.0.50
- AddressSanitizer, UndefinedBehaviorSanitizer, dan ThreadSanitizer: tersedia dan diuji
- Valgrind: **tidak terpasang** di lingkungan penulisan, jadi perintah Valgrind di modul 4 dan 11
  diberikan sebagai panduan yang perlu kamu jalankan sendiri (cara pasang ada di `setup.md`)

Beberapa hal memang tidak bisa dijanjikan, dan materi ini akan jujur menyebutkannya: alamat memory
berubah setiap kali program dijalankan (karena ASLR), output program yang punya data race tidak
deterministik, dan ukuran struct bisa berbeda di arsitektur lain. Setiap kali output bersifat
demikian, modul yang bersangkutan menuliskannya secara eksplisit.

## Definisi selesai

Kamu boleh menyebut diri sudah menyelesaikan kursus ini kalau:

- Kamu sudah mengerjakan (bukan hanya membaca) lab tiap minggu.
- Kamu lolos acceptance criteria di tiap modul, dan bisa menjelaskan jawabannya dengan kata-katamu sendiri.
- Proyek akhir di minggu 12 berjalan di Linux dengan instruksi build yang jelas, bersih dari data race
  pada skenario yang diuji, dan setiap resource punya jalur cleanup.
- Kamu bisa membuka `man 2 mmap` atau halaman POSIX dan memahami isinya tanpa perlu tutorial perantara.

Selamat belajar. Kalau satu minggu terasa lambat, itu normal — minggu 1 sampai 4 adalah fondasi,
dan fondasi yang pelan jauh lebih murah daripada fondasi yang salah.
