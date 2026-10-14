/*
 * Minggu 5 — Lab 2: mengamati peta memory process sendiri.
 *
 * /proc/self adalah symlink ajaib yang mengarah ke direktori /proc/<pid> dari
 * process yang sedang berjalan — jadi "/proc/self" dari dalam program selalu
 * berarti "diriku sendiri". File /proc/self/maps berisi satu baris untuk setiap
 * pemetaan (mapping) di ruang alamat process.
 *
 * Format kolom /proc/[pid]/maps:
 *     address           perms  offset   dev    inode   pathname
 *     55e4a8c00000-55e4a8c02000 r--p  00000000 fd:01  12345  /usr/bin/cat
 *
 * Arti perms:
 *     r = read, w = write, x = execute,
 *     s = shared, p = private (copy-on-write)
 *
 * Pathname:
 *     nama file        : pemetaan berbasis file
 *     [heap] / [stack] : path semu untuk heap dan stack
 *     [vdso]           : path semu untuk objek dinamis kernel
 *     kosong           : pemetaan anonim (biasanya hasil mmap tanpa file)
 * Inode 0 berarti tidak ada inode yang terkait (misalnya BSS).
 *
 * CATATAN LEVEL ABSTRAKSI: /proc adalah filesystem milik Linux. Ini BUKAN
 * fitur bahasa C dan BUKAN bagian dari POSIX. Program ini tidak akan jalan
 * di sistem yang tidak punya /proc.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_maps 02_proc_self_maps.c
 * Jalankan:
 *   ./02_maps
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void)
{
    FILE   *f = fopen("/proc/self/maps", "r");
    char   *baris = NULL;
    size_t  kapasitas = 0;
    ssize_t panjang;
    int     nomor = 1;

    if (f == NULL) {
        perror("fopen /proc/self/maps");
        return 1;
    }

    printf("Isi /proc/self/maps saat program ini berjalan:\n\n");
    printf("  #  address                   perms offset   dev    inode    pathname\n");
    printf(" ---  ------------------------  ----- -------- ------ -------- -----------------\n");

    while ((panjang = getline(&baris, &kapasitas, f)) != -1) {
        printf(" %3d  %s", nomor, baris);
        nomor++;
    }

    free(baris);
    if (fclose(f) != 0) {
        perror("fclose");
        return 1;
    }

    printf("\nCara membacanya:\n");
    printf("  - Kolom pertama adalah rentang ALAMAT VIRTUAL, bukan alamat RAM.\n");
    printf("  - Kolom kedua: r=read, w=write, x=execute, s=shared, p=private.\n");
    printf("  - Baris dengan path kosong adalah pemetaan anonim.\n");
    printf("  - Baris berlabel [heap] dan [stack] adalah label buatan kernel.\n");
    printf("  - Semua isi file ini BERUBAH setiap kali dijalankan (ASLR),\n");
    printf("    tetapi urutan dan jenis barisnya cenderung stabil.\n");

    return 0;
}
