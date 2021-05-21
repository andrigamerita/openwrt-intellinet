/*
 * LED interface for WP3200
 *
 * Copyright (C) 2002, by Allen Hung
 *
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/reboot.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/interrupt.h>

#include "led.h"

#define BUF_LEN		30
#define NAME		"ralink_gpio"
#define RALINK_GPIO_MAX_INFO (RALINK_GPIO_DATA_LEN)

irqreturn_t ralink_gpio_irq_handler(int irq, void *irqaction);

//add by kyle 2007/12/12 for gpio interrupt
struct irqaction ralink_gpio_irqaction = {
        .handler = ralink_gpio_irq_handler,
        .flags = SA_INTERRUPT,
        .mask = 0,
        .name = "ralink_gpio",
};


ralink_gpio_reg_info ralink_gpio_info[RALINK_GPIO_MAX_INFO];
ralink_gpio_reg_info info;
struct LED_DATA  {
    char sts_buf[BUF_LEN+1];
    unsigned long sts;
};
//add by kyle 2007/12/12 for gpio interrupt
int ralink_gpio_irqnum = 0;
pid_t ra_pid=0;
u32 ralink_gpio_intp = 0;
u32 ralink_gpio_edge = 0;

struct LED_DATA led_data[LED_DEV_NUM];



static struct timer_list blink_timer[LED_DEV_NUM];
// sam 01-30-2004 for watchdog
static struct timer_list watchdog;
// Kyle 20080806 for upgrade reboot.
static struct timer_list rebootTimer;
// end sam
static char cmd_buf[BUF_LEN+1];
extern void machine_restart(char *command);

// sam for ps 3205U -- using CSx1 (0xb0e00000)
// bit map as following
// BIT   1      2      3      4      5   
//     POWER  WLEN   PORT1  PORT2  PORT3
//
// value 0 --> led on
// value 1 --> led off

#define _ROUTER_

//volatile unsigned char* gpioA_pddr =(unsigned char*)(0xA0300000);
//volatile unsigned char* gpioA_pdcon=(unsigned char*)(0xA0300000);
//volatile unsigned char* gpioB_pddr =(unsigned char*)(0xA0300000);
//volatile unsigned char* gpioB_pdcon=(unsigned char*)(0xA0300000);

//volatile unsigned int * wdt_lr  = (unsigned int*)(0x1e8c0000+0xC1800000);
//volatile unsigned int * wdtg_cvr= (unsigned int*)(0x1e8c0004+0xC1800000);
//volatile unsigned int * wdt_con = (unsigned int*)(0x1e8c0008+0xC1800000);





// sam 1-30-2004 LED status 
// bit map as following
// BIT 4:0  Link status   -->PHY Link ->1 = up, 0 = down
#define LINK_STATUS     (*(unsigned long *)0xb2000014)
#define WATCHDOG_VAL    (*(unsigned long *)0xb20000c0)
#define WATCHDOG_PERIOD 500 // unit ms
#define EXPIRE_TIME     300 // unit 10 ms
#define CLEAR_TIMEER    0xffffa000l  // bit 14:0 -> count up timer, write 0 to clear
#define ENABLE_WATCHDOG 0x80000000l  // bit 31 -> 1 enable , 0 disable watchdog
#define WATCHDOG_SET_TMR_SHIFT 16    // bit 16:30 -> watchdog timer set
// end sam

 
//------------------------------------------------------------
static void turn_led(int id, int on)
{
	switch ( on ) {
		case 0:
			*(volatile u32 *)(RALINK_REG_PIODATA) |=  (1 << (id));
			break; // LED OFF
		case 1:
			*(volatile u32 *)(RALINK_REG_PIODATA) &= ~(1 << (id));
			break; // LED ON
		case 2:
			*(volatile u32 *)(RALINK_REG_PIODATA) ^=  (1 << (id));
			break; // LED inverse
	}
}


static int led_flash[30]={20,10,100,5,5,150,100,5,5,50,20,50,50,20,60,5,20,10,30,10,5,10,50,2,5,5,5,70,10,50};//Erwin
static unsigned int    wlan_counter;    //Erwin

static void blink_wrapper(unsigned long  id)
{
    u_long sts = led_data[id].sts;

    if ( (sts & LED_BLINK_CMD) == LED_BLINK_CMD )  {
	unsigned long period = sts & LED_BLINK_PERIOD;
	if(period == 0xffff)		// BLINK random
	{
		blink_timer[id].expires = jiffies + 3*led_flash[wlan_counter%30]*HZ/1000;
		wlan_counter++;
	}
	else
		blink_timer[id].expires = jiffies + (period * HZ / 1000);
	turn_led(id, 2);
	add_timer(&blink_timer[id]);
    }
    else if ( sts == LED_ON || sts == LED_OFF )
	turn_led(id, sts==LED_ON ? 1 : 0);
}
//------------------------------------------------------------
static void get_token_str(char *str, char token[][21], int token_num)
{
    int t, i;

    for ( t = 0 ; t < token_num ; t++ )  {
    	memset(token[t], 0, 21);
    	while ( *str == ' ' )  str++;
    	for ( i = 0 ; str[i] ; i++ )  {
    	    if ( str[i] == '\t' || str[i] == ' ' || str[i] == '\n' )  break;
    	    if ( i < 20 )  token[t][i] = str[i];
    	}
    	str += i;
    }
}

//------------------------------------------------------------
static void set_led_status_by_str(int id)
{
    char token[3][21], *p;

    get_token_str(led_data[id].sts_buf, token, 3);

	#ifdef _KREBOOT_
		//add by Kyle 2008.07.29 for Upgrade reboot function.
	    if ( !strcmp(token[0], "REBOOT") ) 
		{
			printk("Reboot System2 !!\n");
			#if defined(_REVERSE_BUTTON_)
				if(id==_HW_BUTTON_WPS_){
			#else
				if(id==_HW_BUTTON_RESET_){
			#endif
				machine_restart(0);
				}
			
		}


	#endif	
    if ( strcmp(token[0], "LED") ) 
	{
        goto set_led_off;
	}

    if ( !strcmp(token[1], "ON") )  
	{
		
    	turn_led(id, 1);
    	led_data[id].sts = LED_ON;
    }
    else if ( !strcmp(token[1], "OFF") )  
	{
		
	    goto set_led_off;
    }
    else if ( !strcmp(token[1], "BLINK") ) 
	{
    	unsigned int period = 0;
    	p = token[2];
    	if ( !strcmp(p, "FAST") )
    	    period = LED_BLINK_FAST & LED_BLINK_PERIOD;
    	else if ( !strcmp(p, "SLOW") )
    	    period = LED_BLINK_SLOW & LED_BLINK_PERIOD;
    	else if ( !strcmp(p, "EXTRA_SLOW") )
    	    period = LED_BLINK_EXTRA_SLOW & LED_BLINK_PERIOD;
    	else if ( !strcmp(p, "RANDOM") )
    	    period = LED_BLINK_RANDOM & LED_BLINK_PERIOD;
    	else if ( !strcmp(p, "OFF") )
	    goto set_led_off;
	else if ( *p >= '0' && *p <= '9' )  
	{
    		while ( *p >= '0' && *p <= '9' )
    	        period = period * 10 + (*p++) - '0';
//    		if ( period > 10000 )  
//			period = 10000;
	}
	else
    		period = LED_BLINK & LED_BLINK_PERIOD;
	
    	if ( period == 0 )
    	    goto set_led_off;
		
	sprintf(led_data[id].sts_buf, "LED BLINK %d\n", period);
    	led_data[id].sts = LED_BLINK_CMD + period;
    	turn_led(id, 2);
     // Set timer for next blink
	del_timer(&blink_timer[id]);
        blink_timer[id].function = blink_wrapper;
        blink_timer[id].data = id;
        init_timer(&blink_timer[id]);
        
	blink_timer[id].expires = jiffies + (1000 * HZ / 1000);
        
	add_timer(&blink_timer[id]);
    }
    else
	{
        goto set_led_off;
	}
    return;
  set_led_off:
    strcpy(led_data[id].sts_buf, "LED OFF\n");
    led_data[id].sts = LED_OFF;
    turn_led(id, 0);
  	 		
}

#if defined(_3G6230N_)
#define RT2882_REG(x)          (*((volatile u32 *)(x)))
static int read_usb_pwr_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RT2882_REG(RALINK_REG_PIODATA) & (1 << _HW_USB_PWR_)) {
		flag = '1';
	}
	else {
		flag = '0';
	}

	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int write_usb_pwr_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag[20];

	if (count < 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		if (flag[0] == '0') {
			*(volatile u32 *)(RALINK_REG_PIODATA) &= cpu_to_le32(~(1 << _HW_USB_PWR_)); // USB POWER OFF
		}
		else {
			*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_USB_PWR_); // USB POWER ON
		}

		return count;
	}
	return -EFAULT;
}

static int read_mode_sw_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;
	int mode;

	mode = (RT2882_REG(RALINK_REG_PIODATA) & (0x00000038));
	if (mode == 24)
		flag = '1'; // Router Mode
	else if (mode == 40)
		flag = '2'; // AP Mode
	else if (mode == 48)
		flag = '3'; // Client Mode
	else
		flag = 'E'; // unknow -> error

	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

#if 0
static int read_router_sw_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RT2882_REG(RALINK_REG_PIODATA) & (1 << _HW_ROUTER_SW_))
		flag = '0';
	else
		flag = '1';

	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int read_ap_sw_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RT2882_REG(RALINK_REG_PIODATA) & (1 << _HW_AP_SW_))
		flag = '0';
	else
		flag = '1';

	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int read_client_sw_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RT2882_REG(RALINK_REG_PIODATA) & (1 << _HW_CLIENT_SW_))
		flag = '0';
	else
		flag = '1';

	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif
#endif

//----------------------------------------------------------------------
static int led_read_proc(char *buf, char **start, off_t fpos, int length, int *eof, void *data)
{
    int len, dev;

    for ( len = dev = 0 ; dev < LED_DEV_NUM ; dev++ )  {
    	len += sprintf(buf+len, "%d: %s", dev, led_data[dev].sts_buf);
    }
    len = strlen(buf) - fpos;
    if ( len <= 0 ) {
	*start = buf;
	*eof = 1;
	return 0;
    }
    *start = buf + fpos;
    if ( len <= length )   *eof = 1;
    return len < length ? len : length;
}

static void reboot_wrapper(unsigned long period)
{
	machine_restart(0);
}

//add by kyle 2007/12/12 for gpio interrupt
static int gpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int id = (int)file->private_data;
		unsigned long  tmp;
 switch ( cmd )  {
	case RALINK_GPIO_REG_IRQ:
  	printk(KERN_ERR NAME ":ioctl GPIO REG\n");
		copy_from_user(&info, (ralink_gpio_reg_info *)arg, sizeof(info));
		if (0 <= info.irq && info.irq < RALINK_GPIO_MAX_INFO) {
			ralink_gpio_info[info.irq].pid = info.pid;
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
			tmp |= (0x1 << info.irq);
			*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
			tmp |= (0x1 << info.irq);
			*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
		}
		else
			printk(KERN_ERR NAME ": irq number(%d) out of range\n",
					info.irq);
		break;
	#ifdef _KREBOOT_
	//add by Kyle 2008.02.29 for Upgrade reboot function.
	case REBOOT:
    rebootTimer.function = reboot_wrapper;
    rebootTimer.data = 3000;
    init_timer(&rebootTimer);
    rebootTimer.expires = jiffies + (3000 * HZ / 1000);
    add_timer(&rebootTimer);
	printk("Add Reboot timer !!\n");
			//machine_restart(0);
		break;
	#endif		

    }
    return 0;
}

//----------------------------------------------------------------------
static int led_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int id = (int)file->private_data;
	switch ( cmd )  {
		case LED_ON:
			strcpy(led_data[id].sts_buf, "LED ON\n");
			set_led_status_by_str(id);
			break;
		case LED_OFF:
			strcpy(led_data[id].sts_buf, "LED OFF\n");
			set_led_status_by_str(id);
			break;
		default:
			if ( (cmd & LED_BLINK_CMD) != LED_BLINK_CMD )
			{
				break;
			}
		case LED_BLINK:
		case LED_BLINK_FAST:
		case LED_BLINK_SLOW:
		case LED_BLINK_EXTRA_SLOW:
		case LED_BLINK_RANDOM:
			sprintf(led_data[id].sts_buf, "LED BLINK %d\n", (int)(cmd & LED_BLINK_PERIOD));
			set_led_status_by_str(id);
			break;
	}
	return 0;
}

static int led_open(struct inode *inode, struct file *file)
{
    int led_id = MINOR(inode->i_rdev);
//    unsigned long led_bit = 1 << (led_id);

    if ( led_id >= LED_DEV_NUM )
        return -ENODEV;
/* sam 12/02/2003
    GPIO_SEL_I_O &= ~led_bit;   // 0 to GPIO
    GPIO_O_EN |= (led_bit << 16);   // 0 to Output
*/	
	
    file->private_data = (void*)led_id;
    return 0;
}

