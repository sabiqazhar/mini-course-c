/* Minggu 12 — implementasi antrean berbatas aman-thread.
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -c tsqueue.c
 */
#include "tsqueue.h"

#include <stdlib.h>   /* malloc, free            */
#include <stdint.h>   /* SIZE_MAX                */
#include <pthread.h>  /* mutex, cond             */

int tsqueue_init(TSQueue *q, size_t cap)
{
    if (q == NULL || cap == 0 || cap > SIZE_MAX / sizeof(int))
        return -1;                       /* kapasitas nol / overflow */

    q->buf = malloc(cap * sizeof(int));
    if (q->buf == NULL)
        return -1;                       /* gagal alokasi buffer */

    q->cap = cap;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->shutdown = 0;

    /* Kalau primitif gagal, bersihkan apa yang sudah dialokasikan agar tidak bocor. */
    if (pthread_mutex_init(&q->mtx, NULL) != 0) {
        free(q->buf);
        q->buf = NULL;
        return -1;
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&q->mtx);
        free(q->buf);
        q->buf = NULL;
        return -1;
    }
    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        pthread_cond_destroy(&q->not_empty);
        pthread_mutex_destroy(&q->mtx);
        free(q->buf);
        q->buf = NULL;
        return -1;
    }
    return 0;
}

int tsqueue_enqueue(TSQueue *q, int v)
{
    if (pthread_mutex_lock(&q->mtx) != 0)
        return -1;

    /* while (bukan if): cegah spurious wakeup & pastikan kondisi dicek ulang
       setelah terbangun dari cond_wait. */
    while (q->count == q->cap && !q->shutdown)
        pthread_cond_wait(&q->not_full, &q->mtx);

    /* Setelah terbangun: kalau queue penuh DAN sudah shutdown, tolak item. */
    if (q->shutdown) {
        pthread_mutex_unlock(&q->mtx);
        return -1;
    }

    q->buf[q->tail] = v;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;

    pthread_cond_signal(&q->not_empty);   /* beri tahu SATU konsumen */
    pthread_mutex_unlock(&q->mtx);
    return 0;
}

int tsqueue_dequeue(TSQueue *q, int *out)
{
    if (pthread_mutex_lock(&q->mtx) != 0)
        return -1;

    while (q->count == 0 && !q->shutdown)
        pthread_cond_wait(&q->not_empty, &q->mtx);

    /* Berarti shutdown DAN queue kosong: konsumen harus keluar. */
    if (q->count == 0) {
        pthread_mutex_unlock(&q->mtx);
        return -1;
    }

    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;

    pthread_cond_signal(&q->not_full);    /* beri tahu SATU produser */
    pthread_mutex_unlock(&q->mtx);
    return 0;
}

void tsqueue_shutdown(TSQueue *q)
{
    if (q == NULL)
        return;
    if (pthread_mutex_lock(&q->mtx) != 0)
        return;
    q->shutdown = 1;
    /* Bangunkan SEMUA thread yang menunggu, produser maupun konsumen. */
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mtx);
}

void tsqueue_destroy(TSQueue *q)
{
    if (q == NULL)
        return;
    /* Pemanggil wajib join semua thread dulu; di sini hanya membersihkan resource. */
    free(q->buf);
    q->buf = NULL;
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    pthread_mutex_destroy(&q->mtx);
}
