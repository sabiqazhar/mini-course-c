/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Perkalian ukuran alokasi dapat OVERFLOW pada tipe size_t. Di mesin 64-bit,
 * size_t maksimum adalah sekitar 1.8e19, tetapi jika faktor pertamanya cukup
 * besar, n * sizeof(int) bisa "meluap" (wrap) menjadi angka kecil.
 *
 * Di sini n = 2^62. Karena int = 4 byte, n * 4 = 2^64, yang pada size_t 64-bit
 * meluap menjadi 0. Maka malloc(0) dipanggil, bukan malloc(besar).
 *
 * Pelajaran: jangan pernah menulis `malloc(n * sizeof(T))` tanpa memeriksa
 * apakah n * sizeof(T) meluap. Idiom yang aman ada di bawah.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_overflow 03_overflow_broken.c
 * Run   : ./03_overflow
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void)
{
    size_t n = (size_t)1 << 62;     /* 2^62, sangat besar */
    size_t bytes = n * sizeof(int);  /* LUAPAN: 2^62 * 4 = 2^64 -> wrap jadi 0 */

    printf("n = %zu\n", n);
    printf("bytes yang diminta = %zu  (seharusnya ~4.6e18, tapi meluap jadi 0!)\n", bytes);

    int *p = malloc(bytes);          /* malloc(0): mengembalikan pointer valid, BUKAN gagal */
    if (p == NULL) {
        printf("malloc(0) mengembalikan NULL\n");
        return 1;
    }
    printf("malloc(0) mengembalikan pointer bukan-NULL: %p\n", (void *)p);
    printf("Menulis p[1000] melanggar batas blok sekecil ini (undefined behavior).\n");

    free(p);
    return 0;
}
