/* Minggu 3 — Array dinamis dengan calloc
 * Tujuan: alokasi banyak objek sekaligus, isi, dan bebaskan.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_array_dinamis 02_array_dinamis.c
 * Run   : ./02_array_dinamis
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t n = 4;

    /* calloc(n, size) mengalokasikan n * size byte dan MENGOSONGKAN (nol) isinya. */
    int *a = calloc(n, sizeof *a);
    if (a == NULL) {
        fprintf(stderr, "calloc gagal\n");
        return 1;
    }

    for (size_t i = 0; i < n; i++)
        a[i] = (int)(i + 1) * 10;

    for (size_t i = 0; i < n; i++)
        printf("a[%zu] = %d\n", i, a[i]);

    free(a);    /* satu panggilan cukup untuk seluruh array */
    return 0;
}
