#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MATSIZE     10

static int mata[MATSIZE][MATSIZE];
static int matb[MATSIZE][MATSIZE];
static int matc[MATSIZE][MATSIZE];
static sem_t print_lock;

void work(unsigned int times)
{
	int i, j, k, size = MATSIZE;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			mata[i][j] = matb[i][j] = 1;
		}
	}

	yield();

	sem_wait(print_lock);
	cprintf("pid %d is running (%d times)!.\n", getpid(), times);
	sem_post(print_lock);

	while (times-- > 0) {
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				matc[i][j] = 0;
				for (k = 0; k < size; k++) {
					matc[i][j] += mata[i][k] * matb[k][j];
				}
			}
		}
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				mata[i][j] = matb[i][j] = matc[i][j];
			}
		}
	}
	sem_wait(print_lock);
	cprintf("pid %d done!.\n", getpid());
	sem_post(print_lock);
	exit(0);
}

const int total = 20;

int main(void)
{
	print_lock = sem_init(1);
	int pids[total];
	memset(pids, 0, sizeof(pids));

	int i;
	for (i = 0; i < total; i++) {
		if ((pids[i] = fork()) == 0) {
			srand(i * i);
			int times = (((unsigned int)rand()) % total);
			times = (times * times + 10) * 100;
			work(times);
		}
		if (pids[i] < 0) {
			goto failed;
		}
	}

	sem_wait(print_lock);
	cprintf("fork ok.\n");
	sem_post(print_lock);

	for (i = 0; i < total; i++) {
		if (wait() != 0) {
			sem_wait(print_lock);
			cprintf("wait failed.\n");
			sem_post(print_lock);
			goto failed;
		}
	}
	
	cprintf("matrix pass.\n");
	return 0;

failed:
	for (i = 0; i < total; i++) {
		if (pids[i] > 0) {
			kill(pids[i]);
		}
	}
	panic("FAIL: T.T\n");
}
