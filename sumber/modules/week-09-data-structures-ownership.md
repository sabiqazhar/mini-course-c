# Minggu 9 — Data Structure, Ownership, dan Resource-Safe API

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** bagaimana pointer digunakan untuk membangun struktur data yang hidup dinamis?

---

## 1. Mengapa ini penting

Minggu 3–4 memberimu pointer ke objek tunggal dan array dinamis. Dunia nyata butuh struktur yang saling
menunjuk: linked list, tree, buffer melingkar. Semua itu hanyalah pointer + kontrak. Kunci keamanannya
bukan trik, melainkan **ownership**: secara eksplisit mendokumentasikan siapa yang bertanggung jawab
memanggil `free`. Tanpa kontrak, node bocor atau di-free dua kali. Minggu ini melatihmu menulis API yang
menjaga invariant.

## 2. Learning objectives

Setelah minggu 9, kamu dapat:

1. Mengimplementasikan linked list (`push_front`, `find`, `remove`, `destroy`) dengan ownership jelas.
2. Menjelaskan kontrak API: siapa yang memiliki node, apa yang "dipinjam", kapan objek dibebaskan.
3. Menangani alokasi gagal saat menyisipkan/menghapus tanpa merusak invariant.
4. Menggunakan `NULL`, sentinel, dan invariant struktur untuk menjaga kebenaran.
5. Membangun ring buffer sebagai dasar queue, lengkap dengan invariant dan kontrak.
6. Melakukan cleanup terpusat yang tidak melepas node dua kali.

## 3. Prasyarat

`malloc`/`free`, ownership, dan dangling/use-after-free dari minggu 3
([`week-03-heap-malloc-free.md`](week-03-heap-malloc-free.md)). Pointer dan struct dari minggu 2.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Kamu butuh koleksi yang bisa tumbuh di mana saja tanpa `realloc` seluruh array. Linked list menjawabnya:
tiap elemen (node) dialokasikan sendiri dan menunjuk ke tetangganya.

### Analogi: rantai kunci

Setiap node seperti gantungan kunci yang menyimpan secuil data dan selembar kertas berisi "kunci
berikutnya ada di laci nomor X". Untuk melihat semua data, kamu mulai dari gantungan pertama lalu
ikutinya berantai. Menghapus satu gantungan = ambil kertasnya, sambungkan gantungan sebelumnya langsung
ke gantungan sesudahnya, lalu lepaskan gantungan itu.

### Batas analogi

Di dunia nyata, "kertas berisi alamat" adalah pointer yang bisa menunjuk ke mana saja, termasuk alamat
yang sudah dilepas (dangling). Dan tidak ada sistem yang otomatis tahu "siapa pemilik gantungan" — itu
harus kamu tulis sebagai kontrak. Analogi tidak menggantikan disiplin ownership.

### Penjelasan teknis

Linked list adalah rantai node; tiap node punya `data` dan pointer `next` ke node berikutnya. Node
terakhir `next == NULL` (sentinel akhir). Operasi inti:

- `push_front(head, v)`: alokasi node, isi `v`, arahkan `next` ke `head` lama, kembalikan node baru
  sebagai `head`. Bila alokasi gagal, **kembalikan head lama** (jangan bocor, jangan rusak list).
- `find(head, v)`: jelajahi rantai sampai `NULL`; kembalikan pointer node atau `NULL`.
- `remove_val(head, v)`: jaga rantai dengan menyambung `prev->next = cur->next`, lalu `free(cur)`.
  Kembalikan head baru (bisa berubah bila node pertama yang dihapus).
- `destroy(head)`: lepas setiap node tepat satu kali, maju sampai `NULL`.

**Ownership** di sini: fungsi yang mengalokasikan node (push) mentransfer kepemilikan ke list; `destroy`
mengakhiri kepemilikan dengan `free`. `find` hanya **meminjam** (mengembalikan pointer tanpa transfer
ownership) — pemanggil tidak boleh `free` hasil `find`. Kontrak ini harus tertulis di komentar.

### Diagram

```
list:  [3|*] -> [1|*] -> [2|NULL]
        ^head

push_front(list, 5):
       [5|*] -> [3|*] -> [1|*] -> [2|NULL]
        ^head baru
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-09/01_linked_list.c`](../labs/week-09/01_linked_list.c)

```c
static Node *push_front(Node *head, int v) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) return head;     /* gagal: list tetap utuh, tidak bocor */
    n->val = v;
    n->next = head;
    return n;                        /* ownership node pindah ke list */
}

static Node *remove_val(Node *head, int v) {
    Node *prev = NULL, *cur = head;
    while (cur != NULL) {
        if (cur->val == v) {
            Node *next = cur->next;
            free(cur);               /* bebaskan tepat satu kali */
            if (prev) prev->next = next;
            else     head = next;     /* hapus head: head bergeser */
            return head;
        }
        prev = cur; cur = cur->next;
    }
    return head;                      /* tidak ditemukan: tidak ada free */
}
```

Poin penting: `remove_val` menyambung rantai **sebelum** `free(cur)`, sehingga tidak ada jendela di mana
list terputus sementara thread/cek lain mengaksesnya. Dan `free` hanya sekali per node.

### Cara kompilasi dan menjalankan

```bash
cd labs/week-09
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 01_linked_list 01_linked_list.c
./01_linked_list
```

### Hasil yang diharapkan

