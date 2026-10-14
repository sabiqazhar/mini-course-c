/*
 * CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 5 — Lab 4: contoh kesimpulan yang terlalu cepat.
 *
 * Program ini sengaja ditulis salah. Ia mengambil beberapa alamat, lalu
 * menarik kesimpulan yang TIDAK bisa dipertanggungjawabkan.
 *
 * Tiga kesalahan di sini:
 *   1. Membandingkan dua pointer yang tidak berkaitan dengan operator < atau >.
 *      Dalam standar C itu undefined behavior: operasi relasional pada pointer
 *      hanya terdefinisi untuk pointer yang menunjuk ke dalam objek/array yang
 *      sama. (&lokal dan heap bukan objek yang sama.)
 *   2. Mengurangi dua pointer yang tidak berkaitan. Itu juga undefined behavior.
 *   3. Menganggap hasil pengurangan alamat sebagai "jarak fisik di RAM". Ini
 *      salah secara konseptual, karena angka yang kita pegang adalah virtual
 *      address, bukan alamat fisik.
 *
 * Kesimpulan "heap pasti di bawah stack" juga tidak bisa digeneralisasi:
 * tata letak memory bergantung pada platform, kernel, dan opsi build.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_rusak 04_kesimpulan_tergesa_rusak.c
 * Jalankan:
 *   ./04_rusak
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int  lokal = 1;
    int *heap  = malloc(sizeof *heap);

    if (heap == NULL) {
        fprintf(stderr, "malloc gagal\n");
        return 1;
    }
    *heap = 2;

    /* SALAH (1): operasi relasional pada pointer yang tidak berkaitan */
    if (&lokal > heap) {
        printf("Kesimpulan: stack SELALU berada di alamat lebih tinggi dari heap.\n");
    } else {
        printf("Kesimpulan: heap berada di alamat lebih tinggi dari stack.\n");
    }

    /* SALAH (2) dan (3): mengurangi dua pointer berbeda, lalu menafsirkannya
       sebagai jarak fisik di RAM */
    printf("Jarak fisik antara objek itu di RAM: %ld byte\n",
           (long)((char *)&lokal - (char *)heap));

    printf("Alamat fisik RAM objek lokal (dihitung sendiri): 0x%lx\n",
           (unsigned long)&lokal);

    free(heap);
    return 0;
}
