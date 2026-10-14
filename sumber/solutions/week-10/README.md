# Solusi Minggu 10 — Memory Allocator

| Latihan  | Bentuk jawaban | File |
|----------|----------------|------|
| Mudah    | Kode           | `arena_reset_demo.c` — alokasi 5 int, reset, alokasi 2 int; bukti alamat kembali ke awal. |
| Menengah | Kode           | `free_list_zero.c` — `pool_alloc_zero` (seperti calloc) + pembahasan trade-off. |
| Tantangan| Kode           | `arena_recorded.c` — arena variabel yang mencatat alokasi agar bisa dibebaskan LIFO. |

Build:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o arena_reset_demo arena_reset_demo.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o free_list_zero free_list_zero.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o arena_recorded arena_recorded.c
```
