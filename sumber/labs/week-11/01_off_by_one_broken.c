/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Off-by-one: loop menulis ke indeks n (satu lewat batas) dari blok malloc(n).
 * Jalankan di bawah AddressSanitizer untuk melihatnya:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 01_off_by_one 01_off_by_one_broken.c
 *   ./01_off_by_one
 *
 * Tanpa sanitizer, program bisa saja selesai "normal" — itulah bahayanya.
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t n = 4;
    int *a = malloc(n * sizeof *a);
    if (a == NULL) return 1;

    for (size_t i = 0; i <= n; i++)     /* BUG: kondisi harus i < n */
        a[i] = (int)i;                   /* a[n] melanggar batas */

    for (size_t i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n");

    free(a);
    return 0;
}
