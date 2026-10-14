/* CONTOH RUSAK — JANGAN DITIRU (tetapi aman dihentikan)
 *
 * Deadlock klasik: dua thread mengunci dua mutex dalam urutan berlawanan.
 * Jalankan dengan timeout agar tidak menggantung shellmu:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o 02_deadlock 02_deadlock_broken.c
 *   timeout 5 ./02_deadlock     # berhenti otomatis setelah 5 detik
 *
 * Pelajaran: urutan penguncian (lock ordering) harus konsisten di semua thread.
 */
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

static void *urut_A(void *v)
{
    (void)v;
    pthread_mutex_lock(&m1);
    pthread_mutex_lock(&m2);     /* thread A: m1 lalu m2 */
    printf("thread A memegang m1 dan m2\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    return NULL;
}

static void *urut_B(void *v)
{
    (void)v;
    pthread_mutex_lock(&m2);
    pthread_mutex_lock(&m1);     /* thread B: m2 lalu m1  -> berlawanan dengan A */
    printf("thread B memegang m2 dan m1\n");
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m2);
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, urut_A, NULL);
    pthread_create(&b, NULL, urut_B, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    return 0;
}
