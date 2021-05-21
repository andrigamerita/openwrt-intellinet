#ifndef _LED_H_INCLUDED
#define _LED_H_INCLUDED

#include <linux/config.h>
#include <asm/rt2880/rt_mmap.h>

#define LED_VERSION 		"v1.0"
#define LED_MAJOR       	240 // tommy-> modify it 166 -> 240
#define LED_DEV_NUM		32 
#define LED_GPIO_START      	1
#define GPIO_MAJOR       	241 // tommy-> modify it 167 -> 241
#define GPIO_DEV_NUM		32
#define REG_MINOR           	128
// sam 1-30-2004 for LAN_STATUS
#define LAN_STATUS_MAJOR   	242 // tommy-> modify it 168 -> 242
#define LAN_DEV_NUM     	5
// end sam

//#define GPIO_IO_BASE   	0xB4002480
//#define GPIO_IO_BASE      ((unsigned long)0xb20000b8)
//#define GPIO_IO_EXTENT	0x40
#ifdef _KREBOOT_
#define REBOOT       0x040000
#endif
#define LED_ON              0x010000
#define LED_OFF             0x020000
#define LED_BLINK_CMD       0x030000
#define LED_BLINK_PERIOD    0x00FFFF
#define LED_BLINK           (LED_BLINK_CMD|1000)
#define LED_BLINK_FAST      (LED_BLINK_CMD|250)
#define LED_BLINK_SLOW      (LED_BLINK_CMD|500)
#define LED_BLINK_EXTRA_SLOW    (LED_BLINK_CMD|2000)
#define LED_BLINK_RANDOM    (LED_BLINK_CMD|0xffff)
#define GPIO_SET_PID        0x050000





/*
 * ioctl commands
 */
#define	RALINK_GPIO_SET_DIR		0x01
#define RALINK_GPIO_SET_DIR_IN		0x11
#define RALINK_GPIO_SET_DIR_OUT		0x12
#define	RALINK_GPIO_READ		0x02
#define	RALINK_GPIO_WRITE		0x03
#define	RALINK_GPIO_SET			0x21
#define	RALINK_GPIO_CLEAR		0x31
#define	RALINK_GPIO_READ_BIT		0x04
#define	RALINK_GPIO_WRITE_BIT		0x05
#define	RALINK_GPIO_READ_BYTE		0x06
#define	RALINK_GPIO_WRITE_BYTE		0x07
#define	RALINK_GPIO_READ_INT		0x02 //same as read
#define	RALINK_GPIO_WRITE_INT		0x03 //same as write
#define	RALINK_GPIO_SET_INT		0x21 //same as set
#define	RALINK_GPIO_CLEAR_INT		0x31 //same as clear
#define RALINK_GPIO_ENABLE_INTP		0x08
#define RALINK_GPIO_DISABLE_INTP	0x09
#define RALINK_GPIO_REG_IRQ		0x0A
#define RALINK_GPIO_LED_SET		0x41

#define FLASH_MAX_RW_SIZE		0x100


/*
 * Address of RALINK_ Registers
 */
#define RALINK_SYSCTL_ADDR		RALINK_SYSCTL_BASE	// system control
#define RALINK_REG_GPIOMODE		(RALINK_SYSCTL_ADDR + 0x60)

#define RALINK_IRQ_ADDR			RALINK_INTCL_BASE
#define RALINK_REG_INTENA		(RALINK_IRQ_ADDR + 0x34)
#define RALINK_REG_INTDIS		(RALINK_IRQ_ADDR + 0x38)

#define RALINK_PRGIO_ADDR		RALINK_PIO_BASE // Programmable I/O
#define RALINK_REG_PIOINT		(RALINK_PRGIO_ADDR + 0)
#define RALINK_REG_PIOEDGE		(RALINK_PRGIO_ADDR + 0x04)
#define RALINK_REG_PIORENA		(RALINK_PRGIO_ADDR + 0x08)
#define RALINK_REG_PIOFENA		(RALINK_PRGIO_ADDR + 0x0C)
#define RALINK_REG_PIODATA		(RALINK_PRGIO_ADDR + 0x20)
#define RALINK_REG_PIODIR		(RALINK_PRGIO_ADDR + 0x24)
#define RALINK_REG_PIOSET		(RALINK_PRGIO_ADDR + 0x2C)
#define RALINK_REG_PIORESET		(RALINK_PRGIO_ADDR + 0x30)




/*
 * Values for the GPIOMODE Register
 */
