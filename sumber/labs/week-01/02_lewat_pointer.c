/* Minggu 1 — Mengubah nilai lewat pointer
 * Tujuan: membuktikan bahwa menulis lewat pointer mengubah objek aslinya.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_lewat_pointer 02_lewat_pointer.c
 * Run   : ./02_lewat_pointer
 */
#include <stdio.h>

int main(void)
{
    int  x = 5;
    int *p = &x;

    printf("sebelum  : x = %d, *p = %d\n", x, *p);

    *p = 10;                       /* menulis ke objek yang ditunjuk p */

    printf("sesudah  : x = %d, *p = %d\n", x, *p);

    printf("\np  (nilai p)      = %p\n", (void *)p);
    printf("&x (alamat x)     = %p\n", (void *)&x);
    printf("&p (alamat p)     = %p   <-- p sendiri juga punya alamat\n", (void *)&p);

    printf("\nsizeof p  = %zu byte  (ukuran si pointer)\n", sizeof p);
    printf("sizeof *p = %zu byte  (ukuran objek yang ditunjuk)\n", sizeof *p);

    return 0;
}
