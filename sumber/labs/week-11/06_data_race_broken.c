/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #6: data race (dua thread menulis variabel yang sama tanpa sinkronisasi)
 *
 * Gejala  : hasil akhir berbeda setiap kali dijalankan; hampir tidak pernah
 *           crash. Data race adalah undefined behavior menurut standar C.
 * Alat    : ThreadSanitizer (-fsanitize=thread)
 *
 * Build (polos) : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 06_polos 06_data_race_broken.c
 * Build (TSan)  : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -pthread -fsanitize=thread -o 06_tsan 06_data_race_broken.c
 * Run           : ./06_polos   lalu   ./06_tsan
 *
 * Catatan: jalankan ./06_polos beberapa kali dan catat hasilnya.
 *          Hasilnya tidak deterministik, jadi jangan kaget kalau beda.
 */
#include <pthread.h>
#include <stdio.h>

#define NAIK   100000
#define NTHREAD 2

static long penghitung = 0;   /* shared, tanpa pelindung */

static void *naikkan(void *arg)
{
    (void)arg;
    for (int i = 0; i < NAIK; i++) {
        /* BUG: baca-modifikasi-tulis tanpa mutex. Dua thread bisa membaca
         * nilai yang sama lalu menulis hasil yang sama: satu kenaikan hilang. */
        penghitung = penghitung + 1;
    }
    return NULL;
}

int main(void)
{
    pthread_t tid[NTHREAD];

    for (int i = 0; i < NTHREAD; i++) {
        int rc = pthread_create(&tid[i], NULL, naikkan, NULL);
        if (rc != 0) {
            return 1;
        }
    }
    for (int i = 0; i < NTHREAD; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("penghitung = %ld  (harusnya %d)\n", penghitung, NAIK * NTHREAD);
    return 0;
}
