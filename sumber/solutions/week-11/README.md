# Solusi Minggu 11 — Debugging Tools

Latihan minggu ini ada di **§7** modul (bukan §8).

| Latihan  | Bentuk jawaban | File / cara |
|----------|----------------|-------------|
| Mudah    | Observasi      | Kompilasi semua `labs/week-11/*.c` dengan `-Wall -Wextra -Wpedantic`. Hitung warning: `01_off_by_one_broken.c` & `03_double_free_broken.c` & `05_signed_overflow_broken.c` memunculkan peringatan; `02_use_after_free_broken.c` & `03_uninitialized_broken.c` memunculkan `-Wuninitialized`/`-Wmaybe-uninitialized`. |
| Menengah | Kode           | `off_by_one_fixed.c` — batas loop diubah `i <= n` -> `i < n`; lulus ASan tanpa laporan. |
| Tantangan| Kode           | `ub_uninit_demo.c` — `int x; int y = x+1;`. Catatan jujur: UBSan biasanya TIDAK menangkap ini (ranah MSan); yang menangkapnya adalah `-Wuninitialized` saat compile. |

Build menengah & tantangan:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o off_by_one_fixed off_by_one_fixed.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=undefined -o ub_uninit_demo ub_uninit_demo.c
```
