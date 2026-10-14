/* Minggu 8 — Solusi mudah: counter dilindungi mutex, hasil selalu sama (tanpa race).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o mutex_counter mutex_counter.c
 * Run  : ./mutex_counter   (coba 10 kali, hasil harus sama)
 */
#include <stdio.h>
#include <pthread.h>

#define N_THREADS 4
#define PER 50000

static long counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < PER; i++) {
        pthread_mutex_lock(&mtx);
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(void)
{
    pthread_t t[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) pthread_create(&t[i], NULL, worker, NULL);
    for (int i = 0; i < N_THREADS; i++) pthread_join(t[i], NULL);
    printf("counter = %ld (harus %d)\n", counter, N_THREADS * PER);
    return 0;
}
