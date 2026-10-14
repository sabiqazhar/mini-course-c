/* Minggu 12 — test dasar (SINGLE-thread) untuk queue.
 *
 * Tujuannya: verifikasi FIFO dan perilaku ring buffer (tail memutar) TANPA thread.
 * Di single-thread, JANGAN panggil dequeue sampai queue kosong, karena dequeue akan
 * memblokir selamanya (tidak ada konsumen/producer lain yang mengirim sinyal). Karena
 * itu kita "drain" sejumlah item yang kita tahu persis jumlahnya.
 *
 * Catatan penting (pelajaran minggu 8 & 12): jangan letakkan pemanggilan fungsi
 * yang punya efek samping (di sini tsqueue_enqueue) sebagai argumen printf bersama
 * pembacaan field struct yang diubahnya — urutan evaluasi argumen di C TIDAK
 * ditentukan. Pisahkan ke pernyataan sendiri, seperti di bawah.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o test_basic tsqueue.c test_basic.c
 * Run  : ./test_basic
 */
#include "tsqueue.h"

#include <stdio.h>

int main(void)
{
    TSQueue q;
    if (tsqueue_init(&q, 3) != 0) {
        fprintf(stderr, "init gagal\n");
        return 1;
    }

    /* enqueue 3 item (kapasitas 3) — tiap panggilan dipisah dari printf */
    int rc;
    rc = tsqueue_enqueue(&q, 10);
    printf("enqueue 10 -> %d  (count=%zu, head=%zu, tail=%zu)\n", rc, q.count, q.head, q.tail);
    rc = tsqueue_enqueue(&q, 20);
    printf("enqueue 20 -> %d  (count=%zu, head=%zu, tail=%zu)\n", rc, q.count, q.head, q.tail);
    rc = tsqueue_enqueue(&q, 30);
    printf("enqueue 30 -> %d  (count=%zu, head=%zu, tail=%zu)\n", rc, q.count, q.head, q.tail);
    /* sekarang penuh: count==3==cap, tail sudah memutar kembali ke 0 */

    /* dequeue 1 (head geser, buka satu slot) */
    int v;
    rc = tsqueue_dequeue(&q, &v);
    printf("\ndequeue -> %d  (rc=%d, count=%zu, head=%zu, tail=%zu)\n", v, rc, q.count, q.head, q.tail);

    /* enqueue 40: harus masuk ke slot yang tadi kosong (tail wrap) */
    rc = tsqueue_enqueue(&q, 40);
    printf("enqueue 40 -> %d  (count=%zu, head=%zu, tail=%zu)\n", rc, q.count, q.head, q.tail);
    /* peek "under the hood" — HANYA untuk belajar; kode nyata pakai API, bukan buka buf */
    printf("isi buffer mentah (bukan urutan FIFO): buf[0]=%d buf[1]=%d buf[2]=%d\n",
           q.buf[0], q.buf[1], q.buf[2]);

    /* drain sejumlah item yang kita tahu persis ada (3: 20, 30, 40) — jangan sampai kosong */
    printf("\ndrain sisa (harus FIFO: 20, 30, 40):\n");
    size_t sisa = q.count;
    for (size_t i = 0; i < sisa; i++) {
        tsqueue_dequeue(&q, &v);
        printf("dequeue -> %d\n", v);
    }

    tsqueue_destroy(&q);
    printf("\nselesai. tidak ada leak (cek dengan -fsanitize=address).\n");
    return 0;
}
