#ifndef LOG_H_
#define LOG_H_

#define Log_BUFFER_SIZE 8192
extern char Log_buffer[Log_BUFFER_SIZE];

void Log_print(char *format, ...);
void Log_flushlog(void);

#endif /* LOG_H_ */
