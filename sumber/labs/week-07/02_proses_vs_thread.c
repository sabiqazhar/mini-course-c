/* Minggu 7 — Process vs thread: siapa yang benar-benar berbagi memory?
 *
 * Tujuan: membuktikan sendiri bahwa
 *   - child process punya RUANG ALAMAT SENDIRI (menulis tidak terlihat parent),
 *   - thread dalam satu process berbagi RUANG ALAMAT YANG SAMA.
 *
 * Build : gcc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -g -O0 \
 *              -o 02_proses_vs_thread 02_proses_vs_thread.c -pthread
 * Run   : ./02_proses_vs_thread
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int bersama = 0;   /* satu objek, dipakai bersama */

static void *tambah_dari_thread(void *arg)
{
    (void)arg;
    bersama += 100;
    printf("  [thread] bersama = %d, alamat = %p\n",
           bersama, (void *)&bersama);
    return NULL;
}

int main(void)
{
    printf("nilai awal bersama = %d, alamat = %p\n", bersama, (void *)&bersama);

    /* ---------- bagian 1: PROCESS ---------- */
    printf("\n--- fork(): PROCESS baru ---\n");

    /* stdout bisa saja masih berisi data yang belum ditulis ke terminal.
     * Kalau tidak dikosongkan dulu, child mewarisi buffer itu dan bisa
     * mencetak baris yang sama dua kali. */
    fflush(NULL);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        /* child: mengubah salinannya sendiri */
        bersama += 100;
        printf("  [child ] bersama = %d, alamat = %p\n",
               bersama, (void *)&bersama);
        /* _exit() tidak mengosongkan buffer stdio, jadi kita lakukan sendiri. */
        fflush(NULL);
        _exit(0);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    printf("  [parent] setelah child selesai, bersama = %d\n", bersama);

    /* ---------- bagian 2: THREAD ---------- */
    printf("\n--- pthread_create(): THREAD baru dalam process yang sama ---\n");
    pthread_t tid;
    int rc = pthread_create(&tid, NULL, tambah_dari_thread, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create gagal: rc=%d\n", rc);
        return 1;
    }
    rc = pthread_join(tid, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_join gagal: rc=%d\n", rc);
        return 1;
    }
    printf("  [parent] setelah thread selesai, bersama = %d\n", bersama);

    return 0;
}
