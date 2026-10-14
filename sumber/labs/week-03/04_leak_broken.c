/* CONTOH RUSAK — JANGAN DITIRU (tetapi jalankan untuk melihat deteksinya)
 *
 * Tiap iterasi mengalokasikan memory lalu "melupakan" pointer-nya tanpa free.
 * Inilah memory leak: blok dialokasikan namun tidak pernah dikembalikan.
 *
 * Untuk melihatnya, kompilasi dengan AddressSanitizer (LeakSanitizer ikut aktif):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address \
 *       -o 04_leak 04_leak_broken.c
 *   ./04_leak
 *
 * Tanpa sanitizer, program biasanya selesai "diam" karena OS mengambil kembali
 * semua memory saat process berakhir. Itu sebabnya leak sering luput — ia baru
 * terasa pada program yang lama berjalan.
 */
#include <stdlib.h>

int main(void)
{
    for (int i = 0; i < 5; i++) {
        int *p = malloc(sizeof *p);
        *p = i;        /* blok dialokasikan... */
    }                  /* ...tetapi p keluar scope, alamatnya hilang, tak pernah free */
    return 0;
}
