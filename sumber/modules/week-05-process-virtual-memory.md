# Minggu 5 — Process, Virtual Address Space, dan Virtual Memory

**Estimasi durasi:** 5–7 jam
**Pertanyaan utama minggu ini:** apakah alamat yang dicetak program merupakan alamat fisik RAM?

---

## 1. Mengapa ini penting

Minggu 1–4 kamu memakai alamat tanpa pernah bertanya apa itu. Sekarang waktunya. Memahami bahwa
alamat dalam program adalah **virtual** — bukan alamat fisik — adalah kunci untuk mengerti `mmap`,
`fork`, *shared library*, dan mengapa dua process bisa mencetak alamat yang sama. Tanpa dasar ini,
minggu 6 (system call) dan minggu 12 (proyek akhir) akan terasa seperti sihir.

## 2. Learning objectives

Setelah minggu 5, kamu dapat:

1. Menjelaskan beda **program** (file di disk) dan **process** (instansi hidup di memory).
2. Menjelaskan bahwa setiap process punya virtual address space sendiri, dan alamat yang dicetak adalah virtual.
3. Menjelaskan secara konseptual page, page table, demand paging, dan page fault.
4. Menjelaskan mengapa virtual address yang sama di dua process tidak berarti memory fisik yang sama.
5. Menjelaskan ASLR dan mengapa alamat berubah tiap run.
6. Membaca dan menafsirkan `/proc/self/maps` (sebagai observasi khusus Linux).

## 3. Prasyarat

Kamu sudah paham pointer dan heap dari minggu 1–4. Konsep "alamat" minggu 1
([`week-01-memory-and-pointers.md`](week-01-memory-and-pointers.md)) adalah dasarnya.

---

## 4. Dari masalah nyata ke konsep

### Masalahnya

Dua program berbeda, masing-masing mencetak `&x` dan mendapat angka yang sama persis. Apakah mereka
berbagi memory yang sama? Jika tidak, mengapa alamatnya sama?

### Analogi: apartemen dengan nomor pintu sama

Bayangkan dua gedung apartemen berbeda, masing-masing punya unit nomor 101. Penghuni gedung A dan
gedung B sama-sama bilang "saya di 101". Nomor pintunya sama, tetapi kamar fisiknya beda. Virtual
address seperti nomor pintu: setiap process punya "gedung" (ruang alamat) sendiri, dan kernel menerjemahkan
nomor itu ke kamar fisik (RAM) lewat tabel.

### Batas analogi

Di gedung nyata, nomor 101 di gedung A dan B benar-benar tidak berhubungan. Di komputer, penerjemahan
virtual→fisik dilakukan perangkat keras (MMU) lewat **page table** yang dikelola kernel. Dan ada fitur
khusus (shared memory, `mmap` `MAP_SHARED`) yang justru membuat dua process *bisa* berbagi halaman fisik
yang sama — itu pengecualian, bukan aturan.

### Penjelasan teknis

Sebuah **program** adalah berkas eksekusi di disk. Sebuah **process** adalah program yang sedang
berjalan, lengkap dengan ruang alamat, file descriptor, dan register sendiri. Setiap process modern
mendapat **virtual address space** sendiri: rentang alamat 0 sampai 2^48 (atau 2^64) yang dipetakan ke
physical memory lewat page table milik process itu.

**Page** adalah unit pemetaan terkecil, umumnya 4096 byte di Linux x86-64. `sysconf(_SC_PAGE_SIZE)`
memberi ukurannya. **Page table** adalah struktur milik kernel yang menerjemahkan virtual→physical.
Ketika program mengakses alamat yang belum dipetakan, terjadi **page fault**: kernel menanganinya —
menyiapkan halaman (misal dari file lewat mmap, atau kosong lewat demand paging) lalu melanjutkan.
Jika aksesnya tidak sah, process mendapat SIGSEGV.

Semua penjelasan di atas bersifat konseptual; detail page-table walk, TLB, dan minor/major page fault
adalah materi lanjutan. Fokus minggu ini: alamat program itu virtual, dan itu cukup untuk lanjut ke
system call.

### Diagram

```
   Process A (virtual)          MMU / page table          RAM (fisik)
┌─────────────────┐          ┌──────────────┐        ┌──────────────┐
│ 0x1000  [kode]  │────────▶ │ 0x1000 → 0x88A0│──────▶│ 0x88A0  kode  │
│ 0x2000  [heap]  │────────▶ │ 0x2000 → 0x12F0│──────▶│ 0x12F0  heap  │
└─────────────────┘          └──────────────┘        └──────────────┘
   Process B (virtual)
┌─────────────────┐
│ 0x1000  [kode]  │──▶ (page table B sendiri) ──▶ RAM berbeda, meski virtual sama
└─────────────────┘
```

