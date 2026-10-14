/* Minggu 8 — Atomic counter dengan <stdatomic.h>
 * Tujuan: memperbaiki counter dengan operasi atomic (satu operasi, bukan critical section).
 *
 * Catatan: atomic membuat operasi TUNGGAL atomik. Ia TIDAK membuat rangkaian beberapa
 * langkah menjadi atomik. Untuk counter ++, atomic_fetch_add tepat.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 03_atomic_counter 03_atomic_counter.c
 * Run   : ./03_atomic_counter
 */
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

static atomic_long counter = 0;     /* _Atomic long */

#define N 100000
#define THREADS 4

static void *inc(void *v)
{
    (void)v;
    for (int i = 0; i < N; i++)
        atomic_fetch_add(&counter, 1);   /* operasi atomik: tambah 1 */
    return NULL;
}

int main(void)
{
    pthread_t t[THREADS];
    for (int i = 0; i < THREADS; i++)
        pthread_create(&t[i], NULL, inc, NULL);
    for (int i = 0; i < THREADS; i++)
        pthread_join(t[i], NULL);

    printf("counter = %ld  (harusnya %d)\n", atomic_load(&counter), THREADS * N);
    return 0;
}
