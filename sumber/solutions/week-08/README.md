# Solusi Minggu 8 — Mutex & Atomic

| Latihan  | Bentuk jawaban | File / cara |
|----------|----------------|-------------|
| Mudah    | Kode           | `mutex_counter.c` — counter dilindungi mutex; hasil selalu sama (coba 10×). |
| Menengah | Kode           | `bench_mutex_vs_atomic.c` — bandingkan waktu mutex vs atomic via `clock()`. |
| Tantangan| Kode (di minggu 12) | Membuat bounded queue **memblokir** dengan condition variable. Pengerjaan penuh ada di `modules/week-12-final-project.md` dan `labs/week-12/`. |

Build:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o mutex_counter mutex_counter.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o bench bench_mutex_vs_atomic.c
```