---

## 5. Contoh kode minimal dan penjelasannya

File: [`labs/week-05/01_virtual_alamat.c`](../labs/week-05/01_virtual_alamat.c)

```c
int global_var = 123;
int main(void) {
    int   lokal = 7;
    int  *heap  = malloc(sizeof *heap);
    *heap = 99;
    printf("alamat global : %p\n", (void *)&global_var);
    printf("alamat lokal  : %p\n", (void *)&lokal);
    printf("alamat heap   : %p\n", (void *)heap);
    free(heap);
}
```

Tidak ada yang istimewa secara sintaks — poin minggu ini adalah *memaknai* angka `%p` itu. Ketiga
alamat adalah **virtual**. Jalankan berkali-kali: angkanya akan berubah (ASLR, lihat bawah).

### Cara kompilasi dan menjalankan

```bash
cd labs/week-05
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 01_virtual_alamat 01_virtual_alamat.c
./01_virtual_alamat
```

### Hasil yang diharapkan

```
alamat global : 0x563f0b4b4010
alamat lokal  : 0x7fffe791cd4c
alamat heap   : 0x563f4a7302a0

Jalankan beberapa kali: keempat alamat di atas akan berubah.
Itu karena ASLR. Alamat ini VIRTUAL, bukan alamat fisik RAM.
```

Yang stabil: `lokal` (di stack) biasanya beralamat tinggi mendekati `0x7fff...`, sedangkan `global`
dan `heap` beralamat rendah (`0x55...`/`0x56...`). **Jangan menyimpulkan "wilayah" dari satu run** —
urutannya bisa berbeda, dan alamat selalu acak karena ASLR.

---

## 6. Eksperimen terbimbing

### Eksperimen 1 — membaca peta memory sendiri (15 menit)

File [`02_proc_self_maps.c`](../labs/week-05/02_proc_self_maps.c) membaca `/proc/self/maps`. Ini
adalah **fitur khusus Linux** (bukan standar C/POSIX).

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 02_proc_self_maps 02_proc_self_maps.c
./02_proc_self_maps
```

Cuplikan hasil (alamat bervariasi, struktur serupa):

```
  #  address                   perms offset   dev    inode    pathname
  1  5600318c6000-5600318c7000 r--p 00000000 00:201 10083   .../w5_02
  2  5600318c7000-5600318c8000 r-xp 00001000 00:201 10083   .../w5_02
  5  5600318ca000-5600318cb000 rw-p 00003000 00:201 10083   .../w5_02
  6  560052d1d000-560052d3e000 rw-p 00000000 00:00 0        [heap]
  8  7f7fec8b3000-7f7fec8db000 r--p 00000000 fd:10  62279349 /usr/lib/.../libc.so.6
 12  7f7fecab6000-7f7fecab8000 rw-p 00202000 fd:10  62279349 /usr/lib/.../libc.so.6
```

Kolom: `alamat`, `perms` (r/w/x, s=shared, p=private copy-on-write), `offset`, `dev`, `inode`,
`pathname`. Lima baris pertama adalah programmu sendiri (kode, data, dll). Baris `[heap]` adalah
heap. Baris dengan path `libc.so.6` adalah *shared library*. Path kosong = pemetaan anonim. Format
lengkap ada di [`proc_pid_maps(5)`](https://man7.org/linux/man-pages/man5/proc_pid_maps.5.html)
(dokumentasi kini berada di halaman `proc_pid_maps(5)`, terpisah dari `proc(5)`).

### Eksperimen 2 — dua process, alamat sama, memory beda (15 menit)

File [`03_fork_shared_addr.c`](../labs/week-05/03_fork_shared_addr.c) memakai `fork()` untuk membuat
child, lalu keduanya mencetak alamat global dan heap.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o 03_fork_shared_addr 03_fork_shared_addr.c
./03_fork_shared_addr
```

Hasil di mesin penulis:

```
CHILD : &global=0x55c581eca01c heap=0x55c5992b82a0 *heap=42 global=42
PARENT: &global=0x55c581eca01c heap=0x55c5992b82a0 *heap=0 global=0

Alamat virtual SAMA (copy-on-write), tapi perubahan child
TIDAK terlihat di parent => memory fisiknya BERBEDA.
```

