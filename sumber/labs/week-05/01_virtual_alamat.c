/* Minggu 5 — Semua alamat yang kamu cetak adalah virtual address
 * Tujuan: melihat bahwa fungsi, global, lokal, dan heap semuanya punya alamat,
 *         dan bahwa alamat berubah tiap run (ASLR). Jangan menyimpulkan "wilayah"
 *         memory hanya dari satu eksperimen.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_virtual_alamat 01_virtual_alamat.c
 * Run   : ./01_virtual_alamat     (jalankan beberapa kali)
 */
#include <stdio.h>
#include <stdlib.h>

void fungsi(void) { }

int global_var = 123;            /* static storage duration */

int main(void)
{
    int   lokal = 7;             /* automatic storage duration */
    int  *heap  = malloc(sizeof *heap);
    if (heap == NULL) return 1;
    *heap = 99;

    /* Catatan: fungsi juga punya alamat, tetapi mencetaknya lewat %p dengan
       portabel bukan hal yang dijamin standar, jadi kita fokus ke tiga di bawah. */
    printf("alamat global : %p\n", (void *)&global_var);
    printf("alamat lokal  : %p\n", (void *)&lokal);
    printf("alamat heap   : %p\n", (void *)heap);

    free(heap);

    printf("\nJalankan beberapa kali: keempat alamat di atas akan berubah.\n");
    printf("Itu karena ASLR. Alamat ini VIRTUAL, bukan alamat fisik RAM.\n");
    return 0;
}
