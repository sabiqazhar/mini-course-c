/* Minggu 6 — Memetakan file ke memory dengan mmap (MAP_PRIVATE, PROT_READ)
 * Tujuan: melihat bahwa setelah mmap, isi file diakses seperti array biasa,
 *         dan bahwa fd boleh ditutup setelah pemetaan berhasil.
 *
 * Buat file uji (bukan kosong):  printf 'Halo dari mmap\n' > data.txt
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_mmap_file 02_mmap_file.c
 * Run   : ./02_mmap_file data.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "pakai: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }

    if (st.st_size == 0) {
        fprintf(stderr, "file kosong: mmap butuh panjang > 0 (EINVAL di Linux)\n");
        close(fd);
        return 1;
    }

    char *m = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (m == MAP_FAILED) {              /* mmap gagal => MAP_FAILED = (void*)-1, errno set */
        perror("mmap");
        close(fd);
        return 1;
    }

    /* fd boleh ditutup SETELAH mmap berhasil; pemetaan tetap valid. */
    close(fd);

    /* Akses isi file seperti array biasa. Halaman diisi saat diakses (demand paging). */
    for (off_t i = 0; i < st.st_size; i++)
        putchar(m[i]);

    if (munmap(m, (size_t)st.st_size) < 0) {  /* lepas pemetaan */
        perror("munmap");
        return 1;
    }
    return 0;
}
