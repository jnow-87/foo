#include <arch/arch.h>
#include <sys/register.h>


/* macros */
#define RESETS_BASE		0x4000c000

#define RESET			MREG(RESETS_BASE + 0x0)
#define RESET_DONE		MREG(RESETS_BASE + 0x8)


/* global functions */
void rp2040_resets_release(rp2040_resets_id_t io){
	RESET &= ~io;

	while((RESET_DONE & io) != io);
}

void rp2040_resets_halt(rp2040_resets_id_t io){
	RESET |= io & 0x00ffffff;
}
