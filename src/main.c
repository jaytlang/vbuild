#include "dat.h"
#include "fns.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
printusageanddie(void)
{
	printf("========= vbuild =========\n");
	printf("===== Jay Lang, 2021 =====\n");
	printf("Usage: vbuild [directives...]\n");
	exit(0);
}

int
main(int argc, char *argv[])
{
	struct config *c;
	char didgen;
	int i;

	if(argc == 1) printusageanddie();

	c = confinit(&didgen);
	FAILIF(!c, "Failed to generate config", fail);

	for(i = 1; i < argc; i++){
		struct directive d;

		FAILIF(directiveparse(argv[i], &d) < 0,
		       "Failed to parse a directive", cfail);
		LOG(DIRECT, d.name);
		FAILIF(directiverun(c, &d) < 0, "Failed to run directive",
		       cfail);
		LOG(INFO, "Directive executed successfully");
	}

	LOG(INFO, "Complete!");
	confdeinit(c);
	return 0;

cfail:
	confdeinit(c);
fail:
	return -1;
}
