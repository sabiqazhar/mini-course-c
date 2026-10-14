/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Mirip 02_race_basic, tetapi mencetak dari dua thread tanpa sinkronisasi agar kamu
 * melihat interleaving. Output dua baris bisa saling tumpang-tindih (tergantung timing).
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 04_race_print 04_race_print_broken.c
 * Run   : ./04_race_print
 */
#include <pthread.h>
#include <stdio.h>

static void *cetak(void *v)
{
    char nama = *(char *)v;
    for (int i = 0; i < 20; i++)
        printf("thread %c baris %d\n", nama, i);   /* printf bukan atomic lintas thread */
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    char na = 'A', nb = 'B';
    pthread_create(&a, NULL, cetak, &na);
    pthread_create(&b, NULL, cetak, &nb);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    return 0;
}