static ssize_t led_read(struct file *file, char *buf, size_t count, loff_t *fpos)
{
    int  rem, len;
    int  id = (int)file->private_data;
    char *p = led_data[id].sts_buf;

    len = strlen(p);
    rem = len - *fpos;
    if ( rem <= 0 )  {
    	*fpos = len;
    	return 0;
    }
    if ( rem > count )   rem = count;
    memcpy(buf, p+(*fpos), rem);
    *fpos += rem;
    return rem;
}

//static ssize_t led_write(struct file *file, char *buf, size_t count, loff_t *fpos)
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *fpos)
{
    int  len;
    int  id = (int)file->private_data;
    char *p = id == REG_MINOR ? cmd_buf : led_data[id].sts_buf;
    memset(p, 0, BUF_LEN);

    p += *fpos;
    len = 0;

	
    while ( count > 0 )  
	{
		
    	if ( *fpos < BUF_LEN )  
		{
    	    int c = *buf++;
            p[len] = c>='a' && c<='z' ? c-'a'+'A' : c;
        }
    	(*fpos)++;
	    len++;
    	count--;
    }
	// sam
    set_led_status_by_str(id);
	(*fpos) = 0;
	//
	
    return len;
}

static int led_flush(struct file *file)
{
    int  id = (int)file->private_data;

    if ( file->f_mode & FMODE_WRITE )
	{
    	set_led_status_by_str(id);
	}
    return 0;
}

