#ifndef SYS_ESCAPE_H
#define SYS_ESCAPE_H


/* macros */
// foreground colors
#define FG_BLACK	"\033[30m"
#define FG_RED		"\033[31m"
#define FG_GREEN	"\033[32m"
#define FG_YELLOW	"\033[33m"
#define FG_BLUE		"\033[34m"
#define FG_VIOLETT	"\033[35m"
#define FG_KOBALT	"\033[36m"
#define FG_WHITE	"\033[37m"

// background colors
#define BG_BLACK	"\033[40m"
#define BG_RED		"\033[41m"
#define BG_GREEN	"\033[42m"
#define BG_YELLOW	"\033[43m"
#define BG_BLUE		"\033[44m"
#define BG_VIOLETT	"\033[45m"
#define BG_KOBALT	"\033[46m"
#define BG_WHITE	"\033[47m"

// controls
#define CLEAR		"\033[2J"
#define CLEARLINE	"\033[K"
#define RESET_ATTR	"\033[0m"
#define BOLD		"\033[1m"
#define UNDERLINE	"\033[4m"
#define BLINK		"\033[5m"
#define INVERSE		"\033[7m"
#define INVISIBLE	"\033[8m"


#endif // SYS_ESCAPE_H
