/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Use-after-free: mengakses memory SETELAH ia dilepas dengan free.
 *
 * Kompilasi dengan AddressSanitizer untuk melihat laporannya:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address \
 *       -o 05_uaf 05_uaf_broken.c
 *   ./05_uaf
 *
 * Tanpa sanitizer, program mungkin saja mencetak 100 lagi, atau nilai lain,
 * atau crash. Semuanya adalah undefined behavior.
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *p = malloc(sizeof *p);
    *p = 100;
    free(p);              /* memory dikembalikan ke allocator */

    printf("%d\n", *p);   /* UNDEFINED BEHAVIOR: akses setelah free */
    return 0;
}
