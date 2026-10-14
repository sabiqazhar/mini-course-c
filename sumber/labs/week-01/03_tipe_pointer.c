/* Minggu 1 — Mengapa tipe pointer penting
 * Tujuan: semua pointer punya ukuran yang sama, tetapi tipe menentukan
 *         bagaimana objek yang ditunjuk dibaca.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_tipe_pointer 03_tipe_pointer.c
 * Run   : ./03_tipe_pointer
 *
 * Catatan: baris yang mengubah tipe pointer lewat cast ada di bawah dan SENGAJA
 *          tidak di-dereference. Mengakses objek int lewat pointer double adalah
 *          undefined behavior.
 */
#include <stdio.h>

int main(void)
{
    int    x  = 42;
    int   *pi = &x;
    char  *pc = (char *)&x;   /* legal sebagai pointer, tapi membaca 1 byte saja */
    double *pd = (double *)&x; /* JANGAN di-dereference: undefined behavior */

    printf("sizeof(int *)    = %zu\n", sizeof pi);
    printf("sizeof(char *)   = %zu\n", sizeof pc);
    printf("sizeof(double *) = %zu\n", sizeof pd);
    printf("sizeof(void *)   = %zu\n", sizeof (void *));

    printf("\n*pi = %d   (dibaca sebagai int)\n", *pi);
    printf("*pc = %d   (dibaca sebagai char: 1 byte pertama dari representasi int)\n", *pc);

    /* Kalau baris ini diaktifkan, program melakukan undefined behavior:
     * printf("*pd = %f\n", *pd);
     */

    return 0;
}
