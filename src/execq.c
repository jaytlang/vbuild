#include "dat.h"
#include "fns.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXARGS 12

struct execitem *
execqadd(struct execitem *q, char *cmd)
{
	struct execitem *new, *eptr;
	new = calloc(1, sizeof(struct execitem));
	RETIF(!new, "Failed to calloc new execq", NULL);
	new->cmd = cmd;

	if(!q) return new;

	for(eptr = q; eptr->next != NULL; eptr = eptr->next)
		;
	eptr->next = new;
	return new;
}

void
execqfree(struct execitem *q)
{
	struct execitem *eptr;
	for(eptr = q; eptr != NULL;){
		struct execitem *oldeptr;

		oldeptr = eptr;
		eptr = eptr->next;
		free(oldeptr);
	}
}

int
execqrun(struct execitem *q, struct execitem **failedat)
{
	struct execitem *eptr;
	*failedat = NULL;

	for(eptr = q; eptr != NULL; eptr = eptr->next){
		LOG(CMD, eptr->cmd);
		FAILIF(system(eptr->cmd) != 0,
		       "Child process did not exit normally", failateptr);
	}
	return 0;

failateptr:
	*failedat = eptr;
	return -1;
}
