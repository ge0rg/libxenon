#include <xetypes.h>

#define POSTADDR 0x20061010

void xenon_post(char post_code) {
	// shift code 56 bits left
	uint64_t code = ((uint64_t)post_code) << 56;
	// write code to post address
	*(volatile uint64_t*)POSTADDR = code;
}
