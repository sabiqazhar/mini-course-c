/* Minggu 7 — Solusi mudah: 3 thread dengan id berbeda, semua di-join.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o three_threads three_threads.c
 * Run  : ./three_threads
 */
#include <stdio.h>
#include <pthread.h>

static void *worker(void *arg)
{
    long id = (long)arg;
    printf("thread id=%ld jalan\n", id);
    fflush(stdout);
    return NULL;
}

int main(void)
{
    pthread_t t[3];
    for (long i = 0; i < 3; i++)
        pthread_create(&t[i], NULL, worker, (void *)i);
    for (int i = 0; i < 3; i++)
        pthread_join(t[i], NULL);
    return 0;
}
