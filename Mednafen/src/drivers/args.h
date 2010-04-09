#ifndef _DRIVERS_ARGH
typedef struct {
        const char *name;
	const char *description;
	int *var;

	void *subs;
	int substype;
} ARGPSTRUCT;

int ParseArguments(int argc, char *argv[], ARGPSTRUCT *argsps, char **);
int ShowArgumentsHelp(ARGPSTRUCT *argsps);

#define _DRIVERS_ARGH
#endif
