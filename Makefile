UNAME := $(shell uname -s)
CC = cc
CFLAGS = -Wall -Wextra -g -std=c11 $(EXTRA_CFLAGS)

# Platform-specific source and flags
ifeq ($(UNAME), Darwin)
    PLATFORM_SRC = src/platform/platform_darwin.c
else
    PLATFORM_SRC = src/platform/platform_linux.c
    # glibc hides POSIX/GNU symbols under -std=c11; expose them on Linux only
    CFLAGS += -D_GNU_SOURCE
endif

# Scheduler sources (will grow with each phase)
SRCS = src/main.c src/pcb.c src/ready_queue.c src/scheduler.c src/timer.c src/shell.c src/monitor.c $(PLATFORM_SRC)

# Sample programs
PROG_SRCS = $(wildcard programs/*.c)
PROG_BINS = $(patsubst programs/%.c, programs/bin/%, $(PROG_SRCS))

# Unit test binary (no scheduler/shell/signals — pure data structure tests)
TEST_BIN = tests/test_queue
TEST_SRCS = tests/test_queue.c src/pcb.c src/ready_queue.c

# === Targets ===

all: minios programs

minios: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

programs: $(PROG_BINS)

programs/bin/%: programs/%.c
	@mkdir -p programs/bin
	$(CC) $(CFLAGS) -o $@ $<

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f minios $(TEST_BIN)
	rm -rf programs/bin

.PHONY: all programs test clean
