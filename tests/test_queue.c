/*
 * tests/test_queue.c — Unit tests for ready_queue and pcb_init
 *
 * Links only against pcb.c and ready_queue.c (no scheduler, no signals).
 * Exit code 0 = all tests passed, 1 = at least one failure.
 */

#include <stdio.h>
#include <string.h>
#include "../src/ready_queue.h"
#include "../src/pcb.h"

static int tests_run    = 0;
static int tests_failed = 0;

#define CHECK(cond) do {                                                  \
    tests_run++;                                                          \
    if (!(cond)) {                                                        \
        fprintf(stderr, "FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #cond);\
        tests_failed++;                                                   \
    }                                                                     \
} while (0)

/* ── ready_queue ─────────────────────────────────────────────────────── */

static void test_init_empty(void) {
    rq_init();
    CHECK(rq_is_empty());
    CHECK(rq_size() == 0);
    CHECK(rq_dequeue() == -1);
    CHECK(rq_peek() == -1);
}

static void test_enqueue_dequeue_fifo(void) {
    rq_init();
    CHECK(rq_enqueue(0) == 0);
    CHECK(rq_enqueue(1) == 0);
    CHECK(rq_enqueue(2) == 0);
    CHECK(rq_size() == 3);
    CHECK(!rq_is_empty());
    CHECK(rq_dequeue() == 0);
    CHECK(rq_dequeue() == 1);
    CHECK(rq_dequeue() == 2);
    CHECK(rq_is_empty());
}

static void test_peek_does_not_remove(void) {
    rq_init();
    rq_enqueue(5);
    CHECK(rq_peek() == 5);
    CHECK(rq_size() == 1);
    CHECK(rq_peek() == 5);
}

static void test_remove_middle(void) {
    rq_init();
    rq_enqueue(0);
    rq_enqueue(1);
    rq_enqueue(2);
    CHECK(rq_remove(1) == 0);
    CHECK(rq_size() == 2);
    CHECK(rq_dequeue() == 0);
    CHECK(rq_dequeue() == 2);
    CHECK(rq_is_empty());
}

static void test_remove_head(void) {
    rq_init();
    rq_enqueue(3);
    rq_enqueue(4);
    CHECK(rq_remove(3) == 0);
    CHECK(rq_size() == 1);
    CHECK(rq_dequeue() == 4);
}

static void test_remove_tail(void) {
    rq_init();
    rq_enqueue(7);
    rq_enqueue(8);
    CHECK(rq_remove(8) == 0);
    CHECK(rq_size() == 1);
    CHECK(rq_dequeue() == 7);
}

static void test_remove_nonexistent(void) {
    rq_init();
    rq_enqueue(0);
    CHECK(rq_remove(99) == -1);
    CHECK(rq_size() == 1);
}

static void test_enqueue_full(void) {
    rq_init();
    for (int i = 0; i < MAX_PROCESSES; i++)
        CHECK(rq_enqueue(i) == 0);
    CHECK(rq_enqueue(0) == -1);
    CHECK(rq_size() == MAX_PROCESSES);
}

static void test_wrap_around(void) {
    rq_init();
    /* Fill, dequeue half, fill again — exercises circular wrap */
    for (int i = 0; i < MAX_PROCESSES; i++)
        rq_enqueue(i);
    for (int i = 0; i < MAX_PROCESSES / 2; i++)
        rq_dequeue();
    int added = 0;
    for (int i = 0; i < MAX_PROCESSES / 2; i++) {
        if (rq_enqueue(i) == 0) added++;
    }
    CHECK(added == MAX_PROCESSES / 2);
    CHECK(rq_size() == MAX_PROCESSES);
}

/* ── pcb ─────────────────────────────────────────────────────────────── */

static void test_pcb_init(void) {
    pcb_t p;
    pcb_init(&p, 1234, "countdown");
    CHECK(p.pid == 1234);
    CHECK(strncmp(p.name, "countdown", MAX_NAME_LEN) == 0);
    CHECK(p.state == PROC_NEW);
    CHECK(p.regs_valid == 0);
    CHECK(p.cpu_time_ms == 0.0);
    CHECK(p.wait_time_ms == 0.0);
    CHECK(p.context_switches == 0);
}

static void test_pcb_init_name_truncation(void) {
    pcb_t p;
    char long_name[MAX_NAME_LEN + 10];
    memset(long_name, 'a', sizeof(long_name));
    long_name[sizeof(long_name) - 1] = '\0';
    pcb_init(&p, 1, long_name);
    CHECK(p.name[MAX_NAME_LEN - 1] == '\0');
}

static void test_pcb_state_name(void) {
    CHECK(pcb_state_name(PROC_NEW)        != NULL);
    CHECK(pcb_state_name(PROC_READY)      != NULL);
    CHECK(pcb_state_name(PROC_RUNNING)    != NULL);
    CHECK(pcb_state_name(PROC_BLOCKED)    != NULL);
    CHECK(pcb_state_name(PROC_TERMINATED) != NULL);
}

/* ── main ────────────────────────────────────────────────────────────── */

int main(void) {
    test_init_empty();
    test_enqueue_dequeue_fifo();
    test_peek_does_not_remove();
    test_remove_middle();
    test_remove_head();
    test_remove_tail();
    test_remove_nonexistent();
    test_enqueue_full();
    test_wrap_around();
    test_pcb_init();
    test_pcb_init_name_truncation();
    test_pcb_state_name();

    if (tests_failed == 0) {
        printf("OK — %d/%d tests passed\n", tests_run, tests_run);
        return 0;
    } else {
        fprintf(stderr, "FAILED — %d/%d tests failed\n", tests_failed, tests_run);
        return 1;
    }
}
