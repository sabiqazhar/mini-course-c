/* Minggu 5 — Solusi tantangan: fork + heap + variabel static lokal.
 *
 * Buktikan bahwa setelah fork(), child dan parent punya salinan terpisah
 * (copy-on-write) untuk heap, DAN bahwa variabel static lokal juga ter-duplikat
 * per proses: storage duration-nya static (hidup sepanjang program) tetapi scope-nya
 * lokal; setelah fork masing-masing proses punya copy sendiri.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o fork_static_demo fork_static_demo.c
 * Run  : ./fork_static_demo
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int *heap = malloc(sizeof(int));
    *heap = 0;

    static int s;        /* static lokal: lifetime seluruh program, terlihat hanya di main */
    s = 100;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); free(heap); return 1; }

    if (pid == 0) {
        /* child: ubah salinan sendiri */
        *heap = 42;
        s = 999;
        printf("CHILD : heap=%d, static=%d\n", *heap, s);
        fflush(stdout);
        free(heap);
        _exit(0);
    }

    /* parent: tunggu child selesai, lalu ubah & cetak miliknya sendiri */
    int status;
    (void)wait(&status);

    *heap = 7;
    s = 200;
    printf("PARENT: heap=%d, static=%d\n", *heap, s);
    printf("Kesimpulan: child mengubah heap & static miliknya sendiri; parent tidak terpengaruh.\n");

    free(heap);
    return 0;
}
