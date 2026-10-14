/* Minggu 7 — CONTOH RUSAK — JANGAN DITIRU
 *
 * Demo terisolasi: dua thread menaikkan SATU penghitung bersama TANPA
 * sinkronisasi apa pun. Tidak ada mutex, tidak ada atomic.
 *
 * Program ini punya data race, jadi perilakunya adalah UNDEFINED BEHAVIOR
 * menurut standar C. Karena itu:
 *   - jangan pernah menjanjikan angka tertentu dari program ini,
 *   - jangan menyimpulkan "aman" hanya karena sekali jalan hasilnya benar.
 *
 * Build biasa : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread \
 *                    -o 04_counter_race_broken 04_counter_race_broken.c
 * Build TSan  : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fsanitize=thread -pthread \
 *                    -o 04_counter_race_tsan 04_counter_race_broken.c
 * Run         : ./04_counter_race_broken
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define JUMLAH_THREAD 2
#define PUTARAN       200000L   /* tiap thread menaikkan sebanyak ini */

static long penghitung = 0;     /* BERSAMA, tanpa pelindung */

static void *naikkan(void *arg)
{
    (void)arg;
    for (long i = 0; i < PUTARAN; i++) {
        penghitung++;           /* BUKAN operasi atomik: baca, tambah, tulis */
    }
    return NULL;
}

int main(void)
{
    pthread_t tid[JUMLAH_THREAD];

    for (int i = 0; i < JUMLAH_THREAD; i++) {
        int rc = pthread_create(&tid[i], NULL, naikkan, NULL);
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

    long diharapkan = (long)JUMLAH_THREAD * PUTARAN;
    printf("diharapkan : %ld\n", diharapkan);
    printf("didapat    : %ld\n", penghitung);
    printf("hilang     : %ld\n", diharapkan - penghitung);
    printf("\nCATATAN: angka di atas TIDAK bisa dijanjikan. Jalankan 10 kali.\n");
    printf("Kalau kebetulan 'hilang = 0', itu bukan bukti program ini aman.\n");
    return 0;
}
