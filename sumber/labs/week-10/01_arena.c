/* Minggu 10 — Bump allocator (arena) dari buffer statis
 * Tujuan: melihat bagaimana sebuah allocator bekerja, paling sederhana.
 *
 * Strategi: simpan satu offset. Alokasi = geser offset. Tidak ada free per-object;
 *           hanya reset (kembalikan offset ke 0) atau bebaskan seluruh buffer.
 *
 * Batas mainan: kapasitas tetap, tidak thread-safe, alignment sederhana, tidak bisa
 *                free satu objek. Ini ALAT BELAJAR, bukan pengganti malloc.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_arena 01_arena.c
 * Run   : ./01_arena
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    char   *buf;
    size_t  cap;
    size_t  off;
} Arena;

static int arena_init(Arena *a, size_t cap)
{
    a->buf = malloc(cap);
    if (a->buf == NULL) return -1;
    a->cap = cap;
    a->off = 0;
    return 0;
}

/* Alokasikan n byte berikutnya dalam arena. NULL bila tidak cukup. */
static void *arena_alloc(Arena *a, size_t n)
{
    if (a->off + n > a->cap)
        return NULL;
    void *p = a->buf + a->off;
    a->off += n;
    return p;
}

static void arena_reset(Arena *a) { a->off = 0; }

static void arena_free(Arena *a)
{
    free(a->buf);
    a->buf = NULL;
    a->cap = a->off = 0;
}

int main(void)
{
    Arena a;
    if (arena_init(&a, 64) != 0) return 1;

    int *x = arena_alloc(&a, sizeof *x);
    int *y = arena_alloc(&a, sizeof *y);
    if (x == NULL || y == NULL) { arena_free(&a); return 1; }

    *x = 10; *y = 20;
    printf("x=%d y=%d  offset sekarang=%zu\n", *x, *y, a.off);

    /* Melebihi kapasitas: harus NULL */
    char *z = arena_alloc(&a, 100);
    printf("alokasi 100 byte (lewati kapasitas) -> %s\n", z ? "berhasil" : "NULL (benar)");

    arena_reset(&a);
    printf("setelah reset, offset=%zu (bisa dipakai lagi dari awal)\n", a.off);

    arena_free(&a);
    return 0;
}
