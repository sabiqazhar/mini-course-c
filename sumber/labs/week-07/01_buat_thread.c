/* Minggu 7 — Membuat thread, mengirim argumen, mengambil nilai kembalian
 *
 * Tujuan: melihat bahwa pthread_create butuh (1) fungsi dengan signature
 *         void *(*)(void *), (2) satu pointer argumen, dan bahwa alur kembali
 *         ke pemanggil lewat pthread_join.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 01_buat_thread 01_buat_thread.c
 * Run   : ./01_buat_thread
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* ---- data BERSAMA: satu salinan, bisa DIBACA semua thread ----
 * Di file ini variabel ini hanya dibaca, tidak pernah ditulis dari thread.
 * Menulis dari dua thread tanpa sinkronisasi adalah data race; lihat file
 * 04_counter_race_broken.c untuk demonya.
 */
static const int konstanta_bersama = 7;

struct argumen {
    int           nomor;   /* identitas thread                     */
    unsigned long jumlah;  /* berapa kali mengulang                */
};

struct hasil {
    int           nomor;
    unsigned long langkah;
};

static void *kerja(void *arg)
{
    struct argumen *a = arg;      /* (1) kembali ke tipe asli        */
    int lokal = a->nomor;         /* (2) variabel lokal = PRIVATE    */

    for (unsigned long i = 0; i < a->jumlah; i++) {
        /* tidak melakukan apa-apa selain menghabiskan waktu sedikit */
    }

    /* (3) dua thread membaca objek yang SAMA: alamatnya sama persis */
    printf("thread %d: &lokal = %p, &konstanta_bersama = %p, nilai = %d\n",
           lokal, (void *)&lokal, (void *)&konstanta_bersama, konstanta_bersama);

    struct hasil *h = malloc(sizeof *h);   /* (4) hasil dialokasikan  */
    if (h == NULL) {
        return NULL;                       /* join akan menerima NULL */
    }
    h->nomor   = lokal;
    h->langkah = a->jumlah;
    return h;   /* (5) sama efeknya dengan pthread_exit(h) */
}

int main(void)
{
    pthread_t tid[2];
    struct argumen arg[2] = { { 1, 3 }, { 2, 5 } };

    for (int i = 0; i < 2; i++) {
        int rc = pthread_create(&tid[i], NULL, kerja, &arg[i]);
        if (rc != 0) {                    /* (6) rc adalah error NUMBER */
            fprintf(stderr, "pthread_create gagal: rc=%d\n", rc);
            return 1;
        }
    }

    unsigned long total = 0;
    for (int i = 0; i < 2; i++) {
        void *kembalian = NULL;
        int rc = pthread_join(tid[i], &kembalian);
        if (rc != 0) {
            fprintf(stderr, "pthread_join gagal: rc=%d\n", rc);
            return 1;
        }
        if (kembalian != NULL) {
            struct hasil *h = kembalian;
            printf("join thread %d: langkah = %lu\n", h->nomor, h->langkah);
            total += h->langkah;
            free(h);                      /* (7) pemanggil yang melepas */
        }
    }

    printf("total langkah dilaporkan = %lu\n", total);
    return 0;
}
