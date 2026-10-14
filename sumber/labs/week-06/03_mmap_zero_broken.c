/* CONTOH RUSAK (tetapi instruktif) — mmap file kosong
 *
 * Linux menolak mmap dengan length = 0 (sejak 2.6.12: EINVAL). Ini menunjukkan
 * bahwa kita HARUS memeriksa ukuran file dan hasil mmap, bukan menganggap berhasil.
 *
 * Buat file kosong dulu:  : > kosong.txt
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_mmap_zero 03_mmap_zero_broken.c
 * Run   : ./03_mmap_zero kosong.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "pakai: %s <file>\n", argv[0]); return 1; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }

    /* Sengaja mencoba mmap panjang 0 untuk melihat kegagalannya. */
    char *m = mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0);
    if (m == MAP_FAILED) {
        printf("mmap(length=0) gagal seperti yang diharapkan: %s (errno=%d)\n",
               strerror(errno), errno);
        close(fd);
        return 1;
    }

    /* Kalau suatu implementasi mengembalikan pointer valid, bersihkan. */
    munmap(m, 0);
    close(fd);
    return 0;
}
