# Minggu 12 — Makefile proyek antrean aman-thread.
# Pastikan baris perintah di bawah diawali TAB (bukan spasi).

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread
ASAN    ?= -fsanitize=address,undefined
TSAN    ?= -fsanitize=thread

.PHONY: all test asan tsan clean

all: test_basic test_mp_mc

test_basic: tsqueue.c test_basic.c tsqueue.h
	$(CC) $(CFLAGS) -o $@ tsqueue.c test_basic.c

test_mp_mc: tsqueue.c test_mp_mc.c tsqueue.h
	$(CC) $(CFLAGS) -o $@ tsqueue.c test_mp_mc.c

test: all
	./test_basic
	./test_mp_mc

asan:
	$(CC) $(CFLAGS) $(ASAN) -o test_basic.asan tsqueue.c test_basic.c
	$(CC) $(CFLAGS) $(ASAN) -o test_mp_mc.asan tsqueue.c test_mp_mc.c
	./test_basic.asan
	./test_mp_mc.asan

tsan:
	$(CC) $(CFLAGS) $(TSAN) -o test_mp_mc.tsan tsqueue.c test_mp_mc.c
	./test_mp_mc.tsan

clean:
	rm -f test_basic test_mp_mc test_basic.asan test_mp_mc.asan test_mp_mc.tsan
