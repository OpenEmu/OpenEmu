#ifndef _DRIVERS_CONFIGH
typedef struct {
        const char *name;
        void *ptr;
        int len;
} CFGSTRUCT;

void SaveMDFNConfig(const char *filename, CFGSTRUCT *cfgst);
int LoadMDFNConfig(const char *filename, CFGSTRUCT *cfgst);

/* Macros for building CFGSTRUCT structures. */

/* CFGSTRUCT structures must always end with ENDCFGSTRUCT */
#define ENDCFGSTRUCT	{ 0,0,0 }

/* When this macro is used, the config loading/saving code will parse
   the new config structure until the end of it is detected, then it
   will continue parsing the original config structure.
*/
#define ADDCFGSTRUCT(x) { 0,&x,0 }

/* Oops.  The NAC* macros shouldn't have the # in front of the w, but
   fixing this would break configuration files of previous versions and it 
   isn't really hurting much.
*/

/* Single piece of data(integer). */
#define AC(x)   { #x,&x,sizeof(x)}
#define NAC(w,x) { #w,&x,sizeof(x)}

/* Array. */
#define ACA(x)   {#x,x,sizeof(x)}
#define NACA(w,x) {#w,x,sizeof(x)}

/* String(pointer) with automatic memory allocation. */
#define ACS(x)  {#x,&x,0}
#define NACS(w,x)  {#w,&x,0}

#define _DRIVERS_CONFIGH
#endif
