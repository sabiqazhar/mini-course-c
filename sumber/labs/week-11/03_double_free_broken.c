/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #3: double free (melepas blok yang sama dua kali)
 *
 * Gejala  : sering baru muncul saat program sudah besar: glibc mencetak
 *           "free(): double free detected in tcache 2" lalu abort, atau
 *           memory rusak tanpa pesan sama sekali.
 * Alat    : AddressSanitizer (-fsanitize=address)
 *
 * Build (polos) : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_polos 03_double_free_broken.c
 * Build (ASan)  : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address -o 03_asan 03_double_free_broken.c
 * Run           : ./03_polos   lalu   ./03_asan
 */
#include <stdio.h>
#include <stdlib.h>

static void simpan(int *p)
{
    /* Fungsi ini mengira dirinya pemilik p, padahal pemanggil juga akan
     * memanggil free(). Kepemilikan tidak terdokumentasi -> double free. */
    free(p);
}

int main(void)
{
    int *data = malloc(sizeof *data);
    if (data == NULL) {
        return 1;
    }

    *data = 7;
    printf("nilai = %d\n", *data);

    simpan(data);

    /* BUG: blok yang sama dilepas untuk kedua kalinya. */
    free(data);

    printf("selesai\n");
    return 0;
}
