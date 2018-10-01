#ifndef MICRORL_FUNC_H_
#define MICRORL_FUNC_H_

#define microrlNUM_OF_TERMINAL_FUNC	10

typedef struct {
	const char * name;
	const char * help;
	int (*func)(int, const char* const*);
} terminalFunc_t;

void microrl_terminalInit();
void microrl_terminalProcess();
void microrl_registerExecuteFunc(int (*func)(int, const char* const*), const char* name, const char* help);

void microrl_printString(const char *str);
void microrl_printStringWithEndl(const char *str);
void microrl_printEndl();

#endif
