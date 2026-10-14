/* Minggu 11 — Solusi menengah: perbaiki off-by-one (i <= n  ->  i < n).
 *
 * Yang diubah: batas loop menjadi i < n (bukan i <= n), sehingga kita tidak menulis
 * melewati akhir array. Di bawah ASan sekarang lulus tanpa laporan.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o off_by_one_fixed off_by_one_fixed.c
 * Run  : ./off_by_one_fixed
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int n = 5;
    int *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)        /* FIX: i < n, bukan i <= n */
        arr[i] = i;

    int sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    printf("sum = %d (harus 10)\n", sum);

    free(arr);
    return 0;
}
