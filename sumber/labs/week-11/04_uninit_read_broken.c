/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #4: membaca variabel yang belum diberi nilai
 *
 * Gejala  : hasil perhitungan berubah-ubah, atau "anehnya benar" di mesin
 *           tertentu. Tidak pernah crash.
 * Alat    : peringatan compiler (-Wall -Wextra, kadang -Wmaybe-uninitialized),
 *           GDB untuk melihat isi variabel, Valgrind Memcheck bila tersedia.
 * Catatan jujur: AddressSanitizer dan UBSan TIDAK menangkap bug ini.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_uninit 04_uninit_read_broken.c
 * Run   : ./04_uninit          (tanpa argumen -> jalur yang memicu bug)
 *         ./04_uninit 1        (dengan argumen -> jalur yang benar)
 */
#include <stdio.h>

static int total_semua(const int *daftar, int n, int boleh_nol)
{
    int total;                 /* tidak diinisialisasi */

    if (boleh_nol) {
        total = 0;             /* hanya diisi di satu jalur */
    }

    for (int i = 0; i < n; i++) {
        total += daftar[i];    /* BUG: bisa membaca nilai sampah */
    }
    return total;
}

int main(int argc, char **argv)
{
    int daftar[4] = { 5, 10, 15, 20 };

    int boleh_nol = (argc > 1);   /* tanpa argumen: 0 -> jalur rusak */
    int total = total_semua(daftar, 4, boleh_nol);

    printf("boleh_nol = %d, total = %d (harusnya 50)\n", boleh_nol, total);
    return 0;
}
