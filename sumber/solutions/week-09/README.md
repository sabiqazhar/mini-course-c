# Solusi Minggu 9 — Struktur Data & Ownership

| Latihan  | Bentuk jawaban | File |
|----------|----------------|------|
| Mudah    | Kode           | `list_len.c` — hitung panjang tanpa mengubah list. |
| Menengah | Kode           | `pop_front.c` — kembalikan nilai + head baru, bebaskan node di dalam fungsi, aman untuk list kosong. |
| Tantangan| Kode           | `reverse.c` — balik list hanya dengan mengubah pointer (tanpa alokasi). |

Build:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o list_len list_len.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o pop_front pop_front.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o reverse reverse.c
```
