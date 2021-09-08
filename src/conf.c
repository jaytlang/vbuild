#include "dat.h"
#include "fns.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* So you want to change the config file?
 * - Update the defconfig
 * - Add a TRYKEY to fillconfline
 * - Add a CHECKKEY to validateconfig
 * - Free your key in confdeinit
 */

static const char *defconf = "pn=xc7a100tcsg324-3\n"
			     "sources=src/\n"
			     "constraints=xdc/\n"
			     "topmodule=top\n"
			     "outputdir=build/\n"
			     "bitstream=target.bit\n";

#define TRYKEY(CONF, K, V, WITH)                                            \
	do{                                                                \
		if(strncmp(K, #WITH, strlen(#WITH) + 1) == 0){             \
			RETIF((*CONF).WITH, "Duplicate config entry", -1);  \
                                                                            \
			(*CONF).WITH = calloc(strlen(V) + 1, sizeof(char)); \
			RETIF(!(*CONF).WITH,                                \
			      "Calloc for new config entry failed", -1);    \
                                                                            \
			memcpy((*CONF).WITH, V, strlen(V) + 1);             \
			return 0;                                           \
		}                                                          \
	}while(0)

#define CHECKKEY(CONF, K) \
	RETIF(!(*CONF).K, "Config missing required field " #K, -1)

/* Fill a field in a conf struct by its key
 * Returns -1 if the conf struct already has
 * this key or if the passed key is illegal
 */
int
fillconfline(struct config *config, char *k, char *v)
{
	TRYKEY(config, k, v, pn);
	TRYKEY(config, k, v, sources);
	TRYKEY(config, k, v, constraints);
	TRYKEY(config, k, v, topmodule);
	TRYKEY(config, k, v, outputdir);
	TRYKEY(config, k, v, bitstream);

	LOG(ERROR, "Passed an illegal key in config file");
	return -1;
}

/* Validate a config, ensuring all fields are non-null */
int
validateconfig(struct config *config)
{
	CHECKKEY(config, pn);
	CHECKKEY(config, sources);
	CHECKKEY(config, constraints);
	CHECKKEY(config, topmodule);
	CHECKKEY(config, outputdir);
	CHECKKEY(config, bitstream);
	return 0;
}

/* Bootstrap the configuration file
 * if somebody didn't create it. Assumes
 * that the file does _not_ currently exist
 * and we are able to create it...
 */
int
confbootstrap(void)
{
	int fd, dclen;

	LOG(INFO, "Generating new configuration file");
	fd = open(CONFFILE, O_RDWR | O_CREAT, 0644);
	FAILIF(fd < 0, "Failed to create new config file", fail);

	dclen = strlen(defconf);

	FAILIF(write(fd, defconf, dclen) != dclen,
	       "Failed to entirely write default config", ffail);
	LOG(INFO, "Generated default configuration file");
	close(fd);
	return 0;

ffail:
	close(fd);
	LOGIF(unlink(CONFFILE) < 0, ERROR,
	      "Failed to unlink attempted new conf file");
fail:
	return -1;
}

/* Validate a given line and then add it to the conf
 * Only keys in confkeys will make it thru here
 * -1 if we have something invalid
 */
int
parseconfline(struct config *config, char *line)
{
	int out;
	char *c, *eqc;

	for(c = line; *c != '='; c++){
		RETIF(*c == '\0' || *c == '\n', "Syntax error in configuration",
		      -1);
		RETIF(*c == ' ', "No whitespace allowed in configuration", -1);
	}
	RETIF(c == line, "Missing a conf key or you have leading whitespace",
	      -1);

	/* hold my beer */
	eqc = c;
	*eqc = '\0';

	for(++c; *c != '\0'; c++)
		RETIF(*c == ' ', "No whitespace allowed in configuration", -1);

	RETIF(c == eqc + 1, "No value specified for config key", -1);

	out = fillconfline(config, line, eqc + 1);
	LOGIF(out < 0, ERROR, "Failed to fill conf line\n");
	return out;
}

struct config *
confinit(char *didgen)
{
	struct config *config;
	char *startf, *startl, *endl;
	int fd, fsz;
	struct stat st;

	LOG(INFO, "Initializing configuration");
	config = calloc(1, sizeof(struct config));
	FAILIF(!config, "Failed to calloc config struct", fail);

	/* NOTE: this is janky and could be messed with bigtime
	 * This should be fixed later on but this works
	 * with a user who knows not to play with conf.vbuild during
	 * execution
	 */
	if(access(CONFFILE, F_OK) < 0){
		FAILIF(confbootstrap() < 0, "Failed to bootstrap defconfig",
		       allocfail);
		*didgen = 1;
	}else
		*didgen = 0;

	fd = open(CONFFILE, O_RDONLY);
	FAILIF(fd < 0, "Failed to open config file for reading", allocfail);
	FAILIF(fstat(fd, &st) < 0, "Failed to stat config file", ffail);
	fsz = st.st_size;

	FAILIF(fsz > MAXCONFSZ,
	       "You have a huge conf file bro keep it under 500MB", ffail);

	startf = calloc(fsz, sizeof(char));
	FAILIF(!startf, "Failed to calloc memory to hold conf file", ffail);
	FAILIF(read(fd, startf, fsz) != fsz, "Failed to read whole file",
	       cfail);

	for(startl = startf; startl < startf + fsz; startl = endl + 1){
		for(endl = startl; *endl != '\n' && *endl != '\0'; endl++)
			;
		FAILIF(*endl == '\0',
		       "Expect trailing newline at end of file\n", cfail);

		*endl = '\0';
		LOG(CONF, startl);
		FAILIF(parseconfline(config, startl) < 0,
		       "Failed to parse config line\n", cfail);
	}

	FAILIF(validateconfig(config) < 0,
	       "Failed to validate config: missing field?\n", cfail);
	free(startf);
	close(fd);
	LOG(INFO, "Initialized configuration successfully");
	return config;

cfail:
	free(startf);
ffail:
	close(fd);
allocfail:
	confdeinit(config);
fail:
	return NULL;
}

void
confdeinit(struct config *conf)
{
	if(conf->pn) free(conf->pn);
	if(conf->sources) free(conf->sources);
	if(conf->constraints) free(conf->constraints);
	if(conf->topmodule) free(conf->topmodule);
	if(conf->outputdir) free(conf->outputdir);
	if(conf->bitstream) free(conf->bitstream);
	free(conf);
}
