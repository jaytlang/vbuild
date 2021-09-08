#include "dat.h"
#include "fns.h"

#include <stdio.h>
#include <unistd.h>

/* Build directive */

#define SCRIPTF "/tmp/make.tcl"

/* For cleanup... */
#define BADEXTS                             \
	{                                   \
		"jou", "log", "html", "xml" \
	}
#define NBADEXTS  4
#define BADDOTDIR ".Xil"

int
buildprepare(struct config *conf, struct execitem **q)
{
	FILE *fd;

	fd = fopen(SCRIPTF, "w");
	RETIF(!fd, "Failed to create temp build script", -1);

	fprintf(fd, "set_part %s\n", conf->pn);
	fprintf(fd, "read_verilog [glob %s/*.sv]\n", conf->sources);
	fprintf(fd, "read_xdc [glob %s/*.xdc]\n", conf->constraints);

	fprintf(fd, "synth_design -top %s -part %s\n", conf->topmodule,
		conf->pn);
	fprintf(fd, "opt_design -directive Explore\n");
	fprintf(fd, "place_design -directive Explore\n");
	fprintf(fd, "phys_opt_design -directive Explore\n");
	fprintf(fd, "route_design -directive Explore\n");
	fprintf(fd, "phys_opt_design -directive Explore\n");
	fprintf(fd, "write_bitstream -force %s/%s\n", conf->outputdir,
		conf->bitstream);

	fclose(fd);

	*q = execqadd(NULL, "vivado -mode batch -source " SCRIPTF);
	RETIF(!(*q), "Failed to add vivado command to execq", -1);

	return 0;
}

void
buildcleanup(void)
{
	LOGIF(utilcleanrun() < 0, ERROR, "Failed to clean working dir");
	LOGIF(unlink(SCRIPTF) < 0, ERROR,
	      "Failed to unlink vivado build script");
	return;
}
