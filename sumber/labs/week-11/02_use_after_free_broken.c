/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #2: use-after-free (memakai memory setelah dilepas)
 *
 * Gejala  : program bisa mencetak nilai yang benar, nilai sampah, atau crash.
 *           Yang paling berbahaya: ia sering "berhasil" di mesin pengembang.
 * Alat    : AddressSanitizer (-fsanitize=address)
 *
 * Build (polos) : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_polos 02_use_after_free_broken.c
 * Build (ASan)  : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 02_asan 02_use_after_free_broken.c
 * Run           : ./02_polos   lalu   ./02_asan
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *nilai = malloc(sizeof *nilai);
    if (nilai == NULL) {
        return 1;
    }

    *nilai = 42;
    printf("sebelum free : *nilai = %d\n", *nilai);

    free(nilai);   /* sejak baris ini, nilai adalah dangling pointer */

    /* BUG: nilai dipakai lagi setelah bloknya dilepas. */
    printf("sesudah free : *nilai = %d\n", *nilai);

    return 0;
}