static struct file_operations led_fops = {
    .read  = led_read,
    .write = led_write,
    .flush = led_flush,
    .ioctl = led_ioctl,
    .open =  led_open,
};

//----------------------------------------------

static int dump_content(char *buf)
{
    return 0;
}

static ssize_t gpio_read(struct file *file, char *buf, size_t count, loff_t *fpos)
{
    int  rem, len;
    int  id = (int)file->private_data;
    char temp[80*10];
    unsigned long gpio_regval =0;

	gpio_regval = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));					
	
#if defined(_REVERSE_BUTTON_)
	if(id==_HW_BUTTON_WPS_)
		id=_HW_BUTTON_RESET_;
	else if(id==_HW_BUTTON_RESET_)
		id=_HW_BUTTON_WPS_;
#endif
	
    if ( id < GPIO_DEV_NUM )  {
        int  gpio_bit = 1 << id;

	switch(id) {
#ifndef _3G6230N_
		case _HW_BUTTON_SWITCH_://ap router switch
			#if defined(_ARSWITCH_) || defined(_WIRELESS_SWITCH_)
				len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 0);
			#else
				len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 1);
			#endif
		
		break;
#endif
		case _HW_BUTTON_WPS_://wps
			#if defined(_WPS_INDEPEND_) || defined(_REVERSE_BUTTON_)
				len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 0);
			#else
				len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 1);
			#endif
		break;

		case _HW_BUTTON_RESET_://reset
			#if defined(_REVERSE_BUTTON_)
				#if defined(_WPS_INDEPEND_) 
					len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 0);
				#else
					len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 1);
				#endif
			#else
				len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 0);
			#endif

		break;	
			
		default:
			len = sprintf(temp, "%d\n", (gpio_regval & gpio_bit) ? 1 : 0);
		break;
	}


    }
    else   // REG device
        len = dump_content(temp);
    rem = len - *fpos;
    if ( rem <= 0 )  {
    	*fpos = len;
    	return 0;
    }
    if ( rem > count )   rem = count;
    memcpy(buf, temp+(*fpos), rem);
    *fpos += rem;
    return rem;
}

