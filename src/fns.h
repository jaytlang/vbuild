#ifndef INC_VBUILD_FNS
#define INC_VBUILD_FNS

struct ht;
struct config;

/* Basic logging macros */
#define INFO   " + "
#define CMD    "cmd"
#define CONF   "cfg"
#define ERROR  "!!!"
#define DIRECT "dir"

#define LOG(LEVEL, MSG) fprintf(stderr, "[%s] %s\n", LEVEL, MSG)

#define LOGIF(COND, LEVEL, MSG) \
	if(COND) LOG(LEVEL, MSG)
#define FAILIF(COND, MSG, LBL)           \
	do{                             \
		if(COND){               \
			LOG(ERROR, MSG); \
			goto LBL;        \
		}                       \
	}while(0)

#define RETIF(COND, MSG, RV)             \
	do{                             \
		if(COND){               \
			LOG(ERROR, MSG); \
			return RV;       \
		}                       \
	}while(0)

/* ht.c: Hash table implementation */

/* Initialize a hash table using a given hash function
 * code. Allocates the hash table + all fields for you
 * and returns the struct if successful; otherwise gives
 * back NULL
 */
struct ht *htinit(int hf);

/* Delete a hash table. The hash table must be
 * entirely existent and not null. This will always
 * succeed one way or another
 */
void htdeinit(struct ht *ht);

/* Insert a key into a hash table. Will overwrite any
 * existing entries like a typical dict, it's your job
 * to check whether a key exists before overriding it
 * Return -1 on failure, 0 on success. All arguments should
 * be non-null, don't use insert with NULL to delete a key
 */
int htinsert(struct ht *ht, unsigned int k, void *v);

/* Get the value of a given key out of a (valid) hash table
 * Requires that you place a valid key and hash table,
 * so the only mechanism of failure is if you never inserted
 * that key. Returns null if this is the case, your
 * value otherwise.
 */
void *htget(struct ht *ht, unsigned int k);

/* Delete a given key out of the (valid) hash table.
 * The key you're passing should exist, otherwise we will
 * return a failure. Returns NULL on failure otherwise the value
 * for the key you wanted us to delete
 */
void *htdel(struct ht *ht, unsigned int k);

/* conf.c: configuration file */

/* Generate or use the config file at CONFFILE
 * to create a new config struct, allocated for you
 * and returned. Returns null on error, and returns a
 * default configuration struct (which is synched to disk
 * at the default location) for you if the file doesn't
 * exist (note that config shouldn't ever fail with a default
 * config written, but in the case of a bug that defconfig
 * might persist to disk anyway)
 */
struct config *confinit(char *didgen);

/* Deallocates a valid configuration struct. Expects
 * a non-null config struct generated with confinit
 */
void confdeinit(struct config *conf);

/* execq.c: Execution queue */

/* Add an execution item to an execution queue. The command
 * passed must not be null, and must be understandable to
 * sh otherwise an error will occur. If the passed execq is
 * not null, we add a new item to the end of the execq passed,
 * otherwise we just make a new item altogether with no predecessor
 * Either way, a pointer to the new item is returned, or null
 * if something happens
 */
struct execitem *execqadd(struct execitem *q, char *cmd);

/* Free an entire (valid) execution queue, up through the linked list
 * until one of the next pointers is null
 */
void execqfree(struct execitem *q);

/* Execute an entire execq from front to back, by forking
 * off a child process and handing the shell the command specified
 * within that child. Returns -1 and sets failedat to point to the
 * failing command if anything goes wrong, otherwise sets failedat to
 * NULL and returns zero. Both passed pointers can't be null
 */
int execqrun(struct execitem *q, struct execitem **failedat);

/* directive.c: Directives! */

/* Put the given directive, named by label, into
 * the directive struct pointed to by out. Returns -1
 * if the parsing failed or 0 otherwise. Both pointers
 * passed must be non-null.
 */
int directiveparse(char *label, struct directive *out);

/* Run a given directive using the given configuration file.
 * Both pointers passed must be non-null. Returns -1 if
 * this failed somewhere along the way, 0 otherwise. Is
 * responsible for creating/flushing/freeing the execq along
 * the way for this directive.
 */
int directiverun(struct config *conf, struct directive *d);

/* util.c: Helper functions for Vivado */

/* Clean up from a vivado run. Deletes the extensions in BADCWDEXTS,
 * as well as the .Xil directory that vivado likes to create. Helps
 * keep the working directory clean. Returns -1 on failure, 0 on
 * success
 */
int utilcleanrun(void);

/* dummy.c: Example directive. */

/* Creates a file named hi.txt, or whatever TESTF is defined as,
 * and populates it another command later. The cleanup task
 * checks to make sure that the file is correctly populated
 * and then deletes it
 */
int dummyprepare(struct config *conf, struct execitem **q);
void dummycleanup(void);

/* build.c: Directive to build verilog bitstreams */

/* Creates a script at SCRIPTF (defined locally to build.c)
 * to feed to Vivado, in order to construct a systemverilog
 * bitstream as defined by the configuration struct. Currently
 * supports:
 * - Compilation of arbitrarily large projects within one src dir
 *   (this constitutes feature parity with GUI vivado)
 *
 * Will eventually support:
 * - Incremental builds upon failure
 * - Customizable optimization passes / strategies as I learn about them
 * - Probably more as I get to it...
 */
int buildprepare(struct config *conf, struct execitem **q);

/* Cleans up runtime files from Vivado executing,
 * for now defined as .jou, .log, .html, and .xml
 */
void buildcleanup(void);

#endif /* INC_VBUILD_FNS */
