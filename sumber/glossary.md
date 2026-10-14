# Glosarium

Glosarium ini bukan untuk dihafal. Pakai sebagai tempat kembali setiap kali sebuah istilah muncul
dan kamu merasa "sepertinya pernah baca, tapi tidak yakin". Definisi di sini sengaja pendek;
penjelasan panjangnya ada di modul yang bersangkutan.

## Dasar: objek, nilai, alamat

**object (objek)**
Region penyimpanan data di lingkungan eksekusi yang isinya bisa mewakili nilai. Dalam C, "objek"
tidak ada hubungannya dengan object-oriented programming. `int x = 5;` menjadikan `x` sebuah objek.

**value (nilai)**
Makna dari isi objek ketika dibaca dengan tipe tertentu. Byte yang sama bisa dibaca sebagai nilai
berbeda tergantung tipe yang dipakai untuk membacanya.

**address (alamat)**
Lokasi objek di memory. Dalam program kita yang tampak adalah *virtual address*, bukan alamat fisik
RAM — lihat minggu 5.

**pointer**
Objek yang menyimpan sebuah alamat. `int *p` berarti "p adalah pointer ke int", dan nilai `p`
adalah alamat dari suatu objek `int`.

**dereference**
Mengikuti pointer untuk mengakses objek yang ditunjuknya. `printf("%d", *p);` membaca nilai objek
di alamat yang disimpan `p`. Dereference pointer yang tidak menunjuk objek valid adalah
undefined behavior.

**null pointer**
Pointer yang secara eksplisit tidak menunjuk ke objek mana pun, ditulis `NULL` atau `0` dalam
konteks pointer. Dereference null pointer adalah undefined behavior (di Linux biasanya SIGSEGV,
tetapi itu perilaku sistem, bukan janji standar C).

## Struktur data dan representasi

**array**
Kumpulan objek bertipe sama yang tersusun berurutan. Indeks dimulai dari 0. Dalam banyak ekspresi,
nama array berubah ("decay") menjadi pointer ke elemen pertamanya — tetapi array *bukan* pointer.

**struct**
Tipe yang mengelompokkan beberapa member (field) dengan tipe boleh berbeda ke dalam satu objek.

**field / member**
Salah satu bagian bernama di dalam struct atau union. Diakses dengan `.` (objek) atau `->` (melalui pointer).

**pointer arithmetic**
Operasi aritmetika pada pointer, misalnya `p + 1`. Hasilnya bergantung pada tipe yang ditunjuk:
untuk `int *p`, `p + 1` menunjuk ke `int` berikutnya, bukan byte berikutnya. Operasi ini hanya
terdefinisi di dalam (dan satu elemen setelah) array yang sama.

**alignment (perataan)**
Syarat bahwa alamat suatu objek harus merupakan kelipatan dari nilai tertentu. Misalnya `int`
biasanya butuh perataan 4 byte. Perataan membuat akses memory lebih efisien, dan kadang diwajibkan
oleh perangkat keras.

**padding**
Byte sisipan yang ditambahkan compiler di dalam struct agar tiap field memenuhi perataannya.
Karena padding, `sizeof(struct)` bisa lebih besar dari jumlah ukuran field-nya, dan urutan field
bisa mengubah ukuran struct.

## Tempat dan umur objek

**scope**
Wilayah dalam source code tempat sebuah nama (identifier) bisa dipakai. Ini konsep *teks/kompilasi*.

**storage duration / lifetime (umur)**
Rentang waktu selama objek dijamin ada. Ini konsep *runtime*. Objek automatic hidup sampai blok
tempat ia dideklarasikan selesai; objek static hidup sepanjang program; objek allocated hidup
sampai `free` dipanggil. Scope dan lifetime sering berbarengan, tetapi tidak sama.

**stack**
Wilayah memory yang dipakai untuk pemanggilan fungsi dan, pada kebanyakan implementasi, variabel
lokal. Standar C tidak pernah mewajibkan "stack" — ini detail implementasi yang sangat umum.
Materi ini memakai model stack sebagai alat bantu berpikir, bukan sebagai janji standar.

**stack frame**
Blok pada stack yang dibuat untuk satu pemanggilan fungsi: berisi argumen, variabel lokal, dan
alamat kembali.

**heap**
Wilayah memory untuk alokasi dinamis yang umur objeknya kamu tentukan sendiri lewat
`malloc`/`free`. Istilah ini juga bukan istilah standar C; standar menyebutnya *allocated storage duration*.

## Alokasi dan kepemilikan

**allocation / deallocation**
Meminta memory dari sistem (`malloc`) dan mengembalikannya (`free`).

**ownership (kepemilikan)**
Konvensi tentang siapa yang bertanggung jawab melepas suatu alokasi. C tidak punya fitur bahasa
untuk ini, jadi ownership adalah *kesepakatan yang kamu tulis dan dokumentasikan*. Tanpa kesepakatan
yang jelas, kebocoran dan double free hampir pasti terjadi.

**aliasing**
Situasi ketika dua pointer atau lebih menunjuk ke objek yang sama. Mengubah lewat satu pointer
akan terlihat dari pointer lainnya. Ini sumber bug sekaligus sumber optimisasi.

## Kesalahan memory

**memory leak (kebocoran memory)**
Alokasi yang tidak pernah dilepas padahal tidak lagi dipakai. Program terus memegang memory yang
tidak bisa dipakai lagi; lama-lama pemakaian memory membengkak.

**dangling pointer**
Pointer yang menyimpan alamat objek yang sudah tidak valid (sudah di-`free`, atau objek lokal
yang fungsinya sudah kembali). Nilai pointer-nya masih ada, yang ditunjuk sudah tidak ada.

