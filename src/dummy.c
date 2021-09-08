#include "dat.h"
#include "fns.h"

#include <alloca.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Haha test by example go brr */

#define TESTF	     "hello.txt"
#define QUOTE	     "Never gonna give you up! Never gonna let you down!"
#define UNUSEDVAR(X) (void)(X)

int
dummyprepare(struct config *conf, struct execitem **q)
{
	UNUSEDVAR(conf);

	RETIF(access(TESTF, F_OK) == 0, "Dummy test file exists", -1);
	*q = execqadd(NULL, "touch " TESTF);
	RETIF(!(*q), "Failed to create initial execq cmd", -1);
	RETIF(!execqadd(*q, "echo " QUOTE " > " TESTF),
	      "Failed to create appending execq cmd", -1);

	return 0;
}

void
dummycleanup(void)
{
	int fd, lenqt;
	char *buf;

	FAILIF(access(TESTF, F_OK) != 0, "Dummy test file doesn't exist", fail);
	fd = open(TESTF, O_RDONLY);
	FAILIF(fd < 0, "Couldn't open test file after it was written", fail);

	lenqt = strlen(QUOTE);
	buf = alloca(lenqt + 1);

	FAILIF(read(fd, buf, lenqt) != lenqt,
	       "Didn't read correct number of bytes", ffail);
	buf[lenqt] = '\0';
	FAILIF(strcmp(QUOTE, buf) != 0, "Incorrect string read back", ffail);

ffail:
	close(fd);
	LOGIF(unlink(TESTF) < 0, ERROR,
	      "Failed to unlink test file post-writing");

fail:
	return;
}
