#ifndef INC_VBUILD_DAT
#define INC_VBUILD_DAT

/* Configuration opts for vbuild
 * Specify part number, location of
 * constraints, top-level module, build
 * directory, and the target .bit filename,
 * respectively.
 */
struct config{
	char *pn;
	char *sources;
	char *constraints;
	char *topmodule;
	char *outputdir;
	char *bitstream;
};

/* Default configuration file we look for
 * If it doesn't exist, we wind up creating it
 * (see fns description)
 */
#define CONFFILE "conf.vbuild"

/* Configs can only be so many bytes large... */
#define MAXCONFSZ 500000000

/* Hash table structure. You shouldn't mutate
 * any of these fields, but they are available for
 * you to look at to accomplish basic tasks (e.g. iterating
 * through keys and values)
 */

struct ht{
	unsigned int nents;
	unsigned int *keys;

	unsigned char *reslist;
	void **vals;

	unsigned int capacity;
	unsigned int (*hf)(unsigned int k);
};

/* Hash table parameters, including function
 * codes, max/min load factors, and initial ht capacity
 */
#define FNV1AHF	      1
#define ROBOTMILLERHF 2
#define MAXLF	      0.75
#define MINLF	      0.1
#define INITCAP	      5

/* An execitem is an entry in the linked list
 * of commands to execute. Every single directive
 * in a build process gets a pre-execitem hook, in
 * which it may splice in arbitrarily many execitems
 * of its choosing beginning at this execitem
 */
struct execitem{
	char *cmd;
	struct execitem *next;
};

/* A directive is a todo item for vbuild.
 * It specifies a pre-build hook, in which it populates
 * an execq based on the provided global configuration
 * and the environment with tasks. The directive is
 * then executed via the execq, and after the execq is
 * executed cleanup() is called. The directive does not
 * need to take care of cleaning up the execq nor the conf,
 * but it is in charge of creating the execq.
 * The cleanup task should use the opportunity to tidy up the
 * working directory etc.
 */

/* Each directory has a name. It is typically small, like 'build'
 * 'clean' or 'tidy'.
 */
#define DIRECTIVENAMESZ 10

struct directive{
	char name[DIRECTIVENAMESZ];
	int (*prepare)(struct config *conf, struct execitem **q);
	void (*cleanup)(void);
};

#endif /* INC_VBUILD_DAT */
