/* Minggu 9 — Solusi tantangan: reverse list dengan mengubah pointer (tanpa alokasi baru).
 *
 * State per langkah: pegang prev=NULL, cur=head; tiap iterasi simpan next, balik
 * panah cur->next = prev, lalu geser. Head baru = node terakhir.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o reverse reverse.c
 * Run  : ./reverse
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

static Node *reverse(Node *head)
{
    Node *prev = NULL;
    Node *cur = head;
    while (cur != NULL) {
        Node *next = cur->next;   /* (1) simpan dulu */
        cur->next = prev;          /* (2) balik panah */
        prev = cur;                /* (3) geser prev  */
        cur = next;                /* (4) geser cur   */
    }
    return prev;                   /* prev = head baru */
}

static void destroy(Node *head)
{
    while (head) { Node *n = head->next; free(head); head = n; }
}

static void print(const Node *head)
{
    for (const Node *c = head; c; c = c->next) printf("%d ", c->val);
    printf("\n");
}

int main(void)
{
    Node *h = malloc(sizeof *h); h->val = 1;
    h->next = malloc(sizeof *h); h->next->val = 2;
    h->next->next = malloc(sizeof *h); h->next->next->val = 3;
    h->next->next->next = NULL;

    print(h);            /* 1 2 3 */
    h = reverse(h);
    print(h);            /* 3 2 1 */
    destroy(h);
    return 0;
}