```
sebelum      : 1 2 3
cari 2       : ditemukan
setelah hapus: 1 3
hapus 99     : 1 3
```

`hapus 99` tidak menemukan apa pun, tidak error, tidak bocor. Di bawah ASan, tidak ada laporan leak
maupun double free — tanda ownership dikelola benar.

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — linked list lengkap (15 menit)

Jalankan `01_linked_list` di bawah ASan (perintah di atas). Tambahkan elemen, cari, hapus beberapa, lalu
biarkan `main` berakhir (memanggil `destroy`). ASan harus melaporkan "0 leaks". Itu pembuktian ownership
terjaga tanpa perlu membaca tiap `free` satu per satu.

### Eksperimen 2 — ring buffer (15 menit)

File [`02_ring_buffer.c`](../labs/week-09/02_ring_buffer.c) membangun queue melingkar tanpa thread.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o 02_ring_buffer 02_ring_buffer.c
./02_ring_buffer
```

Hasil di mesin penulis:

```
enqueue 1 -> 0
enqueue 2 -> 0
enqueue 3 -> 0
enqueue 4 -> 0
enqueue 5 (penuh?) -> -1
dequeue -> 1 (count=3)
dequeue -> 2 (count=2)
dequeue -> 3 (count=1)
dequeue -> 4 (count=0)
```

Perhatikan invariant: `enqueue` mengembalikan `-1` saat penuh, `dequeue` `-1` saat kosong. Head/tail
bergerak melingkar lewat `(idx + 1) % cap`. Ini dasar persis untuk bounded queue thread-safe minggu 12.

### Eksperimen 3 — ubah agar gagal ditangani (10 menit)

Modifikasi `push_front` agar mensimulasikan kegagalan alokasi (misal `return head` tanpa malloc saat
sebuah flag menyala). Pastikan list tetap bisa di-`destroy` tanpa double free. Ini melatih "jalur error
tetap aman".

---

## 7. Kesalahan umum

- **Memutus rantai lalu `free` tanpa menyambung `prev->next`.** Menghasilkan node terisolasi (bocor) atau
  akses ke node sudah bebas.
- **`free` hasil `find`.** `find` meminjam, bukan transfer ownership. Memanggil `free` di situ adalah
  double-free bila `destroy` juga membebaskannya.
- **Menghapus node pertama lupa memperbarui `head`.** Menyisakan list "kepala palsu".
- **Lupa `NULL` sentinel.** Loop traversal tanpa cek `NULL` membaca di luar batas.
- **Menganggap node masih valid setelah `free`.** Use-after-free.

---

## 8. Latihan

### Mudah
Tulis `int list_len(Node *head)` yang menghitung panjang list (jangan memodifikasi list).

### Menengah
Tambahkan `Node *pop_front(Node *head)` yang mengembalikan nilai lewat pointer dan head baru, menangani
list kosong (kembalikan error). Pastikan tidak double free saat dikombinasikan dengan `destroy`.

### Tantangan
Tulis `Node *reverse(Node *head)` yang membalik list dengan mengubah pointer (tanpa alokasi baru), lalu
`destroy`. Gambar state sebelum/sesudah tiap langkah. Petunjuk bertahap: (1) pegang `prev=NULL`, `cur=head`;
(2) tiap langkah simpan `next=cur->next`, set `cur->next=prev`, geser; (3) `head` baru = node terakhir.

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan kontrak ownership `push_front`/`find`/`remove`/`destroy`.
- [ ] Saya tahu `find` meminjam, bukan mentransfer ownership.
- [ ] Saya menangani alokasi gagal tanpa merusak list.
- [ ] Saya tidak melepas node dua kali dan tidak meninggalkan akses ke node bebas.
- [ ] Saya bisa menjelaskan invariant ring buffer dan arti `NULL` sentinel.

---

## 10. Ringkasan

Struktur data dinamis hanyalah pointer + kontrak. Linked list mengalokasikan node sendiri dan merantainya;
ownership ditransfer ke list saat push dan diakhiri saat destroy. `find` meminjam (jangan free hasilnya).
Jaga rantai saat menghapus, gunakan `NULL` sebagai sentinel, dan pastikan cleanup tidak double free. Ring
buffer adalah dasar queue yang akan kita jadikan thread-safe di minggu 12.

**Glosarium mini:** linked list, node, sentinel, ownership, invariant, ring buffer. Lengkap di
[`glossary.md`](../glossary.md).

## 11. Referensi

- [malloc(3) — Linux man-pages](https://man7.org/linux/man-pages/man3/malloc.3.html) — alokasi node.
- [The Linux Programming Interface — TLPI](https://man7.org/tlpi/) — ch. 7 (Memory Allocation), ch. 44 (Pipes, untuk pola buffer).
- [Beej's Guide to C Programming](https://beej.us/guide/bgc/) — intuisi pointer dan struct.

## 12. Jembatan ke minggu 10

Kamu membangun struktur data tanpa pernah bertanya: **siapa yang sebenarnya memenuhi permintaan `malloc`?**
Di balik `malloc` ada allocator. Minggu 10 membukanya: blok bebas/terpakai, metadata, alignment, splitting,
coalescing, dan fragmentasi — serta sebuah allocator mainan yang kamu tulis sendiri. Lanjut:
[`week-10-memory-allocator.md`](week-10-memory-allocator.md)
