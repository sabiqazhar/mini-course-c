/* Minggu 4 — Dynamic array int dengan kontrak keamanan
 * Tujuan: membangun struktur data yang tumbuh, dengan penanganan kegagalan yang konsisten.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_dynamic_array 01_dynamic_array.c
 * Run   : ./01_dynamic_array
 *
 * Versi dengan sanitizer (selalu jalankan ini juga):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined \
 *       -o 01_dynamic_array_san 01_dynamic_array.c
 *   ./01_dynamic_array_san
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int  *data;
    size_t len;       /* jumlah elemen yang sudah terisi */
    size_t cap;       /* kapasitas (banyaknya slot yang dialokasikan) */
} IntVec;

static int vec_init(IntVec *v, size_t cap)
{
    v->len = 0;
    v->cap = cap;
    v->data = malloc(cap * sizeof *v->data);
    return v->data != NULL ? 0 : -1;
}

static int vec_push(IntVec *v, int value)
{
    if (v->len == v->cap) {
        /* Perlu memperbesar. Pakai pointer sementara agar alokasi gagal
         * tidak merusak data yang sudah ada. */
        size_t new_cap = v->cap * 2;
        int *tmp = realloc(v->data, new_cap * sizeof *v->data);
        if (tmp == NULL)
            return -1;            /* alokasi gagal: v->data tetap valid */
        v->data = tmp;
        v->cap = new_cap;
    }
    v->data[v->len++] = value;
    return 0;
}

static void vec_destroy(IntVec *v)
{
    free(v->data);
    v->data = NULL;
    v->len = v->cap = 0;
}

int main(void)
{
    IntVec v;
    if (vec_init(&v, 2) != 0) {
        fprintf(stderr, "init gagal\n");
        return 1;
    }

    /* Dorong lebih dari kapasitas awal untuk memicu resize. */
    for (int i = 0; i < 6; i++) {
        if (vec_push(&v, i * 10) != 0) {
            fprintf(stderr, "push gagal di i = %d\n", i);
            break;
        }
    }

    printf("len=%zu cap=%zu\n", v.len, v.cap);
    for (size_t i = 0; i < v.len; i++)
        printf("v[%zu] = %d\n", i, v.data[i]);

    vec_destroy(&v);
    return 0;
}
