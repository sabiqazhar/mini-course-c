/* CONTOH RUSAK — JANGAN DITIRU (tetapi jalankan untuk melihat pelajarannya)
 *
 * Dua thread menambah counter bersama TANPA sinkronisasi. Ini data race ->
 * undefined behavior di C (C11 5.1.2.4). Hasil tidak dapat diprediksi.
 *
 * Jalankan dua cara:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 02_race 02_race_basic.c
 *   ./02_race
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fsanitize=thread -pthread -o 02_race_tsan 02_race_basic.c
 *   ./02_race_tsan
 *
 * Pelajaran: sekali jalan mungkin tampak benar ("counter = 200000"). Itu BUKAN bukti
 * program aman. Jalankan berkali-kali; dengan TSan kamu akan melihat laporan data race.
 */
#include <stdio.h>
#include <pthread.h>

static long counter = 0;
#define N 100000

static void *inc(void *v)
{
    (void)v;
    for (int i = 0; i < N; i++)
        counter++;            /* UNDEFINED BEHAVIOR: akses bersama tanpa sinkronisasi */
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, inc, NULL);
    pthread_create(&b, NULL, inc, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);

    printf("counter = %ld  (kalau aman, harusnya %d)\n", counter, 2 * N);
    return 0;
}
