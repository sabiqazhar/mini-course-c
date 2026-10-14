/* Minggu 9 — Lab 2: API linked list dengan kontrak ownership yang tertulis
 *
 * Tujuan: membangun linked list kecil yang setiap operasinya menjaga
 * invariant, dan yang jalur gagalnya (alokasi gagal, nilai tidak ditemukan,
 * list kosong) bisa dijalankan dan diamati.
 *
 * ============ KONTRAK (baca ini sebelum memakai fungsi apa pun) ============
 *
 * 1. List memiliki setiap node yang ada di dalamnya.
 *    Pemanggil TIDAK pernah memiliki node di dalam list.
 *
 * 2. list_push_front(list, v)
 *    - Menyalin nilai v ke dalam node baru milik list.
 *    - Mengembalikan LIST_OK bila berhasil, LIST_ERR_ALLOC bila malloc gagal.
 *    - Bila gagal, list TIDAK berubah sama sekali (invariant tetap terjaga).
 *
 * 3. list_find(list, v)
 *    - Mengembalikan pointer ke node yang dimiliki list, atau NULL bila tidak ada.
 *    - Ini BORROWED pointer: PEMINJAMAN, bukan kepemilikan.
 *    - Pemanggil DILARANG memanggil free() pada hasilnya.
 *    - Hasilnya bisa menjadi tidak valid setelah operasi penghapusan atau
 *      list_destroy() berikutnya.
 *
 * 4. list_remove(list, v)
 *    - Menghapus node pertama dengan nilai v dan melepasnya (free).
 *    - LIST_OK bila dihapus, LIST_NOT_FOUND bila tidak ada nilainya,
 *      LIST_EMPTY bila list kosong.
 *
 * 5. list_destroy(list)
 *    - Melepas SEMUA node, lalu mengosongkan struct list.
 *    - Aman dipanggil pada list kosong (tidak melakukan apa-apa).
 *    - Aman dipanggil dua kali berturut-turut, karena setelah selesai
 *      list->head bernilai NULL dan count bernilai 0.
 *    - Setelah destroy, tidak ada pointer internal yang masih dipakai.
 *
 * INVARIANT List (harus benar sebelum dan sesudah setiap operasi):
 *    I1. list->head == NULL  jika dan hanya jika  list->count == 0.
 *    I2. Menelusuri next dari head akan berakhir di NULL (tidak ada siklus).
 *    I3. Banyaknya node yang dikunjungi penelusuran == list->count.
 *    I4. Setiap node bisa dijangkau dari head (tidak ada node yatim).
 * =========================================================================
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_list 02_linked_list_api.c
 * Run   : ./02_list
 * Sanit : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 02_list_san 02_linked_list_api.c
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int          value;
    struct Node *next;
} Node;

typedef struct List {
    Node   *head;    /* NULL berarti list kosong                     */
    size_t  count;   /* banyak node; harus konsisten dengan head      */
} List;

typedef enum {
    LIST_OK = 0,        /* operasi berhasil                            */
    LIST_ERR_ALLOC,     /* malloc gagal; list tidak berubah            */
    LIST_NOT_FOUND,     /* nilai tidak ada di list                     */
    LIST_EMPTY          /* list kosong; operasi tidak berlaku          */
} ListStatus;

static const char *status_teks(ListStatus s)
{
    switch (s) {
        case LIST_OK:        return "OK";
        case LIST_ERR_ALLOC: return "GAGAL ALOKASI (list tidak berubah)";
        case LIST_NOT_FOUND: return "NILAI TIDAK DITEMUKAN";
        case LIST_EMPTY:     return "LIST KOSONG";
    }
    return "?";
}

static void list_init(List *list)
{
    list->head  = NULL;
    list->count = 0;
}

/* Mencetak bentuk list sebagai diagram ASCII. Dipakai untuk melihat struktur
 * SEBELUM dan SESUDAH setiap operasi. */
static void list_gambar(const char *judul, const List *list)
{
    printf("  %s\n", judul);
    if (list->head == NULL) {
        printf("    head -> [ ]   (kosong, count = %zu)\n", list->count);
        return;
    }
    printf("    head -> ");
    for (const Node *p = list->head; p != NULL; p = p->next) {
        printf("[ %d |*]--", p->value);
        if (p->next == NULL) {
            printf("> NULL");
        }
    }
    printf("   (count = %zu)\n", list->count);
}

/* Memeriksa invariant I1..I4. Dipanggil setelah setiap operasi di lab ini.
 * Fungsi ini sengaja memakai batas langkah (limit) agar list yang rusak
 * karena siklus tidak membuat program menggantung. */
static int list_periksa_invariant(const List *list)
{
    size_t ditemukan = 0;
    size_t langkah   = 0;

    if ((list->head == NULL) != (list->count == 0)) {
        printf("    INVARIANT I1 RUSAK: head=%s tetapi count=%zu\n",
               list->head ? "tidak NULL" : "NULL", list->count);
        return 0;
    }
    for (const Node *p = list->head; p != NULL; p = p->next) {
        ditemukan++;
        if (++langkah > 1000) {
            printf("    INVARIANT I2 RUSAK: penelusuran tidak berakhir (siklus?)\n");
            return 0;
        }
    }
    if (ditemukan != list->count) {
        printf("    INVARIANT I3 RUSAK: count=%zu tetapi node yang dihitung=%zu\n",
               list->count, ditemukan);
        return 0;
    }
    printf("    invariant OK (I1,I2,I3 terpenuhi; count=%zu)\n", list->count);
    return 1;
}

