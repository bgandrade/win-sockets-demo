.DEFAULT_GOAL := all

CC ?= gcc
GDB ?= gdb
CFLAGS ?= -g -Wall -Wextra
LDFLAGS ?= -lws2_32

TARGETS := server.exe client.exe

.PHONY: all clean server client debug-server debug-client

all: $(TARGETS)

server: server.exe

client: client.exe

server.exe: server.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

client.exe: client.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

debug-server: server.exe
	$(GDB) .\server.exe

debug-client: client.exe
	$(GDB) .\client.exe

clean:
	-cmd /C del /Q server.exe
	-cmd /C del /Q client.exe
