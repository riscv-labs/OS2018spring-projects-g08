#include <ulib.h>
#include <stdio.h>

static sem_t print_lock;

void do_yield(void)
{
	yield();
	yield();
	yield();
	yield();
	yield();
	yield();
}

int parent, pid1, pid2;

void loop(void)
{
	sem_wait(print_lock);
	cprintf("child 1.\n");
	sem_post(print_lock);
	while (1) ;
}

void work(void)
{
	sem_wait(print_lock);
	cprintf("child 2.\n");
	sem_post(print_lock);
	do_yield();
	if (kill(parent) == 0) {
		sem_wait(print_lock);
		cprintf("kill parent ok.\n");
		sem_post(print_lock);
		do_yield();
		if (kill(pid1) == 0) {
			sem_wait(print_lock);
			cprintf("kill child1 ok.\n");
			sem_post(print_lock);
			exit(0);
		}
	}
	exit(-1);
}

int main(void)
{
	print_lock = sem_init(1);

	parent = getpid();
	if ((pid1 = fork()) == 0) {
		loop();
	}

	assert(pid1 > 0);

	if ((pid2 = fork()) == 0) {
		work();
	}
	if (pid2 > 0) {
		sem_wait(print_lock);
		cprintf("wait child 1.\n");
		sem_post(print_lock);
		waitpid(pid1, NULL);
		panic("waitpid %d returns\n", pid1);
	} else {
		kill(pid1);
	}
	panic("FAIL: T.T\n");
}
