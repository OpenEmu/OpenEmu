
#ifndef _ERROR_H_
#define _ERROR_H_

/* Global variables */
FILE *error_log;

/* Function prototypes */
void error_init(void);
void error_shutdown(void);
void error(char *format, ...);

#endif /* _ERROR_H_ */
