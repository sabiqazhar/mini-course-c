# Solusi Minggu 7 — Thread & Race Condition

| Latihan  | Bentuk jawaban | File / cara |
|----------|----------------|-------------|
| Mudah    | Kode           | `three_threads.c` — 3 thread id berbeda, semua di-join. |
| Menengah | Observasi      | Modifikasi `labs/week-07/01_pthread_basic.c`: 4 thread, tiap menambah `counter` global 50.000× tanpa sinkronisasi. Jalankan 5× biasa (hasil berubah-ubah) dan 5× di bawah TSan (`gcc -fsanitize=thread`): TSan konsisten melaporkan data race. |
| Tantangan| Kode           | `private_accumulate.c` — akumulasi privat per-thread, digabung setelah join; selalu benar tanpa mutex. |

Build:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o three_threads three_threads.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread -o private_accumulate private_accumulate.c
```
