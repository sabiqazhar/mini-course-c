/* Minggu 3 — Mengalokasikan satu int dengan malloc
 * Tujuan: pola dasar alokasi, pemeriksaan gagal, dan free.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_malloc_int 01_malloc_int.c
 * Run   : ./01_malloc_int
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *p = malloc(sizeof *p);   /* alokasikan tempat untuk satu int */
    if (p == NULL) {              /* malloc mengembalikan NULL bila gagal */
        fprintf(stderr, "alokasi gagal\n");
        return 1;
    }

    *p = 7;
    printf("nilai = %d, alamat = %p\n", *p, (void *)p);

    free(p);    /* kembalikan memory ke allocator */
    return 0;
}
