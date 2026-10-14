/* Minggu 8 — Bounded queue sederhana dengan satu mutex (tidak memblokir)
 * Tujuan: melihat struktur queue berbasis ring buffer + kontrak ownership/invariant.
 *         Versi memblokir (producer/consumer) dibangun di minggu 12.
 *
 * Invarian: 0 <= count <= capacity; head/tail bergeser melingkar di dalam buffer.
 * Kontrak: pemanggil harus memanggil queue_destroy. enqueue mengembalikan -1 bila penuh;
 *          dequeue mengembalikan -1 bila kosong (tanpa memblokir).
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 04_bounded_queue 04_bounded_queue.c
 * Run   : ./04_bounded_queue
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int            *buf;
    size_t          cap;
    size_t          head;      /* indeks dequeue berikutnya */
    size_t          tail;      /* indeks enqueue berikutnya */
    size_t          count;
    pthread_mutex_t mtx;
} BQueue;

static int queue_init(BQueue *q, size_t cap)
{
    q->buf = malloc(cap * sizeof *q->buf);
    if (q->buf == NULL) return -1;
    q->cap = cap; q->head = 0; q->tail = 0; q->count = 0;
    return pthread_mutex_init(&q->mtx, NULL);
}

static int queue_enqueue(BQueue *q, int v)
{
    if (pthread_mutex_lock(&q->mtx) != 0) return -1;
    int rc = -1;
    if (q->count < q->cap) {
        q->buf[q->tail] = v;
        q->tail = (q->tail + 1) % q->cap;
        q->count++;
        rc = 0;
    }
    pthread_mutex_unlock(&q->mtx);   /* selalu unlock, baik sukses maupun penuh */
    return rc;
}

static int queue_dequeue(BQueue *q, int *out)
{
    if (pthread_mutex_lock(&q->mtx) != 0) return -1;
    int rc = -1;
    if (q->count > 0) {
        *out = q->buf[q->head];
        q->head = (q->head + 1) % q->cap;
        q->count--;
        rc = 0;
    }
    pthread_mutex_unlock(&q->mtx);
    return rc;
}

static void queue_destroy(BQueue *q)
{
    free(q->buf);
    q->buf = NULL;
    pthread_mutex_destroy(&q->mtx);
}

int main(void)
{
    BQueue q;
    if (queue_init(&q, 3) != 0) { fprintf(stderr, "init gagal\n"); return 1; }

    printf("enqueue 10 : %d\n", queue_enqueue(&q, 10));
    printf("enqueue 20 : %d\n", queue_enqueue(&q, 20));
    printf("enqueue 30 : %d\n", queue_enqueue(&q, 30));
    printf("enqueue 40 (penuh?) : %d\n", queue_enqueue(&q, 40)); /* -1: penuh */

    int v;
    while (queue_dequeue(&q, &v) == 0)
        printf("dequeue -> %d (count sekarang)\n", v);

    queue_destroy(&q);
    return 0;
}
