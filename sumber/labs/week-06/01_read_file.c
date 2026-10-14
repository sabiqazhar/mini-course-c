/* Minggu 6 — Membaca file dengan open/read/write (system call + library)
 * Tujuan: melihat penggunaan file descriptor, penanganan error, dan cleanup fd.
 *
 * Buat file uji dulu:  printf 'Halo dari file\n' > data.txt
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_read_file 01_read_file.c
 * Run   : ./01_read_file data.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "pakai: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {                       /* open gagal => -1 dan errno */
        perror("open");
        return 1;
    }

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0) {
        if (write(STDOUT_FILENO, buf, (size_t)n) != n) {
            perror("write");
            break;
        }
    }
    if (n < 0)
        perror("read");

    if (close(fd) < 0)                  /* selalu tutup fd */
        perror("close");

    return 0;
}
