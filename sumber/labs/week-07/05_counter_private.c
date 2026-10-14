/* Minggu 7 — Versi yang benar TANPA sinkronisasi: akumulasi privat lalu gabung
 *
 * Idelnya: kalau tiap thread bekerja pada objeknya SENDIRI, tidak ada data
 * race, karena data race butuh dua thread menyentuh memory location yang sama.
 *
 * Di sini:
 *   - tiap thread menaikkan penghitung LOKAL (di stack thread itu sendiri),
 *   - hasilnya ditulis ke slot array yang berbeda (hasil[i]),
 *   - penggabungan dilakukan di main SETELAH semua pthread_join selesai.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 05_counter_private 05_counter_private.c
 * Run   : ./05_counter_private
 * TSan  : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fsanitize=thread -pthread \
 *              -o 05_counter_private_tsan 05_counter_private.c
 */
#include <pthread.h>
#include <stdio.h>

#define JUMLAH_THREAD 4
#define TOTAL         400000L   /* jumlah kenaikan yang DIKETAHUI sebelumnya */

static long hasil[JUMLAH_THREAD];   /* slot berbeda untuk tiap thread */

struct tugas {
    int           nomor;    /* indeks slot, sekaligus identitas   */
    unsigned long bagian;   /* berapa kali thread ini menaikkan   */
};

static void *hitung(void *arg)
{
    struct tugas *t = arg;

    long lokal = 0;                 /* PRIVATE: di stack thread ini */
    for (unsigned long i = 0; i < t->bagian; i++) {
        lokal++;
    }

    hasil[t->nomor] = lokal;        /* slot yang berbeda -> bukan data race */
    return NULL;
}

int main(void)
{
    pthread_t   tid[JUMLAH_THREAD];
    struct tugas tugas[JUMLAH_THREAD];

    for (int i = 0; i < JUMLAH_THREAD; i++) {
        tugas[i].nomor  = i;
        tugas[i].bagian = (unsigned long)(TOTAL / JUMLAH_THREAD);
        int rc = pthread_create(&tid[i], NULL, hitung, &tugas[i]);
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

    /* Menggabungkan di sini aman: semua thread sudah selesai (join terjadi
     * sebelum baris ini), jadi hanya ada satu alur eksekusi yang menyentuh
     * variabel bersama. */
    long total = 0;
    for (int i = 0; i < JUMLAH_THREAD; i++) {
        printf("slot[%d] = %ld\n", i, hasil[i]);
        total += hasil[i];
    }

    printf("total    = %ld  (diharapkan %ld)\n", total, (long)TOTAL);
    printf("sesuai   = %s\n", total == (long)TOTAL ? "ya" : "tidak");
    return 0;
}
