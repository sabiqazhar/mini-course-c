/* Minggu 8 — Solusi menengah: bandingkan waktu mutex vs atomic (kasar).
 *
 * Pakai clock() untuk CPU time. Untuk penambahan sederhana, atomic biasanya lebih
 * cepat karena tidak ada sistem locking; mutex lebih mahal tapi fleksibel (bisa
 * melindungi operasi majemuk/non-atomik).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o bench bench_mutex_vs_atomic.c
 * Run  : ./bench
 */
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define N_THREADS 4
#define PER 500000

static long          m_counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static _Atomic long  a_counter = 0;

static void *m_worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < PER; i++) {
        pthread_mutex_lock(&mtx);
        m_counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

static void *a_worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < PER; i++)
        atomic_fetch_add(&a_counter, 1);
    return NULL;
}

static double run_mutex(void)
{
    clock_t t0 = clock();
    pthread_t th[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) pthread_create(&th[i], NULL, m_worker, NULL);
    for (int i = 0; i < N_THREADS; i++) pthread_join(th[i], NULL);
    return (double)(clock() - t0) / CLOCKS_PER_SEC;
}

static double run_atomic(void)
{
    clock_t t0 = clock();
    pthread_t th[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) pthread_create(&th[i], NULL, a_worker, NULL);
    for (int i = 0; i < N_THREADS; i++) pthread_join(th[i], NULL);
    return (double)(clock() - t0) / CLOCKS_PER_SEC;
}

int main(void)
{
    double tm = run_mutex();
    double ta = run_atomic();
    printf("mutex  : counter=%ld  waktu=%.4fs\n", m_counter, tm);
    printf("atomic : counter=%ld  waktu=%.4fs\n", (long)a_counter, ta);
    if (ta > 0)
        printf("atomic ~%.2fx waktu mutex (kasar, tergantung beban mesin)\n", tm / ta);
    return 0;
}
