/* Solusi latihan tantangan minggu 2 — membaca array lewat pointer arithmetic
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o analisis_array analisis_array.c
 * Run   : ./analisis_array
 */
#include <stdio.h>

int main(void)
{
    int a[5] = { 10, 20, 30, 40, 50 };
    int *p = a;

    /* Tiga cara setara untuk elemen ke-i: a[i], *(a + i), *(p + i), p[i] */
    for (size_t i = 0; i < 5; i++) {
        printf("a[%zu] = %-3d  *(a+%zu) = %-3d  *(p+%zu) = %-3d  p[%zu] = %-3d\n",
               i, a[i], i, *(a + i), i, *(p + i), i, p[i]);
    }

    /* Catatan: a + i valid untuk i = 0..4, dan (a + 5) boleh dibentuk sebagai
     * "satu elemen setelah akhir" tetapi TIDAK boleh di-dereference. */
    printf("\n&a[0] = %p, (a + 5) = %p (batas satu-lebih, jangan diakses)\n",
           (void *)&a[0], (void *)(a + 5));

    return 0;
}
