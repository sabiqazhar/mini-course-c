/* Minggu 6 — Solusi mudah: baca file via read() dan hitung jumlah baris ('\n').
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o read_file_lines read_file_lines.c
 * Run  : ./read_file_lines data.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "data.txt";
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    char buf[4096];
    ssize_t n;
    long lines = 0;
    while ((n = read(fd, buf, sizeof buf)) > 0)
        for (ssize_t i = 0; i < n; i++)
            if (buf[i] == '\n') lines++;

    if (n < 0) perror("read");
    close(fd);
    printf("file '%s' punya %ld baris ('\\n' yang ditemukan)\n", path, lines);
    return 0;
}
