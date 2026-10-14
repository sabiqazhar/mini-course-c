/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #1: off-by-one heap overflow (menulis 1 elemen di luar blok)
 *
 * Gejala  : kadang tidak terlihat sama sekali, kadang nilai lain berubah,
 *           kadang program crash jauh dari sini.
 * Alat    : AddressSanitizer (-fsanitize=address)
 *
 * Build (versi polos, untuk melihat gejala tanpa alat):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_polos 01_heap_overflow_broken.c
 * Build (dengan ASan):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 01_asan 01_heap_overflow_broken.c
 * Run:
 *   ./01_polos
 *   ./01_asan
 *
 * Tebak dulu sebelum menjalankan: berapa elemen yang ditulis loop di bawah,
 * dan berapa elemen yang tersedia di blok yang diminta malloc?
 */
#include <stdio.h>
#include <stdlib.h>

#define N 8

static void isi_deret(int *buf, size_t n)
{
    /* BUG: kondisi loop memakai <= sehingga indeks terakhir adalah n,
     * padahal indeks valid hanya 0 .. n-1. */
    for (size_t i = 0; i <= n; i++) {
        buf[i] = (int)i * 10;
    }
}

int main(void)
{
    int *buf = malloc(N * sizeof *buf);
    if (buf == NULL) {
        return 1;
    }

    isi_deret(buf, N);

    for (size_t i = 0; i < N; i++) {
        printf("buf[%zu] = %d\n", i, buf[i]);
    }

    free(buf);
    return 0;
}
