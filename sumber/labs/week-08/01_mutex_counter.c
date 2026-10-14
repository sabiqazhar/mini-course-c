/* Minggu 8 — Memperbaiki counter race dengan mutex
 * Tujuan: melindungi critical section (counter++) dengan mutex.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 01_mutex_counter 01_mutex_counter.c
 * Run   : ./01_mutex_counter
 */
#include <stdio.h>
#include <pthread.h>

static long counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;   /* inisialisasi statis */

#define N 100000
#define THREADS 4

static void *inc(void *v)
{
    (void)v;
    for (int i = 0; i < N; i++) {
        if (pthread_mutex_lock(&mtx) != 0) return NULL;   /* selalu cek return code */
        counter++;
        if (pthread_mutex_unlock(&mtx) != 0) return NULL;
    }
    return NULL;
}

int main(void)
{
    pthread_t t[THREADS];

    for (int i = 0; i < THREADS; i++)
        if (pthread_create(&t[i], NULL, inc, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }

    for (int i = 0; i < THREADS; i++)
        pthread_join(t[i], NULL);

    printf("counter = %ld  (harusnya %d)\n", counter, THREADS * N);
    if (pthread_mutex_destroy(&mtx) != 0)
        perror("pthread_mutex_destroy");
    return 0;
}
