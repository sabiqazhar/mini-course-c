/* Minggu 7 — Thread dasar dengan pthread_create / pthread_join
 * Tujuan: membuat satu thread, mengirim argumen, menunggu selesai, dan membersihkan argumen.
 *
 * Pola lifetime argumen: kita alokasikan struct di heap, berikan ke thread, lalu
 * bebaskan SETELAH pthread_join (karena thread masih mungkin memakainya saat join).
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 01_pthread_basic 01_pthread_basic.c
 * Run   : ./01_pthread_basic
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int  id;
    long iter;
} arg_t;

static void *pekerja(void *v)
{
    arg_t *a = (arg_t *)v;            /* argumen valid selama thread hidup */
    long total = 0;
    for (long i = 0; i < a->iter; i++)
        total += i;

    printf("thread %d selesai, jumlah = %ld\n", a->id, total);
    return NULL;                      /* implicit pthread_exit(NULL) */
}

int main(void)
{
    pthread_t t;
    arg_t *a = malloc(sizeof *a);
    if (a == NULL) return 1;
    a->id = 1;
    a->iter = 1000;

    if (pthread_create(&t, NULL, pekerja, a) != 0) {
        perror("pthread_create");
        free(a);
        return 1;
    }

    pthread_join(t, NULL);            /* tunggu thread selesai */
    free(a);                         /* baru bebaskan argumen */
    return 0;
}
