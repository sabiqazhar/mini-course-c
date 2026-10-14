/* Solusi latihan menengah minggu 2 — menjelaskan padding lewat offset
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o struct_offsets struct_offsets.c
 * Run   : ./struct_offsets
 */
#include <stdio.h>
#include <stddef.h>

struct Contoh { char c; int i; short s; };

int main(void)
{
    printf("sizeof(struct Contoh) = %zu\n", sizeof(struct Contoh));
    printf("offset c = %zu\n", offsetof(struct Contoh, c));   /* 0  */
    printf("offset i = %zu\n", offsetof(struct Contoh, i));   /* 4  */
    printf("offset s = %zu\n", offsetof(struct Contoh, s));   /* 8  */

    /* Mengapa sizeof = 12, bukan 1 + 4 + 2 = 7?
     * - c  di 0 (1 byte)
     * - padding 3 byte agar i (butuh align 4) berada di 4
     * - i  di 4 (4 byte)
     * - s  di 8 (2 byte)
     * - padding 2 byte di akhir agar struct sendiri align 4 (total kelipatan 4)
     * Urutan field memengaruhi padding, jadi memengaruhi ukuran. */
    return 0;
}
