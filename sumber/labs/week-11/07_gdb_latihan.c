/* Minggu 11 — Program latihan GDB (program ini BENAR, bukan contoh rusak)
 *
 * Dipakai untuk berlatih: breakpoint, run, next, step, print, backtrace,
 * perintah x (memeriksa memory), dan watchpoint.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 07_gdb 07_gdb_latihan.c
 * Run   : ./07_gdb
 *
 * Transkrip non-interaktif yang bisa kamu ulang persis:
 *   gdb -batch -ex 'break 22' -ex 'run' -ex 'print s' -ex 'backtrace' ./07_gdb
 */
#include <stdio.h>

#define N 5

static int jumlah(const int *a, int n)
{
    int s = 0;                       /* breakpoint yang enak ada di sini */
    for (int i = 0; i < n; i++) {
        s += a[i];
    }
    return s;
}

static int proses(const int *data, int n)
{
    int j = jumlah(data, n);
    return j * 2;
}

int main(void)
{
    int data[N] = { 1, 2, 3, 4, 5 };

    int hasil = proses(data, N);

    printf("jumlah = %d, hasil = %d\n", jumlah(data, N), hasil);
    return 0;
}
