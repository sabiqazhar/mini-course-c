# Minggu 6 — System Call, File I/O, `mmap`, dan Resource Lifetime

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** bagaimana program meminta layanan dari kernel dan mengakses file?

---

## 1. Mengapa ini penting

Di minggu 5 kamu melihat bahwa process terisolasi dan punya ruang alamat virtual. Untuk berinteraksi
dengan dunia luar — membaca file, mengalokasikan region memory baru — program harus **meminta kernel**
melalui *system call*. Minggu ini memperkenalkan `open`/`read`/`close`, konsep *file descriptor*, dan
`mmap`/`munmap`. Ini juga melatih kebiasaan krusial: **membersihkan resource (resource lifetime)** saat
operasi gagal di tengah jalan.

## 2. Learning objectives

Setelah minggu 6, kamu dapat:

1. Menjelaskan secara konseptual user space vs kernel space, dan library call vs system call.
2. Memakai `open`/`read`/`close` dengan penanganan error yang benar (`-1` + `errno`).
3. Menjelaskan apa itu file descriptor.
4. Memakai `mmap`/`munmap` dengan `MAP_PRIVATE`, proteksi memory, dan menangani kegagalan (`MAP_FAILED`).
5. Menjelaskan perbedaan `read`/`fread` vs memory mapping tanpa mengklaim satu selalu lebih cepat.
6. Membersihkan beberapa resource (fd, mapping) menurut hasil tiap operasi.

## 3. Prasyarat

Minggu 5 ([`week-05-process-virtual-memory.md`](week-05-process-virtual-memory.md)) — khususnya bahwa
virtual address space dimiliki process dan kernel mengelolanya.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Kamu ingin isi file tersedia di memory programmu. Cara paling langsung: baca byte demi byte lewat
`read`. Alternatif: peta file itu ke ruang alamatmu lewat `mmap`, lalu akses seperti array. Keduanya
melibatkan system call, tetapi mekanismenya berbeda.

### Analogi: meminjam buku dari perpustakaan

Kernel adalah pustakawan; process adalah pengunjung. `open` = minta nomor pinjaman (file descriptor).
`read` = pustakawan mengambil buku, menyalin halaman ke mejamu. `mmap` = pustakawan membolehkan kamu
membuka rak langsung dan membaca buku di tempatnya — halaman baru muncul saat kau benar-benar
membaliknya (demand paging). Kedua cara sah; mana lebih nyaman tergantung situasi, bukan aturan
mutlak.

### Batas analogi

`read` menyalin data; `mmap` memetakan halaman secara malas (hanya saat diakses). Analogi "rak langsung"
tidak berarti seluruh file langsung ada di RAM — kernel memuat halaman sesuai kebutuhan. Jangan berasumsi
`mmap` selalu lebih cepat; biayanya berbeda (setup page table vs penyalinan), dan tergantung pola akses.

### Penjelasan teknis

Program berjalan di **user space**; kernel di **kernel space**. Operasi berbahaya (akses hardware,
filesystem) hanya boleh kernel lakukan. **System call** adalah pintu terkontrol: `open`, `read`,
`write`, `mmap`, `close` adalah system call. Fungsi seperti `printf` *bukan* system call sendiri, tetapi
di bawahnya memanggil `write`. Satu library call tidak harus berujung tepat satu system call — jangan
menganggapnya begitu.

`open` mengembalikan **file descriptor** (int kecil) atau `-1` + `errno` saat gagal. `read(fd, buf, n)`
mengembalikan byte yang benar-benar dibaca (bisa kurang dari `n`, atau `0` di akhir file, atau `-1` +
`errno`). Selalu periksa nilai kembalian — jangan anggap `read` selalu membaca `n` byte.

