/* Minggu 3 — free(NULL) aman
 * Tujuan: menunjukkan bahwa memanggil free dengan NULL adalah no-op,
 *         sehingga pola "free lalu set NULL" aman dan idiom lazim.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 06_free_null 06_free_null.c
 * Run   : ./06_free_null
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *p = NULL;

    free(p);                 /* aman: POSIX menyatakan free(NULL) tidak melakukan apa-apa */
    printf("free(NULL) aman: tidak ada yang terjadi\n");

    p = malloc(sizeof *p);
    if (p) {
        *p = 1;
        free(p);
        p = NULL;            /* kebiasaan baik: jangan biarkan pointer menunjuk ke memori bebas */
    }
    return 0;
}
