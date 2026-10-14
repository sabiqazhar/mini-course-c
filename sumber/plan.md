# Peta Belajar 12 Minggu

Halaman ini adalah ringkasan rencana. Baca sekali di awal, lalu kembali setiap kali selesai satu
minggu untuk menandai progres.

## Aturan main yang paling penting

Satu minggu tidak selesai karena kalender berganti, tetapi karena kamu sudah memenuhi acceptance
criteria-nya. Kalau minggu 2 belum mantap, ulangi minggu 2. Tidak ada hadiah untuk cepat selesai,
dan ada harga yang jelas untuk lanjut dengan fondasi yang rapuh: minggu 5 sampai 12 akan terasa
seperti sihir, bukan seperti pemahaman.

Alokasi waktu yang diasumsikan: **5–7 jam per minggu**.

## Ringkasan per minggu

| Minggu | Topik | Pertanyaan utama | Fokus utama |
|---|---|---|---|
| 1 | Memory, variabel, alamat, pointer | Apa bedanya nilai, alamat, dan pointer? | Membangun model mental objek dan alamat |
| 2 | Array, struct, scope, lifetime, layout | Di mana objek hidup, kapan ia mati? | Scope vs lifetime vs lokasi |
| 3 | Heap, malloc/free, ownership | Bagaimana objek hidup melampaui satu fungsi? | Alokasi dan siapa yang melepas |
| 4 | realloc, dynamic array, tool | Bagaimana struktur data tumbuh dengan aman? | Resize aman + mulai pakai sanitizer |
| 5 | Process dan virtual memory | Apakah alamat yang dicetak itu alamat fisik? | Virtual address space, page, ASLR |
| 6 | System call, file I/O, mmap | Bagaimana program meminta layanan kernel? | fd, `read` vs `mmap`, cleanup |
| 7 | Thread dan race condition | Apa yang berubah saat dua alur berbagi data? | Interleaving dan data race |
| 8 | Mutex, atomic, bounded queue | Bagaimana thread bekerja sama dengan aman? | Critical section dan deadlock |
| 9 | Struktur data dan ownership | Bagaimana pointer membangun struktur dinamis? | Kontrak API dan invariant |
| 10 | Dasar allocator | Apa yang dilakukan allocator? | Arena, free list, fragmentasi |
| 11 | Debugging dan sanitizer | Bagaimana menemukan bug yang tak terlihat? | Memilih tool yang tepat |
| 12 | Proyek akhir | Bisakah kamu menggabungkan semuanya? | Bounded thread-safe queue |

## Tiga jenis penilaian

Setiap minggu memakai tiga cara untuk memeriksa pemahaman. "Sudah membaca materi" bukan salah
satunya.

**Explain** — jelaskan konsep dengan kata-katamu sendiri, atau gambar diagramnya di kertas.
Kalau kamu hanya bisa mengulang kalimat dari materi, itu belum Explain.

**Predict** — sebelum menjalankan sebuah program, tulis dulu apa output-nya dan bagaimana state
memory-nya. Jalankan, lalu bandingkan. Yang penting bukan benar atau salah tebakanmu, melainkan
apakah kamu bisa menjelaskan mengapa hasilnya begitu.

**Build/debug** — selesaikan latihan, lalu jelaskan mengapa solusimu benar. Untuk latihan debug,
jelaskan gejalanya, alat yang dipakai, akar masalah, dan perbaikan yang dilakukan.

## Checkpoint

Ada lima checkpoint. Lewat checkpoint tanpa bisa menjawab pertanyaannya berarti harus mengulang.

### Checkpoint 1 — setelah minggu 2

Tanpa membuka materi, gambarkan di kertas: sebuah `int x = 5;`, sebuah pointer `int *p = &x;`,
dan tunjukkan apa yang terjadi pada `*p = 10;`. Lalu jelaskan: apa beda scope dan lifetime?
Kalau kamu masih ragu membedakan alamat dan nilai, **jangan lanjut ke minggu 3**.

### Checkpoint 2 — setelah minggu 4

Bisa menunjukkan implementasi dynamic array yang lolos `-Wall -Wextra -Wpedantic` dan bersih
di bawah AddressSanitizer, termasuk saat `realloc` gagal. Bisa menyebutkan siapa pemilik tiap
alokasi di programmu.

### Checkpoint 3 — setelah minggu 6

Bisa menjelaskan mengapa dua process bisa mencetak alamat yang sama tanpa berbagi memory, dan
menjelaskan apa yang terjadi saat `mmap` mengembalikan `MAP_FAILED`.

### Checkpoint 4 — setelah minggu 8

Bisa menjelaskan mengapa `counter++` dari dua thread tidak aman, dan menunjukkan versi yang benar
dengan mutex. Bisa menyebutkan data apa yang dilindungi sebuah lock, dan mengapa lock/unlock harus
berpasangan. Kalau kamu belum paham shared state, ulangi minggu 7 sebelum menambah pola mutex
yang lebih rumit.

### Checkpoint 5 — setelah minggu 12

Proyek akhir jalan, diuji, dan kamu bisa menjelaskan desainnya: invariant queue, ownership tiap
resource, dan peran mutex serta condition variable yang dipakai.

## Yang sengaja tidak dibahas

Topik berikut relevan dan menarik, tetapi akan mengganggu jalur belajar kalau dimasukkan sekarang.
Catat sebagai jalur lanjutan setelah minggu 12:

- Menulis kernel atau device driver.
- Menulis compiler atau interpreter.
- Assembly lanjutan dan detail ABI.
- Lock-free programming dan memory ordering atomics secara mendalam.
- GPU programming.
- Menggantikan allocator libc dengan allocator produksi buatan sendiri.
- Optimisasi mikro tanpa benchmark.

## Cara memakai folder labs dan solutions

`labs/week-NN/` berisi program siap kompilasi: contoh yang benar, eksperimen terbimbing, dan
contoh rusak yang diberi nama jelas. `solutions/week-NN/` berisi solusi referensi untuk latihan.

Urutan yang disarankan: baca modul → jalankan lab → kerjakan latihan sendiri → bandingkan dengan
solusi. Membuka solusi sebelum mencoba itu seperti membaca halaman terakhir novel detektif: tidak
melanggar hukum, tetapi kamu yang rugi.
