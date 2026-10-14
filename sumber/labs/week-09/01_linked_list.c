/* Minggu 9 — Linked list dengan kontrak ownership yang jelas
 * Tujuan: membangun struktur data dinamis; setiap operasi menjaga invariant
 *         dan kepemilikan (siapa yang memanggil free).
 *
 * Kontrak:
 *   - push_front mengalokasikan node; mengembalikan head baru (atau head lama bila gagal).
 *   - destroy membebaskan SETIAP node tepat satu kali, lalu list kosong.
 *   - Tidak ada double free, tidak ada akses setelah free.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_linked_list 01_linked_list.c
 * Run   : ./01_linked_list
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int           val;
    struct Node  *next;
} Node;

/* Mengembalikan head baru. Bila alokasi gagal, head lama tetap valid (tak bocor). */
static Node *push_front(Node *head, int v)
{
    Node *n = malloc(sizeof *n);
    if (n == NULL)
        return head;
    n->val = v;
    n->next = head;
    return n;
}

static Node *find(Node *head, int v)
{
    for (; head != NULL; head = head->next)
        if (head->val == v)
            return head;
    return NULL;
}

/* Menghapus node bernilai v (pertama yang ditemukan). Mengembalikan head baru. */
static Node *remove_val(Node *head, int v)
{
    Node *prev = NULL, *cur = head;
    while (cur != NULL) {
        if (cur->val == v) {
            Node *next = cur->next;
            free(cur);                 /* bebaskan tepat satu kali */
            if (prev != NULL) prev->next = next;
            else              head = next;
            return head;
        }
        prev = cur;
        cur = cur->next;
    }
    return head;                       /* tak ditemukan: tak ada yang di-free */
}

static void destroy(Node *head)
{
    while (head != NULL) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

static void print(Node *head)
{
    for (; head != NULL; head = head->next)
        printf("%d ", head->val);
    printf("\n");
}

int main(void)
{
    Node *list = NULL;
    for (int i = 3; i >= 1; i--)
        list = push_front(list, i);    /* hasil: 1 2 3 */

    printf("sebelum      : "); print(list);
    printf("cari 2       : %s\n", find(list, 2) ? "ditemukan" : "tidak");

    list = remove_val(list, 2);
    printf("setelah hapus: "); print(list);

    list = remove_val(list, 99);       /* tidak ada: tak error, tak leak */
    printf("hapus 99     : "); print(list);

    destroy(list);                     /* bebaskan semua node */
    list = NULL;
    return 0;
}
