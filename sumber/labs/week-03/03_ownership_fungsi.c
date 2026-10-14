/* Minggu 3 — Ownership yang jelas: fungsi mengalokasikan, pemanggil melepas
 *
 * Kontrak (tulis di dokumentasi, bukan hanya di kepala):
 *   - buat_array() mengembalikan pointer ke array int yang dialokasikan,
 *     atau NULL bila alokasi gagal.
 *   - Pemanggil (main) bertanggung jawab memanggil free() pada hasil itu.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_ownership_fungsi 03_ownership_fungsi.c
 * Run   : ./03_ownership_fungsi
 */
#include <stdio.h>
#include <stdlib.h>

static int *buat_array(size_t n)
{
    return malloc(n * sizeof(int));
}

int main(void)
{
    int *data = buat_array(3);
    if (data == NULL) {
        fprintf(stderr, "alokasi gagal\n");
        return 1;
    }

    data[0] = 1; data[1] = 2; data[2] = 3;
    printf("data[1] = %d\n", data[1]);

    free(data);    /* ownership kembali ke main; bebaskan di sini */
    return 0;
}
