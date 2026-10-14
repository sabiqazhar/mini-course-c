/* Solusi latihan menengah minggu 4 — dynamic array dengan get/set/pop dan uji batas
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o dynamic_array_full dynamic_array_full.c
 * Run   : ./dynamic_array_full
 *
 * Dengan sanitizer:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined \
 *       -o dynamic_array_full_san dynamic_array_full.c && ./dynamic_array_full_san
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *data;
    size_t  len;
    size_t  cap;
} IntVec;

static int vec_init(IntVec *v, size_t cap)
{
    v->len = 0; v->cap = cap;
    v->data = malloc((cap ? cap : 1) * sizeof *v->data);
    return v->data ? 0 : -1;
}

static int vec_reserve(IntVec *v, size_t want)
{
    if (v->cap >= want) return 0;
    size_t new_cap = v->cap ? v->cap : 1;
    while (new_cap < want) new_cap *= 2;
    int *tmp = realloc(v->data, new_cap * sizeof *v->data);
    if (tmp == NULL) return -1;
    v->data = tmp;
    v->cap = new_cap;
    return 0;
}

static int vec_push(IntVec *v, int value)
{
    if (vec_reserve(v, v->len + 1) != 0) return -1;
    v->data[v->len++] = value;
    return 0;
}

static int vec_get(const IntVec *v, size_t i, int *out)
{
    if (i >= v->len) return -1;        /* di luar batas: tolak dengan jelas */
    *out = v->data[i];
    return 0;
}

static int vec_pop(IntVec *v, int *out)
{
    if (v->len == 0) return -1;        /* kosong */
    *out = v->data[--v->len];
    return 0;
}

static void vec_destroy(IntVec *v)
{
    free(v->data);
    v->data = NULL; v->len = v->cap = 0;
}

int main(void)
{
    IntVec v;
    if (vec_init(&v, 2) != 0) { fprintf(stderr, "init gagal\n"); return 1; }

    for (int i = 0; i < 5; i++)
        if (vec_push(&v, i) != 0) { fprintf(stderr, "push gagal\n"); break; }

    int x;
    if (vec_get(&v, 3, &x) == 0) printf("get[3] = %d\n", x);
    if (vec_get(&v, 99, &x) != 0) printf("get[99] ditolak (di luar batas)\n");

    while (vec_pop(&v, &x) == 0)
        printf("pop -> %d (len sekarang %zu)\n", x, v.len);

    if (vec_pop(&v, &x) != 0) printf("pop dari kosong ditolak\n");

    vec_destroy(&v);
    return 0;
}
