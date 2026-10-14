/* Minggu 7 — Versi aman: tiap thread akumulasi secara privat, gabung SETELAH join
 * Tujuan: menghindari shared state sama sekali. Tidak ada sinkronisasi diperlukan
 *         karena tidak ada data yang diakses bersama saat berjalan.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 03_private 03_private_accumulate.c
 * Run   : ./03_private
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 100000

static void *hitung(void *v)
{
    (void)v;
    long *s = malloc(sizeof *s);
    if (s == NULL) return NULL;
    *s = 0;
    for (int i = 0; i < N; i++)
        (*s)++;
    return s;                  /* kembalikan hasil privat lewat pointer */
}

int main(void)
{
    pthread_t a, b;
    void *r1, *r2;

    pthread_create(&a, NULL, hitung, NULL);
    pthread_create(&b, NULL, hitung, NULL);
    pthread_join(a, &r1);
    pthread_join(b, &r2);

    long total = *(long *)r1 + *(long *)r2;
    printf("total = %ld  (harusnya %d)\n", total, 2 * N);

    free(r1);
    free(r2);
    return 0;
}
