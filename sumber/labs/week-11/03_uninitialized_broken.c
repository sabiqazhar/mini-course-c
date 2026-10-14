/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Membaca variabel yang tidak diinisialisasi. Nilainya indeterminate (UB).
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_uninit 03_uninitialized_broken.c
 *   ./03_uninit
 * Compiler biasanya memperingatkan (-Wuninitialized / -Wmaybe-uninitialized).
 */
#include <stdio.h>

int main(void)
{
    int x;                        /* tidak diinisialisasi */
    printf("x = %d (nilai tidak tentu / UB)\n", x);
    return 0;
}
