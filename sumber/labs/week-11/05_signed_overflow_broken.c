/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Minggu 11 — Bug #5: signed integer overflow (undefined behavior)
 *
 * Gejala  : tanpa alat, program sering "berhasil" dengan nilai melingkar
 *           (wrap around). Dengan optimisasi, compiler boleh mengasumsikan
 *           overflow tidak pernah terjadi dan menghasilkan logika yang aneh.
 * Alat    : UndefinedBehaviorSanitizer (-fsanitize=undefined)
 *
 * Build (polos) : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 05_polos 05_signed_overflow_broken.c
 * Build (UBSan) : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=undefined -o 05_ubsan 05_signed_overflow_broken.c
 * Run           : ./05_polos   lalu   ./05_ubsan
 *
 * Eksperimen tambahan (lihat modul): bandingkan -O0 dan -O2 pada versi polos.
 */
#include <limits.h>
#include <stdio.h>

static int tambah_satu(int x)
{
    return x + 1;   /* BUG bila x == INT_MAX: overflow signed = UB */
}

int main(void)
{
    int x = INT_MAX;
    printf("INT_MAX        = %d\n", x);

    int y = tambah_satu(x);        /* UB terjadi di sini */
    printf("INT_MAX + 1    = %d\n", y);

    printf("apakah lebih besar? %s\n", (y > x) ? "ya" : "tidak");
    return 0;
}
