/* Minggu 2 — Padding dan alignment pada struct
 * Tujuan: membuktikan bahwa sizeof(struct) bisa lebih besar dari jumlah sizeof field,
 *         dan bahwa urutan field memengaruhi ukuran.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_struct_padding 02_struct_padding.c
 * Run   : ./02_struct_padding
 */
#include <stdio.h>
#include <stddef.h>

struct A { char c; int  i; };   /* char lalu int: ada padding setelah c */
struct B { int  i; char c; };   /* int lalu char: padding di akhir      */
struct C { char c1; char c2; int i; };

int main(void)
{
    printf("struct A : sizeof = %zu, offset c = %zu, offset i = %zu\n",
           sizeof(struct A), offsetof(struct A, c), offsetof(struct A, i));
    printf("struct B : sizeof = %zu, offset i = %zu, offset c = %zu\n",
           sizeof(struct B), offsetof(struct B, i), offsetof(struct B, c));
    printf("struct C : sizeof = %zu, offset c1 = %zu, offset c2 = %zu, offset i = %zu\n",
           sizeof(struct C), offsetof(struct C, c1),
           offsetof(struct C, c2), offsetof(struct C, i));

    return 0;
}
