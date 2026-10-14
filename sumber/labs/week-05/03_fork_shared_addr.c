/* Minggu 5 — Dua process, alamat virtual SAMA, memory fisik BERBEDA
 * Tujuan: membuktikan bahwa virtual address identik antar process tidak berarti
 *         objek fisik yang sama. Ini inti dari isolasi process.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_fork_shared_addr 03_fork_shared_addr.c
 * Run   : ./03_fork_shared_addr
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var = 0;         /* ada di region data, alamat virtualnya sama di parent & child */

int main(void)
{
    int *heap = malloc(sizeof *heap);
    if (heap == NULL) return 1;
    *heap = 0;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); free(heap); return 1; }

    if (pid == 0) {                       /* CHILD */
        *heap = 42;
        global_var = 42;
        printf("CHILD : &global=%p heap=%p *heap=%d global=%d\n",
               (void *)&global_var, (void *)heap, *heap, global_var);
        fflush(stdout);          /* child memakai _exit; flush dulu agar tidak hilang */
        free(heap);
        _exit(0);
    }

    /* PARENT: tunggu child selesai, lalu periksa nilai sendiri */
    int status;
    wait(&status);

    printf("PARENT: &global=%p heap=%p *heap=%d global=%d\n",
           (void *)&global_var, (void *)heap, *heap, global_var);

    free(heap);

    printf("\nAlamat virtual SAMA (copy-on-write), tapi perubahan child\n");
    printf("TIDAK terlihat di parent => memory fisiknya BERBEDA.\n");
    return 0;
}
