/* Minggu 11 — Solusi tantangan: demonstrasi UB membaca variabel tak terinisialisasi.
 *
 * Penting: UBSan (-fsanitize=undefined) umumnya TIDAK menangkap pembacaan
 * tak-terinisialisasi. Itu ranah MemorySanitizer (MSan), yang tidak terinstal di
 * mesin ini. Yang menangkapnya saat compile adalah peringatan -Wuninitialized /
 * -Wmaybe-uninitialized. Jadi: kompilasi dengan -Wall -Wextra akan memperingatkan,
 * sedangkan ./program di bawah UBSan biasanya diam (nilai tak tentu, UB).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=undefined -o ub_uninit_demo ub_uninit_demo.c
 * Run  : ./ub_uninit_demo
 */
#include <stdio.h>

int main(void)
{
    int x;                 /* tak terinisialisasi */
    int y = x + 1;         /* pembacaan tak terinisialisasi = UB; UBSan biasanya diam */
    printf("y = %d (nilai tak tentu — jangan andalkan)\n", y);
    return 0;
}
