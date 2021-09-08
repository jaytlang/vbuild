#include "dat.h"
#include "fns.h"

#include <stdio.h>
#include <string.h>

/* So you want to add a directive?
 * - Name your build/cleanup functions name##build, name##cleanup
 * - Export your directive's build and cleanup functions in fns.h
 * - Add a TRYDIRECTIVE line to directiveparse
 */

#define TRYDIRECTIVE(INPUT, COMPARE, OUTSTRUCT)                            \
	do{                                                               \
		if(strcmp(INPUT, #COMPARE) == 0){                         \
			RETIF(strlen(INPUT) + 1 > DIRECTIVENAMESZ,         \
			      "Programmer error: directive name too long", \
			      -1);                                         \
			memcpy(OUTSTRUCT->name, INPUT, strlen(INPUT) + 1); \
			OUTSTRUCT->prepare = COMPARE##prepare;             \
			OUTSTRUCT->cleanup = COMPARE##cleanup;             \
			return 0;                                          \
		}                                                         \
	}while(0);

int
directiveparse(char *label, struct directive *out)
{
	TRYDIRECTIVE(label, dummy, out);
	TRYDIRECTIVE(label, build, out);

	LOG(ERROR, "Invalid directive passed");
	return -1;
}

int
directiverun(struct config *conf, struct directive *d)
{
	int res;
	struct execitem *q, *failedat;

	q = NULL;
	LOG(INFO, "Preparing directive...");
	res = d->prepare(conf, &q);

	FAILIF(!q, "Directive didn't populate an execq", fail);
	FAILIF(res < 0, "Directive failed to prepare", qfail);

	LOG(INFO, "Executing directive...");
	res = execqrun(q, &failedat);
	LOG(INFO, "Cleaning up directive...");
	d->cleanup();
	FAILIF(res < 0, "Directive execution did not work", qfail);
	execqfree(q);
	return 0;

qfail:
	execqfree(q);
fail:
	return -1;
}