static int gpio_flush(struct file *file)
{
    return 0;
}

static int gpio_open(struct inode *inode, struct file *file)
{
    int id = MINOR(inode->i_rdev);
    if ( id >= GPIO_DEV_NUM && id != REG_MINOR )
        return -ENODEV;
    file->private_data = (void*)id;
    return 0;
}

static struct file_operations gpio_fops = {
    .read  = gpio_read,
    .open  = gpio_open,
    .flush = gpio_flush,
    .ioctl = gpio_ioctl,
    .write = led_write,
};

//----------------------------------------------


//----------------------------------------------
static int init_status;

#define INIT_REGION	        0x01
#define INIT_LED_REGISTER	0x02
#define INIT_LED_PROC_READ	0x04
#define INIT_GPIO_REGISTER	0x08
#define INIT_LAN_STATUS_REGISTER 0x10
#define INIT_WATCHDOG_REGISTER 0x20

static void led_exit(void)
{
    int id;
    for ( id = 0 ; id < LED_DEV_NUM ; id++ )  {
        del_timer(&blink_timer[id]);
        turn_led(id, 1);
    }
    if ( init_status & INIT_LED_PROC_READ )
    	remove_proc_entry("driver/led", NULL);
    	
    if ( init_status & INIT_LED_REGISTER )
    	unregister_chrdev(LED_MAJOR, "led");

    if ( init_status & INIT_GPIO_REGISTER )
    	unregister_chrdev(GPIO_MAJOR, "gpio");
    
    if( init_status & INIT_WATCHDOG_REGISTER)
       del_timer(&watchdog);
}

