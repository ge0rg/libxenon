#include "xetypes.h"

void __check_argv() {
	if (__system_argv->magic != ARGV_MAGIC) {
		__system_argv->argc = 0;
		__system_argv->argv = NULL;
		return;
	}
}
