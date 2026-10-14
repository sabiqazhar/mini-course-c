/* Minggu 9 — Lab 3: jalur gagal dan pembersihan terpusat (centralized cleanup)
 *
 * Tujuan: melihat bahwa kode yang "biasanya berhasil" bukanlah kode yang sulit.
 * Yang sulit adalah kode yang harus tetap benar saat salah satu langkahnya gagal.
 *
 * Pola yang dipakai di sini: satu titik keluar (single exit) dengan label
 * "bersih". Semua resource dibebaskan di satu tempat, bukan di banyak cabang.
 * Alasannya sederhana: kalau ada lima jalur gagal, lima salinan kode cleanup
 * pasti ada yang lupa satu baris.
 *
 * Kontrak ownership di file ini:
 *   - create_daftar() mengembalikan list yang SUDAH JADI atau error.
 *   - Bila ia mengembalikan error, pemanggil tidak memiliki apa pun:
 *     semua node sudah dibersihkan oleh fungsi itu sendiri.
 *   - Bila ia mengembalikan LIST_OK, PEMANGGIL memiliki seluruh list dan
 *     wajib memanggil list_destroy().
 *
 * Build : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_cleanup 03_cleanup_terpusat.c
 * Run   : ./03_cleanup
 * Sanit : gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 03_cleanup_san 03_cleanup_terpusat.c
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int          value;
    struct Node *next;
} Node;

typedef struct List {
    Node   *head;
    size_t  count;
} List;

typedef enum { LIST_OK = 0, LIST_ERR_ALLOC } ListStatus;

/* ---- Alokator uji yang bisa disetel untuk gagal pada alokasi ke-N ---------
 * Ini bukan fitur produksi. Tujuannya supaya jalur gagal bisa dijalankan
 * hari ini juga, tanpa harus menghabiskan memory komputer. */
static int  hitung_alokasi      = 0;
static int  gagal_pada_ke       = 0;   /* 0 = jangan pernah gagal */

static void *alokasi_terkontrol(size_t n)
{
    hitung_alokasi++;
    if (gagal_pada_ke != 0 && hitung_alokasi == gagal_pada_ke) {
        return NULL;
    }
    return malloc(n);
}

/* ---- Operasi list (versi ringkas) --------------------------------------- */
static void list_init(List *list)
{
    list->head  = NULL;
    list->count = 0;
}

static void list_destroy(List *list)
{
    Node *p = list->head;
    while (p != NULL) {
        Node *berikut = p->next;
        free(p);
        p = berikut;
    }
    list->head  = NULL;
    list->count = 0;
}

static ListStatus list_push_front(List *list, int value)
{
    Node *node = alokasi_terkontrol(sizeof *node);
    if (node == NULL) {
        return LIST_ERR_ALLOC;      /* list tidak disentuh                */
    }
    node->value = value;
    node->next  = list->head;
    list->head  = node;
    list->count++;
    return LIST_OK;
}

static void list_cetak(const List *list)
{
    printf("    isi: ");
    for (const Node *p = list->head; p != NULL; p = p->next) {
        printf("%d ", p->value);
    }
    printf("(count = %zu)\n", list->count);
}

/* ---- Fungsi yang membangun list dari array, dengan cleanup terpusat ------
 * Kontrak:
 *   - Sukses: *out berisi list yang valid; PEMANGGIL memilikinya.
 *   - Gagal : *out dikosongkan; pemanggil tidak memiliki apa pun.
 * Tidak ada jalur keluar yang membiarkan list setengah jadi di tangan pemanggil. */
static ListStatus buat_daftar(const int *nilai, size_t n, List *out)
{
    ListStatus status = LIST_OK;

    list_init(out);

    for (size_t i = 0; i < n; i++) {
        if (list_push_front(out, nilai[i]) != LIST_OK) {
            status = LIST_ERR_ALLOC;
            goto bersih;            /* satu jalur keluar untuk semua kegagalan */
        }
    }
    return LIST_OK;

bersih:
    /* Pembersihan terpusat: berjalan baik untuk sukses maupun gagal. */
    printf("    [cleanup] melepas %zu node yang sudah terlanjur dibuat\n", out->count);
    list_destroy(out);
    return status;
}

int main(void)
{
    int data[] = { 5, 6, 7, 8 };
    List daftar;

    printf("=== 1. Semua alokasi berhasil ===\n");
    gagal_pada_ke  = 0;
    hitung_alokasi = 0;
    ListStatus s = buat_daftar(data, sizeof data / sizeof data[0], &daftar);
    printf("    hasil: %s\n", s == LIST_OK ? "OK" : "GAGAL");
    list_cetak(&daftar);
    list_destroy(&daftar);              /* pemanggil memiliki list -> wajib destroy */
    printf("    setelah destroy: ");
    list_cetak(&daftar);

    printf("\n=== 2. Alokasi ke-3 gagal (separuh jalan) ===\n");
    gagal_pada_ke  = 3;
    hitung_alokasi = 0;
    s = buat_daftar(data, sizeof data / sizeof data[0], &daftar);
    printf("    hasil: %s\n", s == LIST_OK ? "OK" : "GAGAL ALOKASI");
    list_cetak(&daftar);                /* harus kosong: sudah dibersihkan     */
    printf("    pemanggil TIDAK perlu melepas apa pun, dan tidak boleh memakai\n");
    printf("    isi list karena pembuatan gagal.\n");

    /* Destroy tetap aman dipanggil di sini (list sudah kosong).
     * Ini contoh mengapa destroy harus idempoten secara aman. */
    list_destroy(&daftar);

    printf("\n=== 3. Pointer pinjaman menjadi basi setelah dihapus ===\n");
    gagal_pada_ke  = 0;
    hitung_alokasi = 0;
    (void)buat_daftar(data, 2, &daftar);   /* isi: 6 5 */

    Node *pinjam = daftar.head;            /* PINJAMAN, bukan milik kita       */
    if (pinjam != NULL) {
        printf("    pinjam node dengan value=%d di alamat %p\n",
               pinjam->value, (void *)pinjam);
    }

    /* Menghapus node yang sedang kita pinjam membuat pinjaman itu basi. */
    Node *berikutnya = pinjam->next;       /* simpan dulu kalau masih perlu    */
    free(pinjam == daftar.head ? daftar.head : NULL);  /* hati-hati: ini contoh */
    daftar.head = berikutnya;              /* list sudah disambung ulang        */
    daftar.count--;
    pinjam = NULL;                         /* tidak ada lagi pointer basi       */
    printf("    node sudah dilepas; pointer pinjaman dikosongkan: %s\n",
           pinjam == NULL ? "NULL" : "masih terisi (bahaya)");
    list_cetak(&daftar);

    list_destroy(&daftar);
    printf("\nselesai\n");
    return 0;
}
