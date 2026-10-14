/* CONTOH RUSAK — JANGAN DITIRU
 *
 * Fungsi ini mengembalikan alamat variabel lokal. Objek `x` hidup hanya selama
 * eksekusi fungsi `rusak`; begitu fungsi kembali, lifetime objek itu berakhir.
 * Pointer yang dikembalikan menjadi dangling. Mengaksesnya adalah undefined behavior.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_rusak 03_return_local_broken.c
 * Run   : ./03_rusak   (bisa crash, bisa cetak sampah, bisa "kebetulan benar")
 *
 * Compiler biasanya memperingatkan: "function returns address of local variable".
 * Jangan abaikan peringatan itu.
 */
#include <stdio.h>

int *rusak(void)
{
    int x = 42;     /* objek lokal, storage duration = automatic */
    return &x;       /* UNDEFINED BEHAVIOR: mengembalikan alamat objek yang segera mati */
}

int main(void)
{
    int *p = rusak();
    printf("%d\n", *p);    /* UNDEFINED BEHAVIOR: dereference dangling pointer */
    return 0;
}
