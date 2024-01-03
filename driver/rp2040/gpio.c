#include <arch/arch.h>
#include <sys/types.h>
#include <sys/register.h>


#define PADS_BANK0_BASE	0x4001c000
#define SIO_BASE		0xd0000000

#define GPIO(x)			MREG(PADS_BANK0_BASE + 0x4 + x * 4)
#define GPIO_OD			7
#define GPIO_IE			6
#define GPIO_DRIVE		4
#define GPIO_PUE		3
#define GPIO_PDE		2
#define GPIO_SCHMITT	1
#define GPIO_SLEWFAST	0

#define GPIO_OUT		MREG(SIO_BASE + 0x10)
#define GPIO_OE			MREG(SIO_BASE + 0x20)


void gpio_init(uint8_t gpio){
	GPIO(gpio) = (0x0 << GPIO_OD)
			   | (0x1 << GPIO_IE)
			   | (0x1 << GPIO_DRIVE)
			   | (0x1 << GPIO_PUE)
			   | (0x0 << GPIO_PDE)
			   | (0x0 << GPIO_SCHMITT)
			   | (0x0 << GPIO_SLEWFAST)
			   ;
	GPIO_OE |= (0x1 << gpio);
}

void gpio_set(uint8_t gpio, uint8_t v){
	uint8_t out;


	out = GPIO_OUT & (0x1 << gpio);
	GPIO_OUT = out | ((!!v) << gpio);
}