Ini adalah bukti langsung: alamat virtual identik, tetapi child mengubah nilai tanpa memengaruhi
parent. Kernel menyalin halaman saat salah satu menulis (*copy-on-write*), sehingga fisiknya terpisah.
Itulah isolasi antar process.

### Eksperimen 3 — ASLR (5 menit)

Jalankan `./01_virtual_alamat` lima kali. Amati bahwa keempat alamat berubah tiap kali. ASLR
(Address Space Layout Randomization) mengacak tata letak untuk keamanan. Ini sebabnya kamu tidak boleh
menyimpan alamat antar run atau ke process lain.

---

## 7. Kesalahan umum

- **Menganggap alamat = alamat RAM.** Alamat di program adalah virtual; kernel+MMU menerjemahkannya.
- **Menyimpulkan region dari satu eksperimen.** Urutan/alamat bervariasi (ASLR). Lihat banyak run.
- **Mengira alamat sama berarti data sama.** Eksperimen 2 membuktikan sebaliknya.
- **Menganggap `/proc/self/maps` portabel.** Itu Linux-specific; di BSD atau macOS berbeda (atau tiada).

---

## 8. Latihan

### Mudah
Jalankan `01_virtual_alamat` 5 kali, catat alamat `lokal`. Berapa banyak angka yang berbeda? Apa
kesimpulan tentang stack address dan ASLR?

### Menengah
Buka `/proc/self/maps` hasil eksperimen 1. Sebutkan: berapa region milik programmu, mana `[heap]`,
dan mana baris `libc.so.6`. Jelaskan arti `r-xp` pada region kode.

### Tantangan
Ubah `03_fork_shared_addr.c` agar child menulis ke heap dan parent, setelah `wait`, juga menulis lalu
mencetak — buktikan masing-masing punya salinan terpisah. Tambahkan sebuah variabel `static` lokal dan
amati perilakunya (hint: storage duration static, tetapi scope lokal).

---

## 9. Checklist pemahaman

- [ ] Saya bisa menjelaskan beda program dan process.
- [ ] Saya tahu alamat yang dicetak `printf("%p")` adalah virtual, bukan fisik.
- [ ] Saya bisa menjelaskan mengapa dua process bisa punya virtual address sama tanpa berbagi RAM.
- [ ] Saya mengerti ASLR dan kenapa alamat berubah tiap run.
- [ ] Saya bisa membaca kolom dasar `/proc/self/maps` dan tahu itu fitur Linux.

---

## 10. Ringkasan

Alamat dalam program adalah **virtual**; kernel dan MMU menerjemahkannya ke RAM lewat page table.
Setiap process punya ruang alamat sendiri, itulah sebabnya alamat yang sama di dua process tidak
berarti memory fisik yang sama. ASLR membuat alamat berubah tiap run — jangan pernah mengandalkannya.
`/proc/self/maps` memberi gambaran region memory process, sebagai observasi khusus Linux.

**Glosarium mini:** process, virtual address, physical address, page, page fault, ASLR,
`/proc/self/maps`. Lengkap di [`glossary.md`](../glossary.md).

## 11. Referensi

- [The Linux Programming Interface — TLPI (daftar isi)](https://man7.org/tlpi/toc-short.html) — ch. 6 Processes, ch. 49 Memory Mappings, ch. 50 Virtual Memory Operations.
- [proc_pid_maps(5) — Linux man-pages](https://man7.org/linux/man-pages/man5/proc_pid_maps.5.html) — format `/proc/[pid]/maps`.
- [proc(5) — Linux man-pages](https://man7.org/linux/man-pages/man5/proc.5.html) — penjelasan umum `/proc`, termasuk `/proc/self`.
- [Computer Systems: A Programmer's Perspective](https://csapp.cs.cmu.edu/) — Chapter 9 (Virtual Memory) sebagai bacaan konsep (nomor bab tidak diverifikasi).

## 12. Jembatan ke minggu 6

Sekarang kamu tahu alamat itu virtual dan process terisolasi. Pertanyaan berikutnya: **bagaimana
program meminta kernel melakukan sesuatu** — membuka file, mengalokasikan region memory lewat `mmap`?
Di situ kita bahas system call, file descriptor, dan perbedaan `read` vs `mmap`. Lanjut:
[`week-06-system-calls-mmap.md`](week-06-system-calls-mmap.md)
