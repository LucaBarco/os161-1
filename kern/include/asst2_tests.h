#ifndef _ASST2TEST_H_
#define _ASST2TEST_H_

int test_minimal_acquire_release_acquire_counter(void);

int test_pid_upper_limit_counter(void);

int test_pid_release(void);

int test_pid_upper_limit_queue(void);

int test_minimal_acquire_release_acquire_queue(void);

void release_ids(int from, int to);

#endif
