/* Minggu 9 — Lab 1: satu node, lalu tiga node yang disambung
 *
 * Tujuan: melihat dengan mata kepala sendiri bahwa "linked list" bukanlah
 * wadah ajaib, melainkan beberapa struct yang masing-masing menyimpan alamat
 * struct berikutnya.
 *
 * Kontrak ownership di file ini:
 *   - Setiap node dibuat oleh pemanggil dengan malloc().
 *   - Pemanggil yang membuat node adalah PEMILIK node itu.
 *   - Pemilik wajib memanggil free() untuk setiap node yang ia buat.
 *   - Node hanyalah "tahu" alamat node berikutnya lewat field next;
 *     ia TIDAK memiliki node berikutnya.
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_node 01_node_dan_sambungan.c
 * Run   : ./01_node
 * Sanit : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_node_san 01_node_dan_sambungan.c
 */
#include <stdio.h>
#include <stdlib.h>

/* Struct yang menyimpan pointer ke struct bertipe sama.
 * Ini disebut self-referential struct, dan inilah inti linked list. */
typedef struct Node {
    int             value;
    struct Node    *next;   /* alamat node berikutnya, atau NULL bila terakhir */
} Node;

static void cetak_node(const char *label, const Node *n)
{
    if (n == NULL) {
        printf("%-12s : (node NULL, tidak ada yang bisa dicetak)\n", label);
        return;
    }
    printf("%-12s : alamat=%p value=%d next=%p\n",
           label, (void *)n, n->value, (void *)n->next);
}

int main(void)
{
    /* --- Satu node ------------------------------------------------------ */
    Node *a = malloc(sizeof *a);
    if (a == NULL) {                       /* malloc boleh gagal; selalu cek  */
        fprintf(stderr, "malloc gagal untuk node a\n");
        return 1;
    }
    a->value = 10;
    a->next  = NULL;                       /* invariant: ekor selalu NULL     */
    cetak_node("node a", a);

    /* --- Tiga node yang disambung --------------------------------------- */
    Node *b = malloc(sizeof *b);
    Node *c = malloc(sizeof *c);
    if (b == NULL || c == NULL) {
        /* Jalur gagal: node yang berhasil dibuat tetap harus dilepas.
         * free(NULL) tidak melakukan apa-apa (POSIX), jadi aman dipanggil
         * untuk pointer yang belum pernah dialokasi. */
        fprintf(stderr, "malloc gagal untuk node b atau c\n");
        free(b);
        free(c);
        free(a);
        return 1;
    }

    b->value = 20;
    c->value = 30;

    /* Menyambung: a -> b -> c -> NULL */
    a->next = b;
    b->next = c;
    c->next = NULL;

    cetak_node("node a", a);
    cetak_node("node b", b);
    cetak_node("node c", c);

    /* Menelusuri (traversal): mulai dari kepala, berhenti di NULL.
     * Perhatikan bahwa kita tidak pernah men-dereference NULL. */
    printf("\npenelusuran dari a:\n");
    for (const Node *p = a; p != NULL; p = p->next) {
        printf("  %p -> value=%d -> next=%p\n", (void *)p, p->value, (void *)p->next);
    }

    /* --- Melepas --------------------------------------------------------- */
    /* Urutan pembebasan bebas, tetapi setelah free() pointer menjadi
     * dangling: nilainya masih berisi alamat lama, objeknya sudah tidak ada. */
    free(a);
    free(b);
    free(c);

    printf("\nketiga node sudah dibebaskan (3 kali free, masing-masing sekali)\n");
    printf("sizeof(Node) = %zu byte  (int + padding + pointer)\n", sizeof(Node));
    return 0;
}
