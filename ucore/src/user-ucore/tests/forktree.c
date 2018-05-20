#include <ulib.h>
#include <stdio.h>
#include <string.h>

#define DEPTH 4

static sem_t print_lock, sprint_lock;

void forktree(const char *cur);

void forkchild(const char *cur, char branch)
{
	char nxt[DEPTH + 1];

	if (strlen(cur) >= DEPTH)
		return;

	snprintf(nxt, DEPTH + 1, "%s%c", cur, branch);
	if (fork() == 0) {
		forktree(nxt);
		yield();
		exit(0);
	}
}

void forktree(const char *cur)
{
	sem_wait(print_lock);
	cprintf("%04x: I am '%s'\n", getpid(), cur);
	sem_post(print_lock);

	forkchild(cur, '0');
	forkchild(cur, '1');
}

int main(void)
{
	print_lock = sem_init(1);
	sprint_lock = sem_init(1);

	forktree("");
	return 0;
}