[`mmap(2)`](https://man7.org/linux/man-pages/man2/mmap.2.html): `mmap(addr, length, prot, flags, fd, offset)`.
Sukses mengembalikan pointer ke region; **gagal mengembalikan `MAP_FAILED` (= `(void *)-1`) dan
mengatur `errno`**. `offset` harus kelipatan ukuran page (`sysconf(_SC_PAGE_SIZE)`). Sejak Linux 2.6.12,
`mmap` dengan `length = 0` gagal dengan `EINVAL`. Setelah `mmap` sukses, `fd` boleh ditutup — pemetaan
tetap valid. `munmap` melepas pemetaan (`-1` + `errno` bila gagal, biasanya `EINVAL`).

### Diagram

```
read():                              mmap():
 program buffer <── salin ── kernel  program memory <── akses langsung (halaman dimuat saat dibaca)
 (penyalinan eksplisit)              (pemetaan malas / demand paging)
 fd menunjuk file di kernel          fd boleh ditutup setelah mmap; mapping tetap ada
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-06/02_mmap_file.c`](../labs/week-06/02_mmap_file.c) (versi ringkas)

```c
int fd = open(argv[1], O_RDONLY);
if (fd < 0) { perror("open"); return 1; }

struct stat st;
if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }

if (st.st_size == 0) {                     /* mmap butuh length > 0 */
    fprintf(stderr, "file kosong\n"); close(fd); return 1;
}

char *m = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
if (m == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

close(fd);                                /* aman: mapping tetap valid */

for (off_t i = 0; i < st.st_size; i++)    /* akses seperti array */
    putchar(m[i]);

if (munmap(m, (size_t)st.st_size) < 0) { perror("munmap"); return 1; }
```

Perhatikan **urutan cleanup**: jika `fstat` gagal, tutup `fd`; jika `mmap` gagal, tutup `fd`; jika
sukses, `fd` sudah ditutup sebelum akses. Tidak ada jalur di mana `fd` bocor.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-06
printf 'Halo dari mmap\n' > data.txt
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_mmap_file 02_mmap_file.c
./02_mmap_file data.txt
```

### Hasil yang diharapkan

```
Halo dari mmap
```

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — membaca file lewat read (10 menit)

File [`01_read_file.c`](../labs/week-06/01_read_file.c):

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_read_file 01_read_file.c
./01_read_file data.txt
```

Mencetak isi `data.txt`. Perhatikan loop `while ((n = read(...)) > 0)`: kita tidak asumsi sekali baca
selesai. Itu penanganan yang benar karena `read` bisa mengembalikan kurang dari `sizeof buf`.

