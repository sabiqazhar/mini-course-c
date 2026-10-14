/* Minggu 1 — Nilai dan alamat
 * Tujuan: melihat bahwa sebuah variabel punya nilai DAN alamat, dua hal yang berbeda.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_nilai_dan_alamat 01_nilai_dan_alamat.c
 * Run   : ./01_nilai_dan_alamat
 */
#include <stdio.h>

int main(void)
{
    int    umur = 30;
    double suhu = 36.5;
    char   huruf = 'A';

    printf("umur  : nilai = %d,   alamat = %p, ukuran = %zu byte\n",
           umur, (void *)&umur, sizeof umur);
    printf("suhu  : nilai = %.1f, alamat = %p, ukuran = %zu byte\n",
           suhu, (void *)&suhu, sizeof suhu);
    printf("huruf : nilai = %c,   alamat = %p, ukuran = %zu byte\n",
           huruf, (void *)&huruf, sizeof huruf);

    return 0;
}
