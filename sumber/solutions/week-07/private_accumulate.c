/* Minggu 7 — Solusi tantangan: akumulasi privat lalu gabung (tanpa mutex).
 *
 * Tidak ada data race karena tiap thread menulis ke buffer MILIKNYA (malloc sendiri);
 * tidak ada variabel global yang di-write saat thread berjalan. Penggabungan baru
 * dilakukan SETELAH semua thread di-join. Ini selalu benar tanpa mutex.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o private_accumulate private_accumulate.c
 * Run  : ./private_accumulate
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N_THREADS 4
#define PER_THREAD 50000

static void *worker(void *arg)
{
    (void)arg;
    long *acc = malloc(sizeof(long));
    *acc = 0;
    for (int i = 0; i < PER_THREAD; i++) (*acc)++;
    return acc;   /* kembalikan pointer ke hasil miliknya */
}

int main(void)
{
    pthread_t t[N_THREADS];
    for (int i = 0; i < N_THREADS; i++)
        pthread_create(&t[i], NULL, worker, NULL);

    long total = 0;
    for (int i = 0; i < N_THREADS; i++) {
        long *acc;
        pthread_join(t[i], (void **)&acc);
        total += *acc;
        free(acc);
    }
    printf("total = %ld (harus %d)\n", total, N_THREADS * PER_THREAD);
    return 0;
}
