/* Minggu 10 — Solusi tantangan: arena berukuran variabel yang mencatat tiap alokasi
 * ke array kecil, sehingga bisa dibebaskan berurutan (LIFO) tanpa pointer free per-objek.
 *
 * Masih BUKAN pengganti malloc: kita harus tahu urutan bebas (LIFO) dan tidak bisa
 * membebaskan objek acak. Kemudahan: bebas sekaligus / berurutan sangat murah.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o arena_recorded arena_recorded.c
 * Run  : ./arena_recorded
 */
#include <stdio.h>
#include <stdlib.h>

#define MAX_ALLOC 16

typedef struct {
    char    *base;
    size_t   off;
    size_t   sizes[MAX_ALLOC];
    int      nalloc;
    size_t   cap;
} ArenaR;

static int arena_init(ArenaR *a, size_t cap)
{
    a->base = malloc(cap);
    if (!a->base) return -1;
    a->off = 0; a->nalloc = 0; a->cap = cap;
    return 0;
}
static void *arena_alloc(ArenaR *a, size_t n)
{
    if (a->nalloc >= MAX_ALLOC) return NULL;
    if (a->off + n > a->cap) return NULL;
    void *p = a->base + a->off;
    a->sizes[a->nalloc++] = n;
    a->off += n;
    return p;
}
static void arena_free_last(ArenaR *a)
{
    if (a->nalloc > 0) { a->nalloc--; a->off -= a->sizes[a->nalloc]; }  /* bebas LIFO */
}
static void arena_destroy(ArenaR *a) { free(a->base); a->base = NULL; }

int main(void)
{
    ArenaR a; arena_init(&a, 256);
    int    *x = arena_alloc(&a, sizeof(int));
    double *y = arena_alloc(&a, sizeof(double));
    printf("x=%p y=%p (berurutan di arena)\n", (void *)x, (void *)y);
    arena_free_last(&a);   /* bebas y dulu (LIFO) */
    arena_free_last(&a);   /* bebas x */
    printf("semua dibebaskan berurutan; arena masih utuh sampai destroy\n");
    arena_destroy(&a);
    return 0;
}
