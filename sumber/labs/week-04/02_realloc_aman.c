/* Minggu 4 — Pola realloc yang aman (pointer sementara)
 * Tujuan: menunjukkan bahwa setelah realloc berhasil, pointer lama bisa jadi tidak valid,
 *         dan bahwa realloc yang gagal tidak menghapus blok lama.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_realloc_aman 02_realloc_aman.c
 * Run   : ./02_realloc_aman
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t n = 3;
    int *p = malloc(n * sizeof *p);
    if (p == NULL) return 1;
    for (size_t i = 0; i < n; i++) p[i] = (int)i;

    printf("sebelum: p = %p\n", (void *)p);

    /* Salah:  p = realloc(p, baru);  <- bila gagal, blok lama bocor & p jadi NULL */
    int *tmp = realloc(p, 2 * n * sizeof *p);   /* benar: pakai tmp */
    if (tmp == NULL) {
        fprintf(stderr, "realloc gagal; blok lama tetap di p dan masih valid\n");
        free(p);
        return 1;
    }
    p = tmp;                              /* hanya setelah sukses */
    n = 2 * n;
    printf("sesudah : p = %p (bisa sama atau beda dengan sebelumnya)\n", (void *)p);

    for (size_t i = 0; i < n; i++) p[i] = (int)(i * 2);
    printf("p[5] = %d\n", p[5]);

    free(p);
    return 0;
}
