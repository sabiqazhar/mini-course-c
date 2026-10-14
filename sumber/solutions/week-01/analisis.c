/* Solusi latihan tantangan minggu 1 — analisis pointer untuk tiga "tempat tinggal" objek
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o analisis analisis.c
 * Run   : ./analisis
 *
 * Catatan penting: jangan menyimpulkan "wilayah memory" hanya dari satu kali jalan.
 * Alamat berubah setiap kali program dijalankan karena ASLR. Yang bisa diamati dengan
 * aman adalah: apakah dua alamat itu sama, berurutan, atau berjauhan.
 */
#include <stdio.h>

int global_x = 100;          /* static storage duration: hidup sepanjang program */

void analisis(const char *label, int *p)
{
    if (p == NULL) {
        printf("%-8s : p adalah NULL, tidak bisa di-dereference\n", label);
        return;
    }

    printf("%-8s : *p = %d, p = %p, &p = %p, sizeof p = %zu, sizeof *p = %zu\n",
           label, *p, (void *)p, (void *)&p, sizeof p, sizeof *p);
}

int main(void)
{
    int        lokal = 7;              /* automatic storage duration */
    static int statik = 8;             /* static storage duration, scope lokal */
    int       *heap = NULL;            /* akan diisi di minggu 3 */

    analisis("lokal",  &lokal);
    analisis("global", &global_x);
    analisis("statik", &statik);
    analisis("NULL",   heap);

    return 0;
}
