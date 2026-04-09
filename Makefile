.DEFAULT_GOAL := all

CC ?= gcc
GDB ?= gdb
CFLAGS ?= -g -Wall -Wextra
LDLIBS ?= -lws2_32

BUILD_DIR := build
PROGRAMS := server-tcp client-tcp server-udp client-udp
EXECUTABLES := $(addprefix $(BUILD_DIR)/,$(addsuffix .exe,$(PROGRAMS)))
OBJECTS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(PROGRAMS)))

.PHONY: all clean tcp udp debug-server-tcp debug-client-tcp debug-server-udp debug-client-udp $(PROGRAMS)
.SECONDARY: $(OBJECTS)

all: $(EXECUTABLES)

tcp: $(BUILD_DIR)/server-tcp.exe $(BUILD_DIR)/client-tcp.exe

udp: $(BUILD_DIR)/server-udp.exe $(BUILD_DIR)/client-udp.exe

$(PROGRAMS): %: $(BUILD_DIR)/%.exe

$(BUILD_DIR):
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.exe: $(BUILD_DIR)/%.o | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LDLIBS) -o $@

debug-server-tcp: $(BUILD_DIR)/server-tcp.exe
	$(GDB) .\$(BUILD_DIR)\server-tcp.exe

debug-client-tcp: $(BUILD_DIR)/client-tcp.exe
	$(GDB) .\$(BUILD_DIR)\client-tcp.exe

debug-server-udp: $(BUILD_DIR)/server-udp.exe
	$(GDB) .\$(BUILD_DIR)\server-udp.exe

debug-client-udp: $(BUILD_DIR)/client-udp.exe
	$(GDB) .\$(BUILD_DIR)\client-udp.exe

clean:
	-powershell -NoProfile -Command "if (Test-Path '$(BUILD_DIR)') { Remove-Item -LiteralPath '$(BUILD_DIR)' -Recurse -Force -ErrorAction SilentlyContinue }"
