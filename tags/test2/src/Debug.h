#ifndef _DEBUG_H
#define _DEBUG_H

#if (1) //DEBUG
# define dbg_print(level, s, args...) \
	do { fprintf(stderr, "[%s]: "s"\n", level, ##args) ; fflush(stderr); } while(0)
#else
# define dbg_print(level, s, args...)
#endif

#endif /* _DEBUG_H */