#ifdef CONFIG_RALINK_RT2880
#define RALINK_GPIOMODE_I2C		0x01
#define RALINK_GPIOMODE_UARTF		0x02
#define RALINK_GPIOMODE_SPI		0x04
#define RALINK_GPIOMODE_UARTL		0x08
#define RALINK_GPIOMODE_JTAG		0x10
#define RALINK_GPIOMODE_MDIO		0x20
#define RALINK_GPIOMODE_SDRAM		0x40
#define RALINK_GPIOMODE_PCI		0x80
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT2883)
#define RALINK_GPIOMODE_I2C		0x01
#define RALINK_GPIOMODE_SPI		0x02
#define RALINK_GPIOMODE_UARTF		0x1C
#define RALINK_GPIOMODE_UARTL		0x20
#define RALINK_GPIOMODE_JTAG		0x40
#define RALINK_GPIOMODE_MDIO		0x80
#define RALINK_GPIOMODE_SDRAM		0x100
#define RALINK_GPIOMODE_RGMII		0x200
#endif

// if you would like to enable GPIO mode for other pins, please modify this value
// !! Warning: changing this value may make other features(MDIO, PCI, etc) lose efficacy
#define RALINK_GPIOMODE_DFT		(RALINK_GPIOMODE_UARTF)


/*
 * bit is the unit of length
 */
#define RALINK_GPIO_NUMBER		24
#define RALINK_GPIO_DATA_MASK		0x00FFFFFF
#define RALINK_GPIO_DATA_LEN		24
#define RALINK_GPIO_DIR_IN		0
#define RALINK_GPIO_DIR_OUT		1
#define RALINK_GPIO_DIR_ALLIN		0
#define RALINK_GPIO_DIR_ALLOUT		0x00FFFFFF

/*
 * structure used at regsitration
 */
typedef struct {
	unsigned int irq;		//request irq pin number
	pid_t pid;			//process id to notify
} ralink_gpio_reg_info;

#define RALINK_GPIO_LED_LOW_ACT		1
#define RALINK_GPIO_LED_INFINITY	4000


#define RALINK_GPIO_0			0x00000001
#define RALINK_GPIO_1			0x00000002
#define RALINK_GPIO_2			0x00000004
#define RALINK_GPIO_3			0x00000008
#define RALINK_GPIO_4			0x00000010
#define RALINK_GPIO_5			0x00000020
#define RALINK_GPIO_6			0x00000040
#define RALINK_GPIO_7			0x00000080
#define RALINK_GPIO_8			0x00000100
#define RALINK_GPIO_9			0x00000200
#define RALINK_GPIO_10			0x00000400
#define RALINK_GPIO_11			0x00000800
#define RALINK_GPIO_12			0x00001000
#define RALINK_GPIO_13			0x00002000
#define RALINK_GPIO_14			0x00004000
#define RALINK_GPIO_15			0x00008000
#define RALINK_GPIO_16			0x00010000
#define RALINK_GPIO_17			0x00020000
#define RALINK_GPIO_18			0x00040000
#define RALINK_GPIO_19			0x00080000
#define RALINK_GPIO_20			0x00100000
#define RALINK_GPIO_21			0x00200000
#define RALINK_GPIO_22			0x00400000
#define RALINK_GPIO_23			0x00800000
#define RALINK_GPIO(x)			(1 << x)


#if defined(_3G6230N_)
#define RT2880_GPIOMODE_SPI_UART1 0x0A
#define GPIO_POWER_LED			  0x00000200 //GPIO9 -> 3G-6230 Power LED
#define GPIO_Wireless_LED		  0x00000100 //GPIO8 -> 3G-6230 Wireless LED
#define GPIO_USB_LED			  0x00000080 //GPIO7 -> 3G-6230 USB LED
//#define GPIO_WPS_LED			  0x00000800 //GPIO11

#define GPIO_RESET_BUT			  0x00000001 //GPIO0 -> 3G-6230 RESET BUTTON
#define GPIO_ROUTER_SW			  0x00000020 //GPIO5 -> 3G-6230 ROUTER SWITCH
#define GPIO_AP_SW			  0x00000010 //GPIO4 -> 3G-6230 AP SWITCH
#define GPIO_CLIENT_SW			  0x00000008 //GPIO3 -> 3G-6230 CLIENT SWITCH
#define GPIO_USB_PWR			  0x00000400 //GPIO10-> 3G-6230 CLIENT SWITCH
#else
//Add by Kyle 2008.07.01 for RT3052
#define RT2880_GPIOMODE_SPI_UART1 0x0A
//#define RT2880GPIO_DIR			  0x00FFF3BF // set gpio 10 ,6 ,11  Input.
#define GPIO_POWER_LED			  0x00000200 //GPIO9
#define GPIO_Wireless_LED		  0x00004000 //GPIO14
#define GPIO_USB_LED			  0x00000080 //GPIO7
#define GPIO_WPS_LED			  0x00000800 //GPIO11

#define GPIO_RESET_BUT			  0x00001000 //GPIO12
#define GPIO_WPS_BUT			  0x00000001 //GPIO0
#define GPIO_APSWITCH_BUT		  0x00002000 //GPIO13
//End Kyle
#endif



#endif