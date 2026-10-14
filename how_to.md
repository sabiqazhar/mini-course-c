# Kursus Low-Level C — versi HTML interaktif

Satu file `index.html` di folder ini berisi **seluruh materi 12 minggu** (modul, lab, solusi,
glosarium, referensi) dalam bentuk buku interaktif. Tidak butuh internet, tidak butuh server,
tidak butuh instalasi apa pun.

## Cara membuka

Klik dua kali `index.html`, atau:

```bash
xdg-open index.html      # Linux
open index.html          # macOS
start index.html         # Windows
```

Kalau mau lewat server lokal (misalnya karena browser membatasi file lokal):

```bash
python3 -m http.server 8000
# lalu buka http://localhost:8000
```

## Yang bisa dilakukan di dalamnya

| Fitur | Cara pakai |
|---|---|
| Navigasi 12 minggu | Klik daftar di sidebar kiri |
| Tab per minggu | **Materi · Lab · Latihan · Solusi · Checklist** |
| Lihat kode lab | Klik judul file di tab Lab (collapsible, syntax-highlighted) |
| Salin kode | Tombol **Salin** di pojok kanan atas setiap blok kode |
| Cari materi | Kotak pencarian di atas, atau tekan `/` |
| Centang checklist | Tab Checklist per minggu — tersimpan otomatis di browser |
| Tandai minggu selesai | Tombol di bawah tab Checklist |
| Gelap / terang | Tombol **Gelap**, atau tekan `t` |
| Pindah halaman | `j` (berikutnya) / `k` (sebelumnya) |
| Cetak / PDF | `Ctrl+P` (sidebar & tombol otomatis disembunyikan) |

Progres (checklist + minggu yang sudah ditandai selesai) disimpan di `localStorage` browser,
jadi tetap ada saat kamu buka lagi di komputer dan browser yang sama.

## Isi folder ini

```
index.html        buku interaktif (mandiri, offline)
CARA_PAKAI.md     file ini
build_html.py     generator: regenerate index.html dari folder sumber/
sumber/           sumber asli kursus (markdown + semua file .c/.h/Makefile)
```

## Regenerate HTML (kalau kamu edit materinya)

```bash
pip install markdown pygments      # sekali saja
python3 build_html.py sumber index.html
```

## Menjalankan kode C

Semua file `.c` ada di `sumber/labs/week-NN/` dan `sumber/solutions/week-NN/`.
Perintah kompilasi yang tepat juga ditampilkan di atas setiap blok kode di HTML.
Pola umumnya:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o prog prog.c      # + -pthread untuk minggu 7+
./prog

gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined -o prog prog.c
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=thread -pthread -o prog prog.c
```

Rincian toolchain ada di halaman **Setup Toolchain** di dalam HTML.
