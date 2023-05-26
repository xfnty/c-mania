#ifndef SYSTEM_MACROS_H
#define SYSTEM_MACROS_H

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>


#define __FILENAME__ ((strrchr(__FILE__, '/') != NULL) 						\
	 ? strrchr(__FILE__, '/') + 1 											\
	 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

#define STACKARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define IS_OUT_OF_BOUNDS(i, array) (i < 0 || i >= STACKARRAY_SIZE(array))

#define ID_INVALID (~(id_t)0)

#define CONSTRAIN(v, a, b) ((v < a) ? a : ((v > b) ? b : v))

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

#define SAFE_DELETE(p) { if (p != NULL) { free((void*)p); p = NULL; }}

typedef int error_t;


#endif
