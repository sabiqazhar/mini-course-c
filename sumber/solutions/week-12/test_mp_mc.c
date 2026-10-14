/* Minggu 12 — test multi-producer / multi-consumer (MPMC) dengan shutdown deterministik.
 *
 * Skenario:
 *   - Beberapa produser masing-masing memproduksi N item unik, lalu keluar.
 *   - Beberapa konsumer mengambil item sampai queue kosong & shutdown, lalu keluar.
 *   - Main: join semua produser -> panggil tsqueue_shutdown -> join semua konsumer.
 *   - Verifikasi: total dikonsumsi == total diproduksi (tidak ada item hilang/duplikat).
 *
 * Kunci determinisme shutdown:
 *   Produser selesai duluan (di-join), baru shutdown dipanggil. Konsumer yang mungkin
 *   sedang menunggu dibangunkan oleh broadcast, mengosongkan sisa item, lalu keluar
 *   saat melihat (shutdown && count==0). Hasil akhir selalu konsisten.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o test_mp_mc tsqueue.c test_mp_mc.c
 * Run  : ./test_mp_mc
 * San   : gcc ... -fsanitize=thread  (TSan)  /  -fsanitize=address,undefined (ASan)
 */
#include "tsqueue.h"

#include <stdio.h>
#include <pthread.h>

#define N_PRODUCERS 4
#define N_CONSUMERS 4
#define ITEMS_PER_PRODUCER 10000

static TSQueue *g_q;
static unsigned long g_consumed;                 /* dijaga mutex terpisah */
static pthread_mutex_t g_count_mtx = PTHREAD_MUTEX_INITIALIZER;

static void *producer(void *arg)
{
    long id = (long)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        /* nilai unik per (produser, ke-) agar kita bisa curiga kalau ada yang hilang */
        int v = (int)(id * ITEMS_PER_PRODUCER + i);
        tsqueue_enqueue(g_q, v);                 /* memblokir bila penuh */
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    int v;
    while (tsqueue_dequeue(g_q, &v) == 0) {      /* keluar bila shutdown & kosong */
        pthread_mutex_lock(&g_count_mtx);
        g_consumed++;
        pthread_mutex_unlock(&g_count_mtx);
    }
    return NULL;
}

int main(void)
{
    TSQueue q;
    if (tsqueue_init(&q, 1024) != 0) {
        fprintf(stderr, "init gagal\n");
        return 1;
    }
    g_q = &q;
    g_consumed = 0;

    pthread_t prod[N_PRODUCERS], cons[N_CONSUMERS];

    for (long i = 0; i < N_PRODUCERS; i++)
        pthread_create(&prod[i], NULL, producer, (void *)i);
    for (long i = 0; i < N_CONSUMERS; i++)
        pthread_create(&cons[i], NULL, consumer, (void *)i);

    /* 1) tunggu semua produser selesai memproduksi */
    for (int i = 0; i < N_PRODUCERS; i++)
        pthread_join(prod[i], NULL);

    /* 2) nyalakan shutdown agar konsumer keluar saat queue kosong */
    tsqueue_shutdown(&q);

    /* 3) tunggu semua konsumer keluar */
    for (int i = 0; i < N_CONSUMERS; i++)
        pthread_join(cons[i], NULL);

    unsigned long expected = (unsigned long)N_PRODUCERS * ITEMS_PER_PRODUCER;
    printf("diproduksi = %lu, dikonsumsi = %lu, cocok = %s\n",
           expected, g_consumed, (expected == g_consumed) ? "YA" : "TIDAK");

    tsqueue_destroy(&q);
    return (expected == g_consumed) ? 0 : 1;
}
