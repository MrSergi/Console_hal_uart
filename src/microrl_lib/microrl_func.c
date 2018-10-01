#include <string.h>				// for strcmp, strstr
#include "microrl.h"
#include "uart.h"				// for UART_GetCharBlocking, UART_SendString
#include "microrl_func.h"
#include "conf.h"
//#include "core_cm3.h"		// for NVIC_SystemReset

//Define this macros with correct write/read terminal functions
#define microrl_getChar			UART_GetCharBlocking
#define microrl_sendString		UART_SendString

terminalFunc_t terminalFuncArray[microrlNUM_OF_TERMINAL_FUNC];
int terminalFuncArrayIndex = 0;

microrl_t rl;					// Main terminal object
microrl_t * prl = &rl;

static void prv_registerBasicTerminalFuncs();
static int prv_getFuncArrayIndex(const char * name);
static void prv_printMainHelp();
static void prv_printTerminalFuncHelp(const char *name);

static int prv_execute(int argc, const char * const * argv);
#ifdef _USE_COMPLETE
static char ** prv_complet (int argc, const char * const * argv);
#endif
#ifdef _USE_CTLR_C
static void prv_sigint (void);
#endif

//Базовые терминальные функции
static int prv_TerminalFunc_about(int argc, const char * const * argv);
static int prv_TerminalFunc_help(int argc, const char * const * argv);
static int prv_TerminalFunc_clear(int argc, const char * const * argv);

void microrl_terminalInit()
{
	prv_registerBasicTerminalFuncs();

	microrl_init(prl, microrl_sendString);
	microrl_set_execute_callback (prl, prv_execute);

	#ifdef _USE_COMPLETE
	microrl_set_complete_callback (prl, prv_complet);
	#endif

	#ifdef _USE_CTLR_C
	microrl_set_sigint_callback (prl, prv_sigint);
	#endif
}
void microrl_terminalProcess()
{
	microrl_insert_char (prl, microrl_getChar());
}

void microrl_registerExecuteFunc(int (*func)(int, const char* const*), const char* name, const char* help)
{
	assert_param(terminalFuncArrayIndex < microrlNUM_OF_TERMINAL_FUNC);

	terminalFuncArray[terminalFuncArrayIndex].func = func;
	terminalFuncArray[terminalFuncArrayIndex].name = name;
	terminalFuncArray[terminalFuncArrayIndex].help = help;

	terminalFuncArrayIndex++;
}

void microrl_printString(const char *str)
{
	microrl_sendString(str);
}

void microrl_printStringWithEndl(const char *str)
{
	microrl_sendString(str);
	microrl_sendString(ENDL);
}

void microrl_printEndl()
{
	microrl_sendString(ENDL);
}

static void prv_registerBasicTerminalFuncs()
{
	microrl_registerExecuteFunc(prv_TerminalFunc_about, "about", NULL);
	microrl_registerExecuteFunc(prv_TerminalFunc_help, "help", "This function allows to view the available options and their descriptions.");
	microrl_registerExecuteFunc(prv_TerminalFunc_clear, "clear", "This function clears the screen.");
}

static int prv_getFuncArrayIndex(const char * name)
{
	int i;
	for (i = 0; i < terminalFuncArrayIndex; ++i) {
		if (strcmp(name, terminalFuncArray[i].name) == 0)	{
			return i;
		}
	}
	return (-1);
}

static int prv_execute(int argc, const char * const * argv)
{
	int funcIndex;

	funcIndex = prv_getFuncArrayIndex(argv[0]);
	if (funcIndex == (-1))	{
		microrl_printStringWithEndl("Unknown command. Type \"help\" to see available commands");
		return (-1);
	}
	return terminalFuncArray[funcIndex].func(--argc, ++argv);
}

#ifdef _USE_COMPLETE
//TODO simplify this. Quite difficult.
static char ** prv_complet (int argc, const char * const * argv)
{
	static char * compl_world [microrlNUM_OF_TERMINAL_FUNC + 1];
	int j = 0, i;
	compl_world[0] = NULL;
	if (argc == 1)	{
		char * bit = (char*)argv [argc-1];
		for (i = 0; i < terminalFuncArrayIndex; i++) {
			if (strstr(terminalFuncArray[i].name, bit) == terminalFuncArray[i].name) {
				compl_world [j++] = (char*)(terminalFuncArray[i].name);
			}
		}
	}
	compl_world [j] = NULL;
	return compl_world;
}
#endif

#ifdef _USE_CTLR_C
static void prv_sigint (void)
{
	NVIC_SystemReset();
}
#endif

static int prv_TerminalFunc_about(int argc, const char * const * argv)
{
	microrl_printString("Microrl based terminal. Ver. ");
	microrl_printStringWithEndl(MICRORL_LIB_VER);
	return 0;
}

static int prv_TerminalFunc_help(int argc, const char * const * argv)
{
	switch (argc)
	{
	case 0:
		prv_printMainHelp();
		break;
	case 1:
		prv_printTerminalFuncHelp(argv[0]);
		break;
	default:
		microrl_printStringWithEndl("Help is available only for high-level commands. It isn't available for subcommands.");
		break;
	}
	return 0;
}

static void prv_printMainHelp()
{
	microrl_printStringWithEndl("type \"help\" <command> for more details.");
	microrl_printEndl();
	microrl_printStringWithEndl("Available commands:");
	int i;
	for (i = 0; i < terminalFuncArrayIndex; ++i) {
		microrl_printString(terminalFuncArray[i].name);
		microrl_printString(" ");
	}
	microrl_printEndl();
}

static void prv_printTerminalFuncHelp(const char *name)
{
	int funcIndex = prv_getFuncArrayIndex(name);
	if (funcIndex == (-1))	{
		microrl_printStringWithEndl("Command not found.");
		return;
	}
	if (terminalFuncArray[funcIndex].help != NULL)	{
		microrl_printStringWithEndl(terminalFuncArray[funcIndex].help);
	}
	else	{
		microrl_printStringWithEndl("Help is not available for this command.");
	}
}

static int prv_TerminalFunc_clear(int argc, const char * const * argv)
{
	microrl_printString ("\033[2J");    // ESC seq for clear entire screen
	microrl_printString ("\033[H");     // ESC seq for move cursor at left-top corner
	return 0;
}
