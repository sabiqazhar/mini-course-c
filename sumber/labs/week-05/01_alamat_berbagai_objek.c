/*
 * Minggu 5 — Lab 1: mencetak alamat berbagai jenis objek.
 *
 * Pertanyaan minggu ini: apakah alamat yang dicetak program itu alamat fisik RAM?
 * Program ini mencetak alamat dari beberapa jenis objek yang berbeda, lalu
 * membiarkan kamu membandingkannya sendiri.
 *
 * Level abstraksi — baca ini sebelum menyimpulkan apa pun:
 *   - Bahasa C (standar): standar tidak pernah menjanjikan bahwa fungsi, objek
 *     global, objek lokal, dan hasil malloc berada di wilayah memory yang
 *     berbeda. Semua itu detail implementasi.
 *   - Linux x86-64: pada praktiknya mereka memang berada di wilayah berbeda,
 *     dan kita bisa mengamatinya di lab 2 lewat /proc/self/maps.
 *
 * JANGAN menyimpulkan "ini stack, ini heap" hanya dari urutan angka di sini.
 * Satu kali percobaan bukan bukti. Lihat lab 2 untuk cara yang lebih dapat
 * diandalkan.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_alamat 01_alamat_berbagai_objek.c
 * Jalankan:
 *   ./01_alamat
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ---------- objek dengan static storage duration ---------- */

int          global_bernilai   = 42;   /* diberi nilai awal        */
int          global_tanpa_nilai;       /* tidak diberi nilai awal  */
const char  *global_teks       = "halo dari string literal";

static void contoh_fungsi(void)
{
    /* sengaja kosong: yang kita butuhkan hanya alamatnya */
}

/*
 * Mencetak alamat sebuah fungsi butuh sedikit trik.
 *
 * Standar C tidak menjamin bahwa pointer ke fungsi dan pointer ke objek punya
 * representasi yang sama, jadi menulis (void *)contoh_fungsi bukan hal yang
 * dijamin portabel (GCC dengan -Wpedantic akan memperingatkannya).
 *
 * Di sini kita menyalin byte-byte representasi pointer fungsi ke dalam sebuah
 * objek void * lewat memcpy. Ini tidak mencetak "isi" fungsi — hanya menyalin
 * representasi alamatnya supaya bisa dicetak dengan %p.
 */
static void cetak_alamat_fungsi(void)
{
    void (*penunjuk_fungsi)(void) = contoh_fungsi;
    void *alamat = NULL;

    memcpy(&alamat, &penunjuk_fungsi, sizeof alamat);
    printf("  fungsi   contoh_fungsi       : %p\n", alamat);
}

int main(void)
{
    int   lokal        = 7;      /* automatic storage duration  */
    static int statik_lokal = 8; /* static storage duration     */
    int  *heap         = malloc(sizeof *heap);

    if (heap == NULL) {
        fprintf(stderr, "malloc gagal\n");
        return 1;
    }
    *heap = 99;

    printf("Alamat yang terlihat program ini (satu kali jalan):\n\n");

    cetak_alamat_fungsi();
    printf("  global   global_bernilai     : %p\n", (void *)&global_bernilai);
    printf("  global   global_tanpa_nilai  : %p\n", (void *)&global_tanpa_nilai);
    printf("  literal  \"halo dari...\"      : %p\n", (void *)global_teks);
    printf("  static   statik_lokal        : %p\n", (void *)&statik_lokal);
    printf("  lokal    lokal (di main)     : %p\n", (void *)&lokal);
    printf("  heap     malloc(sizeof int)  : %p\n", (void *)heap);
    printf("  lokal    &heap (si pointer)  : %p\n", (void *)&heap);

    printf("\nUkuran page menurut sistem : %ld byte\n", sysconf(_SC_PAGE_SIZE));
    printf("Nilai *heap                : %d\n", *heap);

    printf("\nPERINGATAN:\n");
    printf("  1. Semua angka 0x... di atas akan BERBEDA setiap kali program\n");
    printf("     dijalankan (ASLR). Jalankan program ini 5 kali untuk melihatnya.\n");
    printf("  2. Jangan menyimpulkan wilayah memory dari urutan angka di atas.\n");
    printf("     Gunakan /proc/self/maps (lab 2).\n");
    printf("  3. Angka-angka ini BUKAN alamat fisik RAM. Ini virtual address.\n");

    free(heap);
    return 0;
}
