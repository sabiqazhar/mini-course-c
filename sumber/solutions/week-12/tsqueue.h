/* Minggu 12 — Antrean (queue) berbatas yang aman untuk multithread.
 *
 * API ini dirancang sebagai proyek akhir (capstone) yang menyatukan semua materi:
 *   - struct & ring buffer (minggu 2, 9)
 *   - heap allocation + kegagalan alokasi (minggu 3, 4)
 *   - mutex & condition variable, tanpa busy-wait (minggu 8)
 *   - kontrak ownership & invarian (minggu 9)
 *
 * Invarian (selalu benar saat mutex terkunci):
 *   0 <= count <= cap
 *   head/tail bergerak melingkar modulo cap
 *   0 <= head < cap, 0 <= tail < cap
 *
 * Kontrak ownership:
 *   - Queue memiliki salinan nilai int di buffer internalnya (copy by value).
 *     Pemanggil tetap pemilik int yang dikirimkan; kita tidak menyimpan pointer-nya.
 *   - Pemanggil wajib memanggil tsqueue_destroy SETELAH semua thread di-join.
 *   - tsqueue_shutdown aman dipanggil berkali-kali; ia hanya membangunkan thread.
 *
 * Build (lihat Makefile): gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread
 */
#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <stddef.h>
#include <pthread.h>

typedef struct {
    int             *buf;        /* buffer internal, milik queue (dialokasikan di init) */
    size_t          cap;         /* kapasitas: banyak slot */
    size_t          head;        /* indeks dequeue berikutnya */
    size_t          tail;        /* indeks enqueue berikutnya */
    size_t          count;       /* banyak item valid saat ini */
    pthread_mutex_t mtx;         /* SATU mutex melindungi semua field di atas */
    pthread_cond_t  not_empty;   /* sinyal ke konsumen: ada item baru */
    pthread_cond_t  not_full;    /* sinyal ke produser: ada slot kosong */
    int             shutdown;    /* poison-pill flag; 1 => stop terima item baru */
} TSQueue;

/* Inisialisasi queue dengan kapasitas cap.
 * Return 0 sukses; -1 kalau cap==0, overflow, atau gagal alokasi/init primitif.
 * Kontrak: kalau return -1, TIDAK ADA resource yang bocor (cleanup dilakukan). */
int tsqueue_init(TSQueue *q, size_t cap);

/* Enqueue memblokir sampai ada slot kosong ATAU shutdown dipanggil.
 * Return 0 sukses; -1 kalau shutdown terjadi sebelum ada slot (item tdk ditambah). */
int tsqueue_enqueue(TSQueue *q, int v);

/* Dequeue memblokir sampai ada item ATAU (shutdown DAN queue kosong).
 * Return 0 sukses (*out diisi); -1 kalau shutdown & kosong (konsumen harus keluar). */
int tsqueue_dequeue(TSQueue *q, int *out);

/* Publikasikan shutdown: set flag dan broadcast kedua kondisi supaya semua thread
 * yang menunggu terbangun dan keluar. Aman dipanggil berkali-kali. */
void tsqueue_shutdown(TSQueue *q);

/* Hancurkan queue. HARUS dipanggil SETELAH semua thread di-join.
 * Membebaskan buffer dan mendestroy mutex + kedua kondisi. */
void tsqueue_destroy(TSQueue *q);

#endif /* TSQUEUE_H */
