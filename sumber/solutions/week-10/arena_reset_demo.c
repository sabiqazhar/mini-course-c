/* Minggu 10 — Solusi mudah: arena bump allocator dengan reset.
 *
 * Buktikan bahwa setelah arena_reset, alamat kembali ke awal (offset di-nol-kan).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o arena_reset_demo arena_reset_demo.c
 * Run  : ./arena_reset_demo
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char  *base;
    size_t offset;
    size_t cap;
} Arena;

static int arena_init(Arena *a, size_t cap)
{
    a->base = malloc(cap);
    if (!a->base) return -1;
    a->offset = 0; a->cap = cap;
    return 0;
}
static void *arena_alloc(Arena *a, size_t n)
{
    if (a->offset + n > a->cap) return NULL;
    void *p = a->base + a->offset;
    a->offset += n;
    return p;
}
static void arena_reset(Arena *a) { a->offset = 0; }
static void arena_destroy(Arena *a) { free(a->base); a->base = NULL; }

int main(void)
{
    Arena a;
    if (arena_init(&a, 1024) != 0) return 1;

    int *p[5];
    for (int i = 0; i < 5; i++) { p[i] = arena_alloc(&a, sizeof(int)); *p[i] = i; }
    printf("sebelum reset: p[0]=%p p[4]=%p\n", (void *)p[0], (void *)p[4]);

    arena_reset(&a);
    int *q[2];
    for (int i = 0; i < 2; i++) { q[i] = arena_alloc(&a, sizeof(int)); *q[i] = 100 + i; }
    printf("sesudah reset: q[0]=%p q[1]=%p\n", (void *)q[0], (void *)q[1]);
    printf("q[0]==p[0]? %s (alamat kembali ke awal)\n", (q[0] == p[0]) ? "YA" : "TIDAK");

    arena_destroy(&a);
    return 0;
}
