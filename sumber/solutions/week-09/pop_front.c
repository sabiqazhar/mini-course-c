/* Minggu 9 — Solusi menengah: pop_front mengembalikan nilai lewat *out dan head baru.
 *
 * Kontrak: fungsi membebaskan node depan DI DALAM fungsi, mengembalikan head baru.
 * Bila list kosong, kembalikan NULL dan *out TIDAK diubah. Pemanggil cukup mengikuti
 * head baru yang dikembalikan; tidak perlu (dan tidak boleh) mem-free node lagi.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o pop_front pop_front.c
 * Run  : ./pop_front
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

static Node *pop_front(Node *head, int *out)
{
    if (head == NULL) return NULL;        /* kosong: jangan ubah *out */
    Node *rest = head->next;
    if (out) *out = head->val;
    free(head);                          /* node depan dibebaskan di sini */
    return rest;
}

int main(void)
{
    Node *h = malloc(sizeof *h); h->val = 10;
    h->next = malloc(sizeof *h); h->next->val = 20; h->next->next = NULL;

    int v;
    while (h != NULL) {
        h = pop_front(h, &v);            /* h digeser ke head baru (atau NULL) */
        printf("pop -> %d\n", v);
    }
    /* h == NULL: list kosong, tidak ada node yang bocor */
    return 0;
}