static int __init led_init(void)
{
	//Kyle init gpio ping for rt3052. 2008/07/24
	printk("***********Init LED Driver*****************\n");	
	printk("Init GPIO Pin\n");
	//config these pins to gpio mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = (*(volatile u32 *)(RALINK_REG_GPIOMODE) | cpu_to_le32(RALINK_GPIOMODE_UARTF));
	
	printk("Init GPIO Direction\n");
#if defined(_3G6230N_)
	//config goio Direction
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_BUTTON_RESET_)); // INput mode(Reset)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_BUTTON_WPS_)); // INput mode(WPS)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_ROUTER_SW_)); // INput mode(Router SW)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_AP_SW_)); // INput mode(AP SW)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_CLIENT_SW_)); // INput mode(Client SW)

	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_POWER_); // Output mode(Power Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_WIRELESS_); // Output mode(Wireless Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_USB_); // Output mode(USB Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_USB_PWR_); // Output mode(USB PWR Control)


	*(volatile u32 *)(RALINK_REG_PIODATA) &= cpu_to_le32(~(1 << _HW_LED_POWER_)); // Power Led ON
	*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_LED_WIRELESS_); // Wireless Led OFF
	*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_LED_USB_); // USB Led OFF

	//*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_USB_PWR_); // USB POWER ON
	*(volatile u32 *)(RALINK_REG_PIODATA) &= cpu_to_le32(~(1 << _HW_USB_PWR_)); // USB POWER OFF

	struct proc_dir_entry *res=NULL;
	res = create_proc_entry("USB_PWR", 0, NULL);
	if (res) {
		res->read_proc = read_usb_pwr_proc;
		res->write_proc = write_usb_pwr_proc;
	}
	else {
		printk("Ralink GPIO Driver, create USB_PWR failed!\n");
	}

	res = create_proc_entry("MODE_SW", 0, NULL);
	if (res) {
		res->read_proc = read_mode_sw_proc;
		res->write_proc = NULL;
	}
	else {
		printk("Ralink GPIO Driver, create MODE_SW failed!\n");
	}
#if 0
	res = create_proc_entry("ROUTER_MODE", 0, NULL);
	if (res) {
		res->read_proc = read_router_sw_proc;
		res->write_proc = NULL;
	}
	else {
		printk("Ralink GPIO Driver, create MODE_SW failed!\n");
	}

	res = create_proc_entry("AP_MODE", 0, NULL);
	if (res) {
		res->read_proc = read_ap_sw_proc;
		res->write_proc = NULL;
	}
	else {
		printk("Ralink GPIO Driver, create MODE_SW failed!\n");
	}

	res = create_proc_entry("CLIENT_MODE", 0, NULL);
	if (res) {
		res->read_proc = read_client_sw_proc;
		res->write_proc = NULL;
	}
	else {
		printk("Ralink GPIO Driver, create MODE_SW failed!\n");
	}
#endif

