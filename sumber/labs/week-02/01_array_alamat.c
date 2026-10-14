/* Minggu 2 — Alamat elemen array dan perbedaan sizeof
 * Tujuan: melihat bahwa elemen array berurutan, dan bahwa nama array BUKAN pointer.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_array_alamat 01_array_alamat.c
 * Run   : ./01_array_alamat
 */
#include <stdio.h>

int main(void)
{
    int a[5] = { 10, 20, 30, 40, 50 };

    printf("sizeof a       = %zu byte  (seluruh array, 5 x int)\n", sizeof a);
    printf("sizeof a[0]    = %zu byte  (satu elemen)\n", sizeof a[0]);
    printf("sizeof &a[0]   = %zu byte  (sebuah pointer ke int)\n", sizeof &a[0]);

    for (int i = 0; i < 5; i++) {
        printf("&a[%d] = %p   (jarak dari &a[0] = %td byte)\n",
               i, (void *)&a[i], (char *)&a[i] - (char *)&a[0]);
    }

    int *p = a;                 /* array di sini "decay" menjadi pointer ke elemen 0 */
    printf("\nsizeof p       = %zu byte  (ini pointer, beda dari sizeof a)\n", sizeof p);
    printf("p + 1 - p      = %td         (satu elemen, bukan satu byte)\n", (p + 1) - p);

    return 0;
}
