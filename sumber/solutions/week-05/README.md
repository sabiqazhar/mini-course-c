# Solusi Minggu 5 — Proses & Virtual Memory

| Latihan  | Bentuk jawaban | File / cara |
|----------|----------------|-------------|
| Mudah    | Observasi      | Jalankan `labs/week-05/01_virtual_alamat` 5 kali. Alamat `lokal` (stack) akan **berbeda setiap kali** karena ASLR (minggu 5, modul §4). Kesimpulan: stack address tidak stabil antar-eksekusi; jangan pernah menyimpan/sinkronisasi alamat antar-proses. |
| Menengah | Observasi      | Buka `/proc/self/maps` (hasil eksperimen 1 modul). Cari baris berlabel `[heap]` (heap libc), dan baris `libc.so.6` (pustaka dipetakan). `r-xp` = read/exec, private: region kode (bisa dieksekusi, tidak bisa ditulis, copy-on-write). |
| Tantangan| Kode           | `fork_static_demo.c` — bukti child & parent punya salinan terpisah untuk heap dan untuk variabel `static` lokal. |

Build tantangan:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o fork_static_demo fork_static_demo.c
./fork_static_demo
```
