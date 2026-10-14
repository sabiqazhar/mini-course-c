/* Minggu 10 — Free-list allocator mainan (blok ukuran tetap, alignment sederhana)
 * Tujuan: melihat splitting/coalescing secara mini — kita tidak benar-benar coalesce,
 *         tetapi punya daftar blok bebas yang dipakai-ulang.
 *
 * Batas mainan: ukuran blok tetap, tipe blok tunggal, tidak thread-safe, tidak panggil
 *                brk/mmap (buffer dari malloc). BUKAN pengganti malloc.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 02_free_list 02_free_list.c
 * Run   : ./02_free_list
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define BLK_SZ 32          /* ukuran blok tetap (byte) */

typedef struct Block {
    struct Block *next;    /* hanya valid bila blok sedang "bebas" */
    /* sisa BLK_SZ - sizeof(void*) dipakai user saat blok "dipakai" */
} Block;

typedef struct {
    Block *free_list;
    void  *pool;
    size_t pool_cap;
} Pool;

static int pool_init(Pool *p, size_t nblocks)
{
    p->pool = malloc(nblocks * BLK_SZ);
    if (p->pool == NULL) return -1;
    p->pool_cap = nblocks * BLK_SZ;
    p->free_list = NULL;

    /* Bangun rantai blok bebas */
    char *base = p->pool;
    for (size_t i = 0; i < nblocks; i++) {
        Block *b = (Block *)(base + i * BLK_SZ);
        b->next = p->free_list;
        p->free_list = b;
    }
    return 0;
}

static void *pool_alloc(Pool *p)
{
    if (p->free_list == NULL) return NULL;   /* habis */
    Block *b = p->free_list;
    p->free_list = b->next;
    return b;                                /* user tulis mulai dari sini */
}

static void pool_free(Pool *p, void *ptr)
{
    if (ptr == NULL) return;
    Block *b = ptr;
    b->next = p->free_list;                  /* kembalikan ke daftar bebas */
    p->free_list = b;
}

static void pool_destroy(Pool *p)
{
    free(p->pool);
    p->pool = NULL;
    p->free_list = NULL;
    p->pool_cap = 0;
}

int main(void)
{
    Pool p;
    if (pool_init(&p, 4) != 0) return 1;

    int *a = pool_alloc(&p);
    int *b = pool_alloc(&p);
    int *c = pool_alloc(&p);
    if (a) *a = 1;
    if (b) *b = 2;
    if (c) *c = 3;
    printf("a=%d b=%d c=%d\n", a ? *a : -1, b ? *b : -1, c ? *c : -1);

    pool_free(&p, b);          /* kembalikan b */
    int *d = pool_alloc(&p);    /* seharusnya mendapat kembali blok b */
    if (d) *d = 99;
    printf("setelah free(b), alokasi baru d -> %d (alamat sama? %s)\n",
           d ? *d : -1, d == b ? "ya" : "tidak");

    pool_free(&p, a); pool_free(&p, c); pool_free(&p, d);
    pool_destroy(&p);
    return 0;
}
