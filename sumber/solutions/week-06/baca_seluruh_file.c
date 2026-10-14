/* Minggu 6 — Solusi tantangan: baca seluruh file via mmap.
 *
 * Kontrak: pemanggil harus memanggil munmap(ptr, *out_len) setelah selesai, dengan
 * panjang yang SAMA dengan yang dikembalikan. Pada kegagalan, kembalikan NULL dan
 * ALL resource dibersihkan (tidak bocor).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o baca_seluruh_file baca_seluruh_file.c
 * Run  : ./baca_seluruh_file data.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Mengembalikan pointer ke isi file (read-only) dan panjang lewat out_len.
 * Return NULL + *out_len = 0 kalau gagal (dan tidak bocor). */
static char *baca_seluruh_file(const char *path, size_t *out_len)
{
    *out_len = 0;
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); return NULL; }

    if (st.st_size == 0) { close(fd); return NULL; }   /* mmap len=0 tidak valid */

    char *m = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);                              /* tutup fd segera setelah mmap */
    if (m == MAP_FAILED) return NULL;

    *out_len = (size_t)st.st_size;
    return m;
}

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "data.txt";
    size_t len = 0;
    char *buf = baca_seluruh_file(path, &len);
    if (buf == NULL) { fprintf(stderr, "gagal baca '%s'\n", path); return 1; }

    printf("dibaca %zu byte:\n%.*s\n", len, (int)len, buf);
    munmap(buf, len);                      /* kontrak: panjang sama */
    return 0;
}
