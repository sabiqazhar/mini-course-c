/* Minggu 11 — Program dengan bug logika, untuk dilatih dengan GDB
 * Bug: sum_to_n menghitung 1..(n-1), bukan 1..n.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 04_gdb_demo 04_gdb_demo.c
 * Debug : gdb -batch -ex 'break sum_to_n' -ex run -ex 'print n' -ex 'step' -ex 'print i' -ex 'print s' -ex 'continue' ./04_gdb_demo
 */
#include <stdio.h>

static int sum_to_n(int n)
{
    int s = 0;
    int i = 1;
    while (i < n) {        /* BUG: harus i <= n */
        s += i;
        i++;
    }
    return s;
}

int main(void)
{
    printf("sum 1..5 = %d  (harusnya 15)\n", sum_to_n(5));
    return 0;
}
