#include <config/config.h>
#include <arch/avr/atmega.h>
#include <kernel/init.h>
#include <kernel/devfs.h>
#include <sys/types.h>
#include <sys/register.h>


/* macros */
#define PORT_OFFSET		4
#define PIN_OFFSET		0
#define PORT_INDICATOR	9


/* types */
typedef struct{
	uint8_t in_mask,
			out_mask;

	char *reg_ddr,
		 *reg_pin,
		 *reg_port;
} port_cfg_t;


/* local/static prototypes */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);


/* static variables */
static port_cfg_t ports[] = {
#ifdef CONFIG_GPIO_PORTA_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTA_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTA_OUT_MASK,
		.reg_ddr = (char*)DDRA,
		.reg_pin = (char*)PINA,
		.reg_port = (char*)PORTA
	},
#endif

#ifdef CONFIG_GPIO_PORTB_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTB_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTB_OUT_MASK,
		.reg_ddr = (char*)DDRB,
		.reg_pin = (char*)PINB,
		.reg_port = (char*)PORTB
	},
#endif

#ifdef CONFIG_GPIO_PORTC_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTC_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTC_OUT_MASK,
		.reg_ddr = (char*)DDRC,
		.reg_pin = (char*)PINC,
		.reg_port = (char*)PORTC
	},
#endif

#ifdef CONFIG_GPIO_PORTD_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTD_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTD_OUT_MASK,
		.reg_ddr = (char*)DDRD,
		.reg_pin = (char*)PIND,
		.reg_port = (char*)PORTD
	},
#endif

#ifdef CONFIG_GPIO_PORTE_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTE_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTE_OUT_MASK,
		.reg_ddr = (char*)DDRE,
		.reg_pin = (char*)PINE,
		.reg_port = (char*)PORTE
	},
#endif

#ifdef CONFIG_GPIO_PORTF_IN_MASK
	{
		.in_mask = CONFIG_GPIO_PORTF_IN_MASK,
		.out_mask = CONFIG_GPIO_PORTF_OUT_MASK,
		.reg_ddr = (char*)DDRF,
		.reg_pin = (char*)PINF,
		.reg_port = (char*)PORTF
	},
#endif
};


/* local functions */
static int init(void){
	size_t i,
		   j;
	char name[] = "pa0";
	int *data;
	devfs_ops_t ops;


	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = 0x0;

	for(i=0; i<CONFIG_AVR_NPORTS; i++){
		if(!ports[i].in_mask && !ports[i].out_mask)
			continue;

		/* allocate port */
		name[1] = 'a' + i;
		name[2] = 0;
		data = (int*)((i << PORT_OFFSET) | PORT_INDICATOR);

		if(devfs_dev_register(name, &ops, data) == 0x0)
			return -errno;

		/* allocate pins */
		for(j=0; j<8; j++){
			if(!(ports[i].in_mask & (0x1 << j)) && !(ports[i].out_mask & (0x1 << j)))
				continue;

			name[2] = '0' + j;
			data = (int*)((i << PORT_OFFSET) | j);

			if(devfs_dev_register(name, &ops, data) == 0x0)
				return -errno;
		}

		/* configure port direction
		 *
		 * 	according to the following logic the out_mask can be
		 * 	used directly, due to the DDR bits being input on 0
		 * 	and output on 1
		 *
		 * 	bit in		bit in		bit in
		 * 	out_mask	in_mask		DDR (0 = input, 1 = output)
		 * 		0			0			0 (retain reset value)
		 * 		0			1			0 (input)
		 * 		1			0			1 (output)
		 * 		1			1			1 (output)
		 */
		*ports[i].reg_ddr = ports[i].out_mask;
		*ports[i].reg_port = ports[i].in_mask;
	}

	return 0;
}

driver_init(init);

static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	uint8_t port,
			pin;
	uint8_t mask;
	uint8_t v;


	/* identify port, pin and pin mask */
	port = (((int)dev->data) & 0xf0) >> PORT_OFFSET;
	pin = ((int)dev->data) & 0xf;
	mask = 0x1;

	if(pin == PORT_INDICATOR){
		mask = 0xff;
		pin = 0;
	}

	/* read port */
	v = *ports[port].reg_pin & (ports[port].in_mask | ports[port].out_mask);
	*((char*)buf) = bits(v, pin, mask);

	return 1;
}

static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	uint8_t port,
			pin;
	uint8_t mask;
	uint8_t v;


	/* identify port, pin and pin mask */
	port = (((int)dev->data) & 0xf0) >> PORT_OFFSET;
	pin = ((int)dev->data) & 0xf;
	mask = 0x1 << pin;

	if(pin == PORT_INDICATOR){
		mask = 0xff;
		pin = 0;
	}

	/* update port */
	v = *ports[port].reg_pin & ~mask;
	v |= (*((char*)buf) << pin) & mask;
	*ports[port].reg_port = v & ports[port].out_mask;

	return 1;
}
