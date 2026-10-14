/*
 * Minggu 5 — Lab 3: dua process, alamat sama, nilai berbeda.
 *
 * Ini percobaan paling meyakinkan di minggu ini.
 *
 * Satu program membuat satu objek di heap, lalu memanggil fork() sehingga
 * menjadi dua process: induk (parent) dan anak (child). Keduanya mewarisi
 * nilai pointer yang sama persis. Lalu anak mengubah isi objek itu.
 *
 * Kalau alamat itu "alamat RAM yang nyata dan bersama", induk pasti melihat
 * perubahan itu. Kalau alamat itu virtual address milik masing-masing process,
 * induk tidak akan melihat apa-apa.
 *
 * Level abstraksi:
 *   - fork(), waitpid(), _exit(): POSIX, bukan bagian dari bahasa C.
 *   - Konsep "tiap process punya ruang alamat sendiri": konsep sistem operasi,
 *     berlaku di semua sistem mirip Unix modern (dan juga Windows), bukan
 *     aturan bahasa C.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_dua_proses 03_dua_proses_alamat_sama.c
 * Jalankan:
 *   ./03_dua_proses
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int  *angka = malloc(sizeof *angka);
    pid_t pid;

    if (angka == NULL) {
        fprintf(stderr, "malloc gagal\n");
        return 1;
    }

    *angka = 100;
    printf("sebelum fork : angka = %p, *angka = %d  (pid %ld)\n",
           (void *)angka, *angka, (long)getpid());

    fflush(NULL);   /* kosongkan buffer stdio sebelum fork,
                       supaya baris di atas tidak tercetak dua kali */

    pid = fork();
    if (pid == -1) {
        perror("fork");
        free(angka);
        return 1;
    }

    if (pid == 0) {
        /* ---- process anak ---- */
        printf("anak         : angka = %p, *angka = %d  (pid %ld)\n",
               (void *)angka, *angka, (long)getpid());

        *angka = 200;                       /* anak mengubah isinya */
        printf("anak         : setelah *angka = 200 -> *angka = %d\n", *angka);

        /*
         * _exit() mengakhiri process TANPA menjalankan pembersihan stdio.
         * Karena stdout ke pipe/terminal bisa ter-buffer, baris yang baru saja
         * kita printf() belum tentu sampai ke layar. Karena itu kita flush
         * secara eksplisit dulu. (Ini salah satu jebakan fork yang paling
         * sering membuat pemula bingung: "kok output anak hilang?")
         */
        fflush(NULL);

        free(angka);
        _exit(0);                           /* anak selesai */
    }

    /* ---- process induk ---- */
    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        free(angka);
        return 1;
    }

    printf("induk        : angka = %p, *angka = %d  (pid %ld)\n",
           (void *)angka, *angka, (long)getpid());
    printf("induk        : status keluar anak = %d\n", WEXITSTATUS(status));

    printf("\nPerhatikan: alamatnya SAMA, nilainya BERBEDA.\n");
    printf("Itu bukti bahwa alamat itu bukan alamat fisik RAM yang dipakai\n");
    printf("bersama, melainkan virtual address milik masing-masing process.\n");

    free(angka);
    return 0;
}
