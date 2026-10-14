/* Solusi latihan menengah minggu 1 — menukar dua nilai lewat pointer
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o swap swap.c
 * Run   : ./swap
 */
#include <stdio.h>

void swap_salah(int a, int b)   /* pass by value: hanya salinan yang tertukar */
{
    int tmp = a;
    a = b;
    b = tmp;
}

void swap_benar(int *a, int *b) /* menerima alamat, jadi bisa mengubah objek asli */
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main(void)
{
    int x = 5;
    int y = 9;

    printf("awal            : x = %d, y = %d\n", x, y);

    swap_salah(x, y);
    printf("setelah salah   : x = %d, y = %d   (tidak berubah)\n", x, y);

    swap_benar(&x, &y);
    printf("setelah benar   : x = %d, y = %d\n", x, y);

    /* Kontrak fungsi swap_benar:
     * - a dan b harus pointer valid ke objek int yang masih hidup.
     * - a dan b boleh menunjuk objek yang sama (hasilnya: tidak ada perubahan).
     * - NULL tidak boleh dikirim; fungsi ini tidak memeriksanya.
     */
    return 0;
}
