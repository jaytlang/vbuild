#include "dat.h"
#include "fns.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Various utilities for use with Vivado, and otherwise */

#define ENTBUFSZ 500
#define KILLENT(NM, PREFIX, WHERE)                                             \
	do{                                                                   \
		char *pathbuf;                                                 \
                                                                               \
		pathbuf =                                                      \
		    calloc(strlen(PREFIX) + strlen(NM) + 2, sizeof(char));     \
		FAILIF(!pathbuf, "Failed to calloc buffer for dirent to kill", \
		       WHERE);                                                 \
                                                                               \
		memcpy(pathbuf, PREFIX, strlen(PREFIX) + 1);                   \
		pathbuf[strlen(PREFIX)] = '/';                                 \
                                                                               \
		memcpy(pathbuf + strlen(PREFIX) + 1, NM, strlen(NM) + 1);      \
                                                                               \
		if(unlink(pathbuf) < 0){                                      \
			LOG(ERROR, "Failed to delete junk file");              \
			free(pathbuf);                                         \
			goto WHERE;                                            \
		}                                                             \
		free(pathbuf);                                                 \
	}while(0)

#define BADCWDEXTS                          \
	{                                   \
		"jou", "log", "html", "xml" \
	}
#define NBADCWDEXTS 4
#define BADXILDIR   ".Xil"

int
rmrf(char *dirname, char **badexts, int nbadexts)
{
	struct dirent *dr;
	DIR *d;

	d = opendir(dirname);
	FAILIF(!d, "Failed to open directory", fail);

	while((dr = readdir(d))){
		char *lastdot, *c;
		int i;

		if(strcmp(dr->d_name, ".") == 0 ||
		   strcmp(dr->d_name, "..") == 0)
			continue;

		if(nbadexts == 0) KILLENT(dr->d_name, dirname, drfail);
		else{
			lastdot = NULL;
			for(c = dr->d_name; *c != '\0'; c++)
				if(*c == '.') lastdot = c;

			if(!lastdot) continue;
			++lastdot;

			for(i = 0; i < nbadexts; i++)
				if(strcmp(lastdot, badexts[i]) == 0)
					KILLENT(dr->d_name, dirname, drfail);
		}
	}

	closedir(d);
	return 0;

drfail:
	closedir(d);
fail:
	return -1;
}

int
utilcleanrun(void)
{
	char *badcwdexts[] = BADCWDEXTS;

	RETIF(rmrf(".", badcwdexts, NBADCWDEXTS) < 0,
	      "Failed to delete offending entries in cwd during cleanup", -1);

	if(access(BADXILDIR, F_OK) == 0){
		RETIF(rmrf(BADXILDIR, NULL, 0) < 0,
		      "Failed to delete entries from " BADXILDIR, -1);
		RETIF(rmdir(BADXILDIR) < 0, "Failed to delete " BADXILDIR, -1);
	}

	return 0;
}