/* Versi internal yang menerima fungsi alokasi, supaya jalur gagal bisa diuji
 * secara deterministik di lab ini tanpa harus membuat sistem kehabisan memory. */
static ListStatus list_push_front_alloc(List *list, int value,
                                        void *(*alloc)(size_t))
{
    Node *node = alloc(sizeof *node);
    if (node == NULL) {
        return LIST_ERR_ALLOC;   /* list belum disentuh: invariant tetap aman */
    }
    node->value = value;
    node->next  = list->head;    /* node baru menunjuk ke kepala lama        */
    list->head  = node;          /* kepala pindah ke node baru               */
    list->count++;
    return LIST_OK;
}

static ListStatus list_push_front(List *list, int value)
{
    return list_push_front_alloc(list, value, malloc);
}

/* Mengembalikan BORROWED pointer. Bukan milik pemanggil. */
static Node *list_find(const List *list, int value)
{
    for (Node *p = list->head; p != NULL; p = p->next) {
        if (p->value == value) {
            return p;
        }
    }
    return NULL;   /* NULL berarti "tidak ada", BUKAN kegagalan alokasi */
}

static ListStatus list_remove(List *list, int value)
{
    if (list->head == NULL) {
        return LIST_EMPTY;                 /* tidak ada yang bisa dihapus */
    }

    Node *sebelum = NULL;
    Node *p       = list->head;
    while (p != NULL && p->value != value) {
        sebelum = p;
        p       = p->next;
    }
    if (p == NULL) {
        return LIST_NOT_FOUND;             /* nilai tidak ada di list     */
    }

    if (sebelum == NULL) {
        list->head = p->next;              /* menghapus kepala            */
    } else {
        sebelum->next = p->next;           /* menjembatani node yang dihapus */
    }
    list->count--;

    /* Melepas node. Kita simpan dulu alamat berikutnya (sudah dilakukan lewat
     * p->next di atas), jadi tidak ada pointer internal yang masih menunjuk
     * ke node yang sudah bebas. */
    p->next = NULL;
    free(p);
    return LIST_OK;
}

/* Melepas semua node. Aman untuk list kosong dan aman dipanggil dua kali. */
static void list_destroy(List *list)
{
    Node *p = list->head;
    while (p != NULL) {
        Node *berikut = p->next;   /* simpan dulu, karena p akan dilepas */
        free(p);
        p = berikut;
    }
    list->head  = NULL;            /* tidak ada pointer internal tersisa */
    list->count = 0;
}

/* Alokator palsu yang selalu gagal: dipakai untuk menguji jalur LIST_ERR_ALLOC. */
static void *alokasi_gagal(size_t n)
{
    (void)n;
    return NULL;
}

int main(void)
{
    List daftar;
    list_init(&daftar);

    printf("=== 1. List kosong ===\n");
    list_gambar("keadaan awal:", &daftar);
    list_periksa_invariant(&daftar);

    printf("\n  mencari 7 di list kosong  -> %s\n",
           status_teks(list_find(&daftar, 7) == NULL ? LIST_NOT_FOUND : LIST_OK));
    printf("  menghapus 7 di list kosong-> %s\n",
           status_teks(list_remove(&daftar, 7)));

    printf("\n=== 2. push_front 30, 20, 10 ===\n");
    int v[] = { 30, 20, 10 };
    for (size_t i = 0; i < sizeof v / sizeof v[0]; i++) {
        ListStatus s = list_push_front(&daftar, v[i]);
        printf("  push_front(%d) -> %s\n", v[i], status_teks(s));
        list_gambar("  sesudah:", &daftar);
        list_periksa_invariant(&daftar);
    }

    printf("\n=== 3. find: ketemu dan tidak ketemu ===\n");
    Node *ketemu = list_find(&daftar, 20);
    if (ketemu != NULL) {
        printf("  find(20) -> ketemu, value=%d, alamat=%p (DIPINJAM, jangan di-free)\n",
               ketemu->value, (void *)ketemu);
    }
    Node *tidak = list_find(&daftar, 99);
    printf("  find(99) -> %s\n", tidak == NULL ? "NULL (tidak ditemukan)" : "ketemu");

    printf("\n=== 4. remove: nilai ada, nilai tidak ada ===\n");
    list_gambar("sebelum remove(20):", &daftar);
    printf("  remove(20) -> %s\n", status_teks(list_remove(&daftar, 20)));
    list_gambar("sesudah remove(20):", &daftar);
    list_periksa_invariant(&daftar);

    printf("  remove(99) -> %s\n", status_teks(list_remove(&daftar, 99)));
    list_gambar("sesudah remove(99):", &daftar);
    list_periksa_invariant(&daftar);

    printf("\n=== 5. jalur gagal alokasi ===\n");
    list_gambar("sebelum push yang gagal:", &daftar);
    ListStatus s = list_push_front_alloc(&daftar, 77, alokasi_gagal);
    printf("  push dengan alokator yang selalu gagal -> %s\n", status_teks(s));
    list_gambar("sesudah push yang gagal:", &daftar);
    list_periksa_invariant(&daftar);

    printf("\n=== 6. destroy, lalu destroy lagi ===\n");
    list_gambar("sebelum destroy:", &daftar);
    list_destroy(&daftar);
    list_gambar("sesudah destroy:", &daftar);
    list_periksa_invariant(&daftar);

    printf("  destroy kedua kalinya (harus aman)...\n");
    list_destroy(&daftar);
    list_gambar("sesudah destroy kedua:", &daftar);
    list_periksa_invariant(&daftar);

    printf("\nselesai: tidak ada node yang tersisa\n");
    return 0;
}
