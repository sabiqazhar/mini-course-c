/* Minggu 9 — Ring buffer sebagai dasar queue (tanpa thread)
 * Tujuan: melihat invariant buffer melingkar dan kontrak kepemilikan.
 *
 * Invarian: 0 <= count <= cap; head/tail bergerak melingkar (mod cap).
 * Kontrak: pemanggil memanggil ring_destroy; enqueue -1 bila penuh, dequeue -1 bila kosong.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 02_ring_buffer 02_ring_buffer.c
 * Run   : ./02_ring_buffer
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *buf;
    size_t  cap;
    size_t  head;
    size_t  tail;
    size_t  count;
} Ring;

static int ring_init(Ring *r, size_t cap)
{
    r->buf = malloc(cap * sizeof *r->buf);
    if (r->buf == NULL) return -1;
    r->cap = cap; r->head = 0; r->tail = 0; r->count = 0;
    return 0;
}

static int ring_enqueue(Ring *r, int v)
{
    if (r->count == r->cap) return -1;     /* penuh */
    r->buf[r->tail] = v;
    r->tail = (r->tail + 1) % r->cap;
    r->count++;
    return 0;
}

static int ring_dequeue(Ring *r, int *out)
{
    if (r->count == 0) return -1;          /* kosong */
    *out = r->buf[r->head];
    r->head = (r->head + 1) % r->cap;
    r->count--;
    return 0;
}

static void ring_destroy(Ring *r)
{
    free(r->buf);
    r->buf = NULL;
}

int main(void)
{
    Ring r;
    if (ring_init(&r, 4) != 0) return 1;

    for (int i = 1; i <= 4; i++)
        printf("enqueue %d -> %d\n", i, ring_enqueue(&r, i));
    printf("enqueue 5 (penuh?) -> %d\n", ring_enqueue(&r, 5));

    int v;
    while (ring_dequeue(&r, &v) == 0)
        printf("dequeue -> %d (count=%zu)\n", v, r.count);

    ring_destroy(&r);
    return 0;
}
