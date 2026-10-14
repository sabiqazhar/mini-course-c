/* Minggu 7 — CONTOH RUSAK — JANGAN DITIRU
 *
 * Bug: mengirim ALAMAT variabel yang terus berubah nilai (variabel loop),
 *      lalu thread membacanya belakangan. Yang diterima thread bukan salinan
 *      nilai, melainkan pointer ke objek yang masih hidup dan masih diubah
 *      oleh pembuatnya.
 *
 * Program ini sengaja salah. Ia tidak crash, dan kadang-kadang malah
 * menghasilkan angka yang terlihat benar. Itu justru yang membuat bug ini
 * berbahaya.
 *
 * Build : gcc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -g -O0 \
 *              -pthread -o 03_argumen_berulang_broken 03_argumen_berulang_broken.c
 * Run   : ./03_argumen_berulang_broken     (jalankan beberapa kali)
 */
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define JUMLAH_THREAD 4

static void *cetak_nomor(void *arg)
{
    /* Supaya bugnya terlihat: thread sengaja menunggu 20 ms sebelum membaca
     * argumennya. Tanpa jeda pun bug ini tetap ADA, hanya saja lebih jarang
     * terlihat — dan itulah ciri bug yang bergantung pada timing. */
    struct timespec jeda = { 0, 20 * 1000 * 1000 };  /* 20 milidetik */
    nanosleep(&jeda, NULL);

    int nomor = *(int *)arg;    /* BUG: membaca objek yang sedang diubah */
    printf("  thread ini mengira dirinya nomor %d\n", nomor);
    return NULL;
}

int main(void)
{
    pthread_t tid[JUMLAH_THREAD];

    printf("membuat %d thread, mengirim &i dari loop:\n", JUMLAH_THREAD);

    for (int i = 0; i < JUMLAH_THREAD; i++) {
        int rc = pthread_create(&tid[i], NULL, cetak_nomor, &i);  /* BUG */
        if (rc != 0) {
            fprintf(stderr, "pthread_create gagal: rc=%d\n", rc);
            return 1;
        }
    }

    for (int i = 0; i < JUMLAH_THREAD; i++) {
        int rc = pthread_join(tid[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_join gagal: rc=%d\n", rc);
            return 1;
        }
    }

    printf("selesai. Seharusnya ada 4 nomor berbeda: 0, 1, 2, 3.\n");
    printf("yang muncul di atas bisa saja duplikat, bisa saja 4,4,4,4.\n");
    return 0;
}
