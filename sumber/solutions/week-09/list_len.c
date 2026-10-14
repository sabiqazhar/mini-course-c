/* Minggu 9 — Solusi mudah: hitung panjang list tanpa memodifikasi.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o list_len list_len.c
 * Run  : ./list_len
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

static int list_len(const Node *head)
{
    int n = 0;
    for (const Node *cur = head; cur != NULL; cur = cur->next) n++;
    return n;
}

int main(void)
{
    Node a = {1, NULL}, b = {2, &a}, c = {3, &b};
    printf("panjang = %d (harus 3)\n", list_len(&c));
    return 0;
}