### Eksperimen 2 — memetakan file lewat mmap (10 menit)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_mmap_file 02_mmap_file.c
./02_mmap_file data.txt
```

Sama-sama mencetak isi file, tetapi kali ini lewat pemetaan memory. Ubah `putchar(m[i])` menjadi
`printf("karakter ke-%ld = %c\n", (long)i, m[i])` untuk melihat bahwa `m` memang array biasa.

### Eksperimen 3 — kegagalan mmap pada file kosong (10 menit)

File `03_mmap_zero_broken.c` adalah **contoh rusak** yang sengaja memanggil `mmap` dengan panjang 0:

```bash
: > kosong.txt
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_mmap_zero 03_mmap_zero_broken.c
./03_mmap_zero kosong.txt
```

Hasil di mesin penulis:

```
mmap(length=0) gagal seperti yang diharapkan: Invalid argument (errno=22)
```

Ini mengajarkan: selalu periksa ukuran file dan hasil `mmap`. `MAP_FAILED` adalah cara `mmap` melaporkan
gagal, persis seperti `NULL` bagi `malloc`.

---

## 7. Kesalahan umum

- **Lupa memeriksa `-1` dari `open`/`read`.** Selalu cek; `read` bisa mengembalikan kurang dari minta
  atau `-1`.
- **Mengira `read` selalu membaca `n` byte.** Gunakan loop sampai `0` (EOF) atau `-1`.
- **Menutup `fd` sebelum selesai, atau lupa menutupnya.** `close` tepat sekali; setelah `mmap`, `fd`
  boleh ditutup.
- **Mengabaikan `MAP_FAILED`.** Bandingkan dengan `MAP_FAILED`, bukan `NULL`.
- **Berasumsi `mmap` lebih cepat.** Ia punya trade-off; jangan jadikan klaim umum.
- **Mengira seluruh file langsung di RAM setelah `mmap`.** Halaman dimuat saat diakses (demand paging).

---

## 8. Latihan

### Mudah
Buat file teks, lalu modifikasi `01_read_file.c` agar menghitung jumlah baris (hitung `'\n'`).

### Menengah
Tulis program yang memetakan file lewat `mmap`, mengubah satu karakter di buffer, lalu `munmap`. Jelaskan
mengapa perubahan tidak kembali ke file saat `MAP_PRIVATE` (hint: copy-on-write). Coba ulangi dengan
`MAP_SHARED` dan amati isi file setelah `msync`/`munmap` (hati-hati: ini mengubah file asli!).

### Tantangan
Tulis fungsi `char *baca_seluruh_file(const char *path, size_t *out_len)` yang membuka file, `fstat`,
`mmap` dengan `PROT_READ|PROT_WRITE`? (tidak, gunakan `PROT_READ`), lalu mengembalikan pointer. Dokumentasikan
bahwa pemanggil harus `munmap` dengan panjang yang sama. Tangani semua kegagalan dengan cleanup yang
benar. Petunjuk bertahap: (1) `open` + cek; (2) `fstat` + cek + tutup fd bila gagal; (3) `mmap` + cek
+ tutup fd; (4) kembalikan pointer; (5) tulis kontrak `munmap` di dokumentasi.

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan beda user space dan kernel space secara konseptual.
- [ ] Saya tahu `open`/`read` mengembalikan `-1` + `errno`, bukan `NULL`.
- [ ] Saya paham apa itu file descriptor.
- [ ] Saya memeriksa `MAP_FAILED` dan tahu `fd` boleh ditutup setelah `mmap` sukses.
- [ ] Saya bisa menjelaskan perbedaan `read` dan `mmap` tanpa mengklaim satu selalu lebih cepat.
- [ ] Saya membersihkan fd/mapping di setiap jalur error.

---

## 10. Ringkasan

System call adalah pintu ke kernel: `open`/`read`/`close` lewat file descriptor, `mmap`/`munmap` untuk
memetakan region memory. Selalu periksa nilai kembalian (`-1`+`errno` untuk fd, `MAP_FAILED` untuk mmap),
dan bersihkan setiap resource sesuai hasil operasi. `mmap` memetakan halaman secara malas; jangan anggap
seluruh file langsung di RAM, dan jangan anggap ia selalu lebih cepat dari `read`.

**Glosarium mini:** system call, user space, kernel space, file descriptor, memory mapping, `MAP_FAILED`.
Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [open(2) — Linux man-pages](https://man7.org/linux/man-pages/man2/open.2.html) — return `-1` + `errno`.
- [read(2) — Linux man-pages](https://man7.org/linux/man-pages/man2/read.2.html) — dapat mengembalikan kurang dari diminta.
- [mmap(2) — Linux man-pages](https://man7.org/linux/man-pages/man2/mmap.2.html) — `MAP_FAILED = (void *)-1`, `offset` kelipatan page, `EINVAL` bila `length = 0`.
- [The Linux Programming Interface — TLPI](https://man7.org/tlpi/) — ch. 4 (Files), ch. 49 (Memory Mappings).

## 12. Jembatan ke minggu 7

Sekarang kamu bisa meminta kernel membaca file dan memetakan memory. Langkah berikutnya: **bagaimana
satu process menjalankan beberapa alur eksekusi sekaligus?** Di situ threads dan race condition masuk,
fondasi untuk sinkronisasi minggu 8. Lanjut:
[`week-07-threads-races.md`](week-07-threads-races.md)
