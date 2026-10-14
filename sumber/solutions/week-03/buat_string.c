/* Solusi latihan tantangan minggu 3 — fungsi mengembalikan string yang dialokasikan
 *
 * Kontrak: panggil free() pada hasil jika tidak NULL.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o buat_string buat_string.c
 * Run   : ./buat_string
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *greeting(const char *nama)
{
    const char *awalan = "Halo, ";
    size_t len = strlen(awalan) + strlen(nama) + 1;

    char *s = malloc(len);
    if (s == NULL)
        return NULL;

    snprintf(s, len, "%s%s", awalan, nama);
    return s;
}

int main(void)
{
    char *s = greeting("Dunia");
    if (s == NULL) {
        fprintf(stderr, "alokasi gagal\n");
        return 1;
    }
    printf("%s\n", s);
    free(s);        /* pemanggil melepas */
    return 0;
}
