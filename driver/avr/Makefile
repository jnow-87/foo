#
# Copyright (C) 2022 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-$(CONFIG_AVR_EEPROM) += eeprom.o
obj-$(CONFIG_AVR_GPIO) += gpio.o
obj-$(CONFIG_AVR_I2C) += i2c.o
obj-$(CONFIG_AVR_PWM16) += pwm16.o
obj-$(CONFIG_AVR_PWM8) += pwm8.o
obj-$(CONFIG_AVR_SPI) += spi.o
obj-$(CONFIG_AVR_UART) += uart.o


eeprom-cppflags-$(CONFIG_EEPROM_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
spi-cppflags-$(CONFIG_SPI_DEBUG) := -DBUILD_KERNEL_MSG_DEBUG
