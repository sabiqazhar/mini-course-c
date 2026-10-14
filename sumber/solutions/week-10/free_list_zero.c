/* Minggu 10 — Solusi menengah: free-list pool dengan pool_alloc_zero (seperti calloc).
 *
 * Trade-off: menulis nol ke setiap blok memakan waktu ekstra, tetapi mencegah pembacaan
 * sisa data lama dari alokasi sebelumnya (lebih aman, deterministik).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o free_list_zero free_list_zero.c
 * Run  : ./free_list_zero
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_BLOCKS 8
#define BLOCK_SIZE 16

typedef struct Block { struct Block *next; char data[BLOCK_SIZE]; } Block;

typedef struct {
    Block *free_list;
    Block blocks[N_BLOCKS];
} Pool;

static void pool_init(Pool *p)
{
    p->free_list = NULL;
    for (int i = N_BLOCKS - 1; i >= 0; i--) {
        p->blocks[i].next = p->free_list;
        p->free_list = &p->blocks[i];
    }
}

static Block *pool_alloc(Pool *p)
{
    Block *b = p->free_list;
    if (!b) return NULL;
    p->free_list = b->next;
    return b;
}

static Block *pool_alloc_zero(Pool *p)
{
    Block *b = pool_alloc(p);
    if (b) memset(b->data, 0, BLOCK_SIZE);   /* zeroing = trade-off waktu */
    return b;
}

static void pool_free(Pool *p, Block *b)
{
    if (!b) return;
    b->next = p->free_list;
    p->free_list = b;
}

int main(void)
{
    Pool p; pool_init(&p);
    Block *a = pool_alloc_zero(&p);
    Block *b = pool_alloc(&p);
    printf("alloc_zero -> data[0]=%d (harus 0)\n", (unsigned char)a->data[0]);
    pool_free(&p, a); pool_free(&p, b);
    printf("blok dikembalikan ke pool\n");
    return 0;
}
