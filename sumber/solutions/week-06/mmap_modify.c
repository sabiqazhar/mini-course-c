/* Minggu 6 — Solusi menengah: ubah satu karakter di buffer mmap.
 *
 * Mode default MAP_PRIVATE: perubahan TIDAK kembali ke file (copy-on-write).
 * Mode "shared": perubahan MENULIS ke file asli (hati-hati!).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -o mmap_modify mmap_modify.c
 * Run  : ./mmap_modify file.txt            (MAP_PRIVATE, aman)
 *        ./mmap_modify file.txt shared     (MAP_SHARED, MENGUBAH FILE!)
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "data.txt";
    int shared = (argc > 2 && argv[2][0] == 's');

    /* MAP_SHARED menulis ke file -> butuh O_RDWR. MAP_PRIVATE cukup O_RDONLY
       (tulisannya masuk ke salinan copy-on-write, tak menyentuh file). */
    int fd = open(path, shared ? O_RDWR : O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) != 0) { perror("fstat"); close(fd); return 1; }
    if (st.st_size == 0) { fprintf(stderr, "file kosong\n"); close(fd); return 1; }

    /* Kita memang mengubah isi buffer. Untuk PRIVATE, PROT_WRITE membuat tulisan
       jatuh ke salinan COW (file tetap utuh); untuk SHARED, tulisan ke file asli. */
    int prot  = PROT_READ | PROT_WRITE;
    int flags = shared ? MAP_SHARED : MAP_PRIVATE;

    char *m = mmap(NULL, st.st_size, prot, flags, fd, 0);
    if (m == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);   /* fd boleh ditutup setelah mmap sukses */

    printf("sebelum: %.*s\n", (int)st.st_size, m);
    m[0] = (m[0] == 'H') ? 'J' : 'H';   /* ubah karakter pertama */
    printf("sesudah (di buffer): %.*s\n", (int)st.st_size, m);

    if (shared) {
        msync(m, st.st_size, MS_SYNC);
        printf("(MAP_SHARED) perubahan TERSIMPAN ke file asli.\n");
    } else {
        printf("(MAP_PRIVATE) perubahan TIDAK tersimpan (copy-on-write).\n");
    }

    munmap(m, st.st_size);
    return 0;
}