#else
	//config goio Direction
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_BUTTON_RESET_));//INput mode(Reset)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_BUTTON_WPS_));//INput mode(WPS)
	*(volatile u32 *)(RALINK_REG_PIODIR) &= cpu_to_le32(~(1<< _HW_BUTTON_SWITCH_));//INput mode(AP_Mode)
		
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_POWER_);//Output mode(Power Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_WIRELESS_);//Output mode(Wireless Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_USB_);//Output mode(USB Led)
	*(volatile u32 *)(RALINK_REG_PIODIR) |= cpu_to_le32(1<< _HW_LED_WPS_);//Output mode(WPS Led)


	*(volatile u32 *)(RALINK_REG_PIODATA) &= cpu_to_le32(~(1 << _HW_LED_POWER_));//Power Led ON
	*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_LED_WIRELESS_);//Power Wireless OFF
	*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_LED_USB_);//USB Led OFF
	*(volatile u32 *)(RALINK_REG_PIODATA) |= cpu_to_le32(1 << _HW_LED_WPS_);//WPS Led OFF
	*(volatile u32 *)(RALINK_REG_PIODATA) |=  (1 << (7));
	*(volatile u32 *)(RALINK_REG_PIODATA) |=  (1 << (11));
#endif

	printk("Init GPIO Interrupt\n");	
	int i=0;
	//enable gpio interrupt
	*(volatile u32 *)(RALINK_REG_INTENA) = cpu_to_le32(RALINK_INTCTL_PIO);
	for (i = 0; i < RALINK_GPIO_NUMBER; i++) {
		ralink_gpio_info[i].irq = i;
		ralink_gpio_info[i].pid = 0;
	}
	
	
	printk("***********Init LED Driver Finishing*****************\n");	 


		


    int result, id;
    init_status = 0;
	
  //----- register device (LED)-------------------------
  
    result = register_chrdev(LED_MAJOR, "led", &led_fops);
    if ( result < 0 )   {
    	printk(KERN_ERR "led: can't register char device\n" );
    	led_exit();
    	return result;
    }
    
	init_status |= INIT_LED_REGISTER;
  //----- register device (GPIO)-------------------------
    result = register_chrdev(GPIO_MAJOR, "gpio", &gpio_fops);
    if ( result < 0 )   {
    	printk(KERN_ERR "gpio: can't register char device\n" );
    	led_exit();
    	return result;
    }
    init_status |= INIT_GPIO_REGISTER;
    

//------ read proc -------------------
    if ( !create_proc_read_entry("driver/led", 0, 0, led_read_proc, NULL) )  {
	printk(KERN_ERR "led: can't create /proc/driver/led\n");
    	led_exit();
    	return -ENOMEM;
    }
    init_status |= INIT_LED_PROC_READ;
//------------------------------
    
    for ( id = 0 ; id < LED_DEV_NUM ; id++ )  {
    	strcpy(led_data[id].sts_buf, "LED ON\n" );
    	set_led_status_by_str(id);
    }

    printk("Ralink_gpio_init_irq \n");
    setup_irq(SURFBOARDINT_GPIO, &ralink_gpio_irqaction);
   
    printk(KERN_INFO "LED & GPIO & LAN Status Driver LED_VERSION \n");
    return 0;
}


//add by kyle 2007/12/12 for gpio interrupt
void ralink_gpio_notify_user(int usr)
{
	struct task_struct *p = NULL;

	if (ralink_gpio_irqnum < 0 || RALINK_GPIO_MAX_INFO <= ralink_gpio_irqnum) {
		printk(KERN_ERR NAME ": gpio irq number out of range\n");
		return;
	}

	//don't send any signal if pid is 0 or 1
	if ((int)ralink_gpio_info[ralink_gpio_irqnum].pid < 2)
		return;
	p = find_task_by_pid(ralink_gpio_info[ralink_gpio_irqnum].pid);
	if (NULL == p) {
		printk(KERN_ERR NAME ": no registered process to notify\n");
		return;
	}

	if ( usr == 1 ) {
		printk(KERN_NOTICE NAME ": sending a SIGUSR1 to process %d\n",
				ralink_gpio_info[ralink_gpio_irqnum].pid);
		send_sig(SIGUSR1, p, 0);
	}
	else if (usr == 2) {
		printk(KERN_NOTICE NAME ": sending a SIGUSR2 to process %d\n",
				ralink_gpio_info[ralink_gpio_irqnum].pid);
		send_sig(SIGUSR2, p, 0);
	}//for wireless H/W switch function
#if defined(_WIRELESS_SWITCH_) 
	else if ( usr == 3 ) {
		printk(KERN_NOTICE NAME ": sending a SIGIO to process %d\n",
				ralink_gpio_info[ralink_gpio_irqnum].pid);
		send_sig(SIGIO, p, 0);
		
	}
#endif
}
//add by kyle 2007/12/12 for gpio interrupt
/*
 * 1. save the PIOINT and PIOEDGE value
 * 2. clear PIOINT by writing 1
 * (called by interrupt handler)
 */
