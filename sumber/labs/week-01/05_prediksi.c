/* Minggu 1 — Latihan prediksi
 *
 * SEBELUM menjalankan program ini, tulis di kertas:
 *   1. Berapa nilai x, y, dan *p pada tiap baris cetak?
 *   2. Berapa alamat yang dicetak relatif satu sama lain (berurutan atau tidak)?
 *   3. Apakah p dan &x selalu sama?
 *
 * Jalankan, lalu bandingkan dengan tebakanmu. Yang penting bukan benar atau salah,
 * melainkan apakah kamu bisa menjelaskan hasilnya.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 05_prediksi 05_prediksi.c
 * Run   : ./05_prediksi
 */
#include <stdio.h>

int main(void)
{
    int x = 1;
    int y = 2;
    int *p = &x;

    printf("A: x=%d y=%d *p=%d\n", x, y, *p);

    p = &y;
    *p = 99;

    printf("B: x=%d y=%d *p=%d\n", x, y, *p);

    printf("C: &x=%p &y=%p p=%p\n", (void *)&x, (void *)&y, (void *)p);

    int y_lagi = 7;   /* nama berbeda, objek berbeda */
    printf("D: y=%d y_lagi=%d, &y=%p &y_lagi=%p\n",
           y, y_lagi, (void *)&y, (void *)&y_lagi);

    return 0;
}