**use-after-free (UAF)**
Mengakses memory setelah ia dilepas. Termasuk kategori dangling pointer, dan merupakan
undefined behavior.

**double free**
Melepas blok yang sama dua kali. Undefined behavior; sering merusak struktur internal allocator.

**out-of-bounds**
Mengakses di luar batas objek: `a[10]` pada array berukuran 10, atau satu byte setelah akhir blok
`malloc`. Undefined behavior walaupun program tampak jalan normal.

**undefined behavior (UB)**
Perilaku yang tidak ditentukan oleh standar C — standar tidak memberi batasan apa pun mengenai
hasilnya. Penting: UB **tidak berarti** program pasti crash. Program bisa saja menghasilkan nilai
aneh, bebas crash, atau tampak benar hari ini lalu rusak setelah ganti compiler.

## Virtual memory dan sistem operasi

**process (proses)**
Satu program yang sedang berjalan, beserta ruang alamatnya sendiri, file descriptor-nya, dan
state-nya. Program adalah file di disk; process adalah kejadian hidup di memory.

**thread**
Alur eksekusi di dalam process. Thread dalam satu process berbagi ruang alamat yang sama,
tetapi masing-masing punya stack dan register sendiri.

**virtual address vs physical address**
Alamat yang dipakai program adalah virtual address, yang diterjemahkan perangkat keras (MMU)
menjadi physical address di RAM. Dua process boleh memakai virtual address yang sama persis dan
tetap menyentuh memori fisik yang berbeda.

**page**
Unit terkecil pemetaan memory yang dikelola kernel, umumnya 4096 byte di Linux x86-64.
Ukurannya bisa kamu dapatkan dengan `sysconf(_SC_PAGE_SIZE)`.

**page fault**
Kejadian saat program mengakses alamat virtual yang belum memiliki pemetaan fisik. Kernel
menangani ini: kalau aksesnya sah, halaman disiapkan lalu program lanjut; kalau tidak sah,
program menerima SIGSEGV.

**system call (syscall)**
Cara terkontrol bagi program di user space untuk meminta layanan kernel: buka file, minta memory,
buat process. Contoh: `open`, `read`, `mmap`, `write`.

**file descriptor (fd)**
Angka kecil (int) yang mewakili file atau objek I/O yang terbuka milik suatu process.
Diperoleh dari `open`, dipakai `read`/`write`, dikembalikan dengan `close`.

**memory mapping**
Menjadikan sebagian ruang alamat process berkorespondensi dengan isi file atau memory anonim,
lewat `mmap`. Akses ke wilayah itu dilakukan seperti mengakses array biasa.

## Concurrency

**shared state**
Data yang bisa diakses lebih dari satu thread.

**interleaving**
Urutan bergantiannya operasi dari beberapa thread. Banyak urutan berbeda mungkin terjadi, dan
sebagian di antaranya menghasilkan hasil yang salah.

**race condition**
Bug logika yang muncul karena hasil program bergantung pada *timing* atau urutan eksekusi.
Istilah ini lebih luas dari data race.

**data race**
Kondisi yang lebih sempit dan teknis: dua thread mengakses memory location yang sama, salah satunya
menulis, dan akses itu tidak diurutkan oleh mekanisme sinkronisasi. Di C, data race adalah
undefined behavior.

**critical section**
Bagian kode yang mengakses data bersama dan harus dijalankan seolah-olah atomik: tidak boleh ada
 thread lain yang ikut masuk mengubah data yang sama.

**mutex (mutual exclusion)**
Objek sinkronisasi yang dipakai untuk melindungi critical section: `pthread_mutex_lock` di awal,
`pthread_mutex_unlock` di akhir. Hanya satu thread yang bisa memegangnya pada satu waktu.

**deadlock**
Keadaan ketika dua thread atau lebih saling menunggu lock yang dipegang yang lain, sehingga tidak
ada yang maju.

**atomic operation**
Operasi yang tidak bisa diamati "setengah jalan" oleh thread lain, misalnya
`atomic_fetch_add` dari `<stdatomic.h>`. Atomic membuat *satu* operasi tidak terpecah; ia tidak
membuat rangkaian beberapa operasi menjadi atomik.

**condition variable**
Mekanisme untuk menunggu sampai suatu kondisi menjadi benar, tanpa busy-wait. Dipakai bersama
mutex, dan harus dipakai dengan loop uji kondisi karena spurious wakeup diperbolehkan.

## Tool dan debugging

**compiler warning**
Peringatan dari compiler tentang kode yang sah secara sintaksis tetapi mencurigakan. Bukan
kesalahan, dan sering kali petunjuk bug yang paling murah.

**debugger (GDB)**
Program untuk menjalankan program lain secara terkendali: berhenti di titik tertentu, melihat
nilai variabel, melihat stack trace.

**sanitizer**
Instrumen yang disuntikkan compiler ke dalam program untuk mendeteksi kesalahan saat runtime,
misalnya AddressSanitizer (memory), UndefinedBehaviorSanitizer (UB), ThreadSanitizer (data race).

**stack trace / backtrace**
Daftar pemanggilan fungsi yang sedang aktif pada suatu titik, dari fungsi paling dalam mundur ke
`main`.

**invariant**
Kondisi yang harus selalu benar tentang suatu data, misalnya "jumlah elemen queue tidak pernah
melebihi kapasitas" atau "linked list tidak pernah punya siklus". Sinkronisasi dan error handling
pada dasarnya ada untuk menjaga invariant.