void ralink_gpio_save_clear_intp(void)
{
	ralink_gpio_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOINT));
	ralink_gpio_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
	*(volatile u32 *)(RALINK_REG_PIOINT) = cpu_to_le32(0x00FFFFFF);
	*(volatile u32 *)(RALINK_REG_PIOEDGE) = cpu_to_le32(0x00FFFFFF);
}

//add by kyle 2007/12/12 for gpio interrupt
irqreturn_t ralink_gpio_irq_handler(int irq, void *irqaction)
{
	extern unsigned long volatile jiffies;
	struct gpio_time_record {
		unsigned long falling;
		unsigned long rising;
	};
	static struct gpio_time_record record[RALINK_GPIO_MAX_INFO];
	unsigned long now;
	int i;

	int resetButton=_HW_BUTTON_RESET_;
	int wpsButton=_HW_BUTTON_WPS_;
#ifndef _3G6230N_
	int switchButton1=_HW_BUTTON_SWITCH_;
#endif
	#if defined (_REVERSE_BUTTON_)
		resetButton=_HW_BUTTON_WPS_;
		wpsButton=_HW_BUTTON_RESET_;
	#endif

	ralink_gpio_save_clear_intp();
	now = jiffies;
	
	for (i = 0; i < RALINK_GPIO_MAX_INFO; i++) {
		if (! (ralink_gpio_intp & (1 << i)))
			continue;
		ralink_gpio_irqnum = i;
		if (ralink_gpio_edge & (1 << i)) { //rising edge
			if (record[i].rising != 0 && time_before_eq(now,
						record[i].rising + 30L)) {
				/*
				 * If the interrupt comes in a short period,
				 * it might be floating. We ignore it.
				 */
			}
			else {
				record[i].rising = now;
#ifndef _3G6230N_
				#if defined(_WIRELESS_SWITCH_)
					if (i == _HW_BUTTON_SWITCH_){
						ralink_gpio_notify_user(3);
						printk(KERN_NOTICE NAME ":Wireless Switch!\n");
					}
				#endif
#endif

		// #if defined(_corega_) 
		 		//2~5 Second 
		//		if (time_before(now, record[i].falling + 530L) && !time_before(now,record[i].falling+ 170L)) {
		 //#else 
		 		if (time_before(now, record[i].falling + 500L)) {
		 //#endif 
		 		#if !defined(_COREGA_WPS_TRIGGER_CONDITION_) || !defined(_HW_MACFILTER_IPHONE_)
						//one click
					#if defined(_WPS_INDEPEND_)
					    if(i==wpsButton){
							ralink_gpio_notify_user(1);
							printk(KERN_NOTICE NAME ":WPS Function!\n");
						}
					#else
					    if(i==resetButton){
							ralink_gpio_notify_user(1);
							printk(KERN_NOTICE NAME ":WPS Function!\n");
						}
					#endif		
				#endif 
				}
				else {
					//press for several seconds
					printk(KERN_NOTICE NAME ":press for 5 seconds!\n");
					}
			}
		}
		else { //falling edge
			if (record[i].falling != 0 && time_before_eq(now,
						record[i].falling + 30L)) {
				/*
				 * If the interrupt comes in a short period,
				 * it might be floating. We ignore it.
				 */
			}else{
				record[i].falling = now;

				if(i==resetButton){
				 	ralink_gpio_notify_user(2);
				#if defined(_COREGA_WPS_TRIGGER_CONDITION_)
					ralink_gpio_notify_user(1);
				#endif
				}
#ifndef _3G6230N_
			#if defined(_WIRELESS_SWITCH_) 
				else if (i == switchButton1){
					ralink_gpio_notify_user(3);
				}
			#endif
#endif

			#if defined(_HW_MACFILTER_IPHONE_) 
				if(i==wpsButton){
					ralink_gpio_notify_user(1);
					printk(KERN_NOTICE NAME ":WPS Function!\n");
				}
			#endif
				
			}
				
		}

		break;
	}

	return IRQ_HANDLED;

}

#if 0 // RexHua move to module_init
void __init ralink_gpio_init_irq(void)
{
	printk("\n\n\n ralink_gpio_init_irq \n\n\n");
	setup_irq(SURFBOARDINT_GPIO, &ralink_gpio_irqaction);
}
#endif
module_init(led_init);
module_exit(led_exit);

