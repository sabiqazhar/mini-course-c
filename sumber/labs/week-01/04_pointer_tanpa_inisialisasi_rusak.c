/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Pointer yang tidak diinisialisasi: nilainya tidak tentu (indeterminate).
 * Men-dereference-nya adalah undefined behavior. Program ini bisa crash,
 * bisa mencetak angka aneh, atau (yang paling berbahaya) tampak berjalan normal.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_rusak 04_pointer_tanpa_inisialisasi_rusak.c
 * Run   : ./04_rusak   (kemungkinan besar SIGSEGV; kalau tidak, itu pun bagian dari masalahnya)
 *
 * Gunakan contoh ini hanya untuk melihat seperti apa bentuk kegagalannya.
 */
#include <stdio.h>

int main(void)
{
    int *p;          /* tidak diinisialisasi — isinya tidak tentu */

    printf("nilai p (tidak tentu) = %p\n", (void *)p);

    *p = 123;        /* UNDEFINED BEHAVIOR */
    printf("*p = %d\n", *p);

    return 0;
}
