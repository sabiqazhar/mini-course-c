# Setup Lingkungan Belajar

Modul ini tidak butuh banyak alat, tetapi butuh alat yang tepat dan bisa dipakai dari terminal.
Targetnya: setelah selesai mengikuti halaman ini, kamu bisa mengompilasi, menjalankan,
meng-*debug*, dan memeriksa memory sebuah program C dari satu direktori kerja.

## 1. Periksa dulu apa yang sudah ada

```bash
gcc --version
gdb --version
make --version
```

Kalau ketiganya mencetak versi, kamu sudah hampir siap. Kalau `gcc` tidak dikenali, lanjut ke bagian 2.

## 2. Pasang compiler dan tool dasar

Pilih yang sesuai distro kamu. Perintah untuk Ubuntu/Debian paling umum dipakai; sisanya ditandai
supaya kamu tidak menjalankan perintah yang salah.

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install -y build-essential gdb make
```

Fedora / RHEL:

```bash
sudo dnf install -y gcc gdb make
```

Arch Linux:

```bash
sudo pacman -S --needed base-devel gdb
```

`build-essential` di Ubuntu/Debian berisi GCC, `libc6-dev` (header C), dan `make`. Tanpa paket
`*-dev`, kompilasi akan gagal dengan pesan seperti `stdio.h: No such file or directory`.

Clang bersifat opsional, tapi berguna karena kadang memberi peringatan yang berbeda dari GCC:

```bash
sudo apt install -y clang      # Ubuntu/Debian
```

## 3. Pasang Valgrind (opsional, dipakai minggu 4 dan 11)

```bash
sudo apt install -y valgrind   # Ubuntu/Debian
sudo dnf install -y valgrind   # Fedora
sudo pacman -S valgrind        # Arch
```

Verifikasi dengan `valgrind --version`. Kalau Valgrind tidak bisa dipasang, tidak apa-apa:
AddressSanitizer dan LeakSanitizer dari GCC/Clang menutupi sebagian besar kebutuhan modul 4 dan 11,
dan setiap modul menyebutkan alternatifnya.

Catatan jujur: Valgrind tidak terpasang di lingkungan tempat materi ini ditulis, sehingga
contoh output Valgrind di materi ini **belum dijalankan**. Contoh output AddressSanitizer,
UndefinedBehaviorSanitizer, dan ThreadSanitizer **sudah dijalankan** di Ubuntu 24.04 dengan GCC 13.3.

## 4. Perintah kompilasi yang dipakai di seluruh materi

Simpan sebagai kebiasaan. Flag ini menangkap banyak kesalahan sebelum program dijalankan:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o program program.c
```

| Flag | Fungsi singkat |
|---|---|
| `-std=c11` | Memilih edisi bahasa C secara eksplisit (C11 jadi baseline materi ini) |
| `-Wall -Wextra` | Menyalakan peringatan umum dan tambahan |
| `-Wpedantic` | Mengingatkan saat kamu memakai ekstensi di luar standar |
| `-g` | Menyimpan informasi debugging untuk GDB dan sanitizer |
| `-O0` | Mematikan optimisasi, supaya perilaku program mudah ditebak saat eksperimen |
| `-o program` | Nama file hasil kompilasi |

Tambahan per modul:

```bash
-pthread                      # wajib untuk pthreads (compile dan link)
-fsanitize=address,undefined  # minggu 3, 4, 11: deteksi memory bug dan UB
-fsanitize=thread             # minggu 7, 8: deteksi data race
-D_GNU_SOURCE                 # hanya bila modul menyebutnya, untuk ekstensi glibc tertentu
```

## 5. Direktori kerja dan Makefile kecil

Buat satu direktori untuk latihan:

```bash
mkdir -p ~/belajar-c && cd ~/belajar-c
```

Untuk minggu 4 sampai 12, kode mulai terdiri dari beberapa file. Makefile berikut cukup untuk
sebagian besar latihan; simpan sebagai `Makefile` di direktori latihanmu.

```make
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS := -pthread

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o
```

Pakai dengan `make namafile` lalu jalankan `./namafile`. Ingat: baris perintah di dalam Makefile
**harus** diawali karakter tab, bukan spasi. Kalau `make` mengeluh `missing separator`, itu
penyebabnya.

## 6. Troubleshooting yang paling sering muncul

`undefined reference to 'pthread_create'`
Kamu lupa `-pthread`. Tambahkan flag itu di perintah compile:
`gcc -std=c11 -Wall -Wextra -g -pthread -o program program.c`

`stdio.h: No such file or directory`
Header development belum terpasang. Pasang `build-essential` (Debian) atau `glibc-devel` (Fedora).

`Permission denied` saat menjalankan `./program`
File hasil kompilasi belum punya bit execute, atau kamu menulis output ke direktori yang tidak
bisa dieksekusi (misalnya partisi yang di-mount `noexec`). Coba `chmod +x ./program`; kalau masih
gagal, pindah ke direktori home.

Program langsung mati tanpa pesan saat dipakai bersama sanitizer
Jalankan dengan `ASAN_OPTIONS=detect_leaks=1 ./program` untuk melihat laporan kebocoran, atau
baca bagian sanitizer di [minggu 11](modules/week-11-debugging-tools.md).

`AddressSanitizer: DEADLYSIGNAL` atau tidak ada simbol fungsi di laporan
Kamu lupa `-g`, atau frame pointer dihilangkan optimisasi. Tambahkan `-g -O0`, dan bila perlu
`-fno-omit-frame-pointer`.

GDB mencetak `No symbol table info loaded`
Sama: kompilasi ulang dengan `-g`.

Valgrind melaporkan banyak hal di dalam libc
Itu wajar; yang perlu kamu perhatikan adalah baris yang menyebut file dan fungsi **milikmu**.

## 7. Alat baca dokumentasi

Bagian dari tujuan kursus ini adalah membiasakan diri membaca sumber primer. Biasakan tiga ini:

```bash
man 2 mmap         # manual system call (bagian 2)
man 3 malloc       # manual fungsi library (bagian 3)
man 7 pthreads     # penjelasan topik (bagian 7)
```

Kalau `man` tidak menampilkan apa-apa atau bilang tidak ada entri, pasang `man-db` dan
`manpages-dev`:

```bash
sudo apt install -y man-db manpages-dev manpages-posix-dev
```

Buku manual juga tersedia daring di [man7.org](https://man7.org/linux/man-pages/), yang berguna
kalau kamu ingin menyalin tautan saat mencatat.

## 8. Kebiasaan kecil yang menghemat banyak waktu

Selalu baca peringatan compiler, jangan abaikan hanya karena program tetap jalan. Simpan setiap
eksperimen dalam file terpisah dengan nama yang jelas. Tulis tebakan output di kertas atau komentar
sebelum menjalankan program — kebiasaan ini yang paling cepat mengubah "saya pernah membaca
tentang pointer" menjadi "saya paham pointer".

Kalau sudah siap, buka [`plan.md`](plan.md) lalu mulai minggu pertama.
