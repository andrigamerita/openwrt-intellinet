/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Interrupt routines for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>

#include <asm/irq.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/gdb-stub.h>

#include <asm/rt2880/eureka_ep430.h>

#define shutdown_surfboard_irq	disable_surfboard_irq
#define mask_and_ack_surfboard_irq disable_surfboard_irq


extern volatile unsigned int dma1_intstat;
extern volatile unsigned int dma1_rawstat;
extern volatile unsigned int surfpcmcia_intstat;
extern volatile unsigned int surfpcmcia_rawstat;

extern asmlinkage void mipsIRQ(void);
void mips_timer_interrupt(void);
void __init ralink_gpio_init_irq(void);

#ifdef CONFIG_KGDB
extern void breakpoint(void);
extern int remote_debug;
#endif


struct surfboard_ictrl_regs *surfboard_hw0_icregs
	= (struct surfboard_ictrl_regs *)RALINK_INTCL_BASE;

static unsigned char pci_order = 0;

#if 0
#define DEBUG_INT(x...) printk(x)
#else
#define DEBUG_INT(x...)
#endif

void disable_surfboard_irq(unsigned int irq_nr)
{
	//printk("%s(): irq_nr = %d\n",__FUNCTION__,  irq_nr);
	if(irq_nr >5){
		surfboard_hw0_icregs->intDisable = (1 << irq_nr);
	}
}

void enable_surfboard_irq(unsigned int irq_nr)
{
	//printk("%s(): irq_nr = %d\n",__FUNCTION__,  irq_nr);
	if(irq_nr >5)
	surfboard_hw0_icregs->intEnable = (1 << irq_nr);
}

static unsigned int startup_surfboard_irq(unsigned int irq)
{
	enable_surfboard_irq(irq);
	return 0; /* never anything pending */
}


static void end_surfboard_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		enable_surfboard_irq(irq);
}
#if 0
static void rt2880_irq_handler(unsigned int irq)
{
	return;
}
#endif

static void __inline__ disable_all(void)
{
	unsigned long int_status;
	int_status = read_32bit_cp0_register(CP0_STATUS);
	int_status &= 0xfffffffe;
	write_32bit_cp0_register(CP0_STATUS, int_status);
}

static void __inline__ enable_all(void)
{
	unsigned long int_status;
	int_status = read_32bit_cp0_register(CP0_STATUS);
	int_status |= 0x1;
	write_32bit_cp0_register(CP0_STATUS, int_status);
}

#if 0
static void enable_rt2880_irq(unsigned int irq)
{
	unsigned long int_status;
	int_status = read_32bit_cp0_register(CP0_STATUS);

	if ( irq == 3)
		int_status = int_status | CAUSEF_IP5;	
	write_32bit_cp0_register(CP0_STATUS, int_status);
}

static void disable_rt2880_irq(unsigned int irq)
{
	unsigned long int_status;
	int_status = read_32bit_cp0_register(CP0_STATUS);

	if ( irq == 3)
		int_status = int_status & ~(CAUSEF_IP5);	
	write_32bit_cp0_register(CP0_STATUS, int_status);
}
#endif

#if 1
#define	startup_rt2880_irq	enable_rt2880_irq
#define	shutdown_rt2880_irq	disable_rt2880_irq
#else
#define	startup_rt2880_irq 	rt2880_irq_handler	
#define	shutdown_rt2880_irq	rt2880_irq_handler
#endif

#define	enable_rt2880_irq	rt2880_irq_handler
#define	disable_rt2880_irq	rt2880_irq_handler
#define	mask_and_ack_rt2880_irq	rt2880_irq_handler
#define	end_rt2880_irq		rt2880_irq_handler

#if 0
static struct hw_interrupt_type rt2880_irq_type = {
	"RT2880",
	startup_rt2880_irq,
	shutdown_rt2880_irq,
	enable_rt2880_irq,
	disable_rt2880_irq,
	mask_and_ack_rt2880_irq,
	end_rt2880_irq,
	NULL
};
#endif

static struct hw_interrupt_type surfboard_irq_type = {
  .typename = "Surfboard",
  .startup  = startup_surfboard_irq,
  .shutdown = shutdown_surfboard_irq,
  .enable   = enable_surfboard_irq,
  .disable  = disable_surfboard_irq,
  .ack      = mask_and_ack_surfboard_irq,
  .end      = end_surfboard_irq,
};

static inline int ls1bit32(unsigned int x)
{
	int b = 31, s;

	s = 16; if (x << 16 == 0) s = 0; b -= s; x <<= s;
	s =  8; if (x <<  8 == 0) s = 0; b -= s; x <<= s;
	s =  4; if (x <<  4 == 0) s = 0; b -= s; x <<= s;
	s =  2; if (x <<  2 == 0) s = 0; b -= s; x <<= s;
	s =  1; if (x <<  1 == 0) s = 0; b -= s;

	return b;
}

void surfboard_hw0_irqdispatch(void)
{
	struct irqaction *action;
	unsigned long int_status;
	int irq;

	int_status = surfboard_hw0_icregs->irq0Status; 
	/* if int_status == 0, then the interrupt has already been cleared */
	if (int_status == 0)
		return;
	irq = ls1bit32(int_status);

	/*
	 * RT2880:
	 * bit[3] PIO Programmable IO Interrupt Status after Mask
	 * bit[2] UART Interrupt Status after Mask
	 * bit[1] WDTIMER Timer 1 Interrupt Status after Mask
	 * bit[0] TIMER0 Timer 0 Interrupt Status after Mask
	 *
	 * RT2883/RT3052:
	 * bit[6] PIO Programmable IO Interrupt Status after Mask
	 * bit[5] UART Interrupt Status after Mask
	 * bit[2] WDTIMER Timer 1 Interrupt Status after Mask
	 * bit[1] TIMER0 Timer 0 Interrupt Status after Mask
	 */
#ifdef CONFIG_RALINK_TIMER
#if defined (CONFIG_RALINK_RT2880_SHUTTLE) || \
    defined (CONFIG_RALINK_RT2880_MP)
	if (irq == 0) {
#else
	if (irq == 1) {
#endif
		irq = SURFBOARDINT_TIMER0;
	}
#endif

#if defined (CONFIG_RALINK_RT2880_SHUTTLE) ||   \
    defined (CONFIG_RALINK_RT2880_MP)
	if (irq == 3) {
#ifdef CONFIG_RALINK_GPIO 
		/* cause gpio registered irq 7 (see rt2880gpio_init_irq()) */
		irq = SURFBOARDINT_GPIO;
		printk("surfboard_hw0_irqdispatch(): INT #7...\n");
#else
		printk("surfboard_hw0_irqdispatch(): External INT #3... surfboard discard!\n");
#endif
	}
#else
	/* ILL_ACC */ 
	if (irq == 3) {
		irq = SURFBOARDINT_ILL_ACC;
	}
#endif
#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
	/* PCM */ 
	if (irq == 4) {
		irq = SURFBOARDINT_PCM;
	}
#endif
	/* UARTF */ 
	if (irq == 5) {
		irq = SURFBOARDINT_UART;
	}
	   /* ESW */

	   if (irq == 17) {

	       irq = SURFBOARDINT_ESW;

	   }
	action = irq_desc[irq].action;
	do_IRQ(irq);
	return;
}


static void enable_rt2880_cp_int(unsigned int IP_X)
{
	unsigned long int_status;

	int_status = read_32bit_cp0_register(CP0_STATUS);
	int_status = int_status | IP_X ;
	write_32bit_cp0_register(CP0_STATUS, int_status);
}

static void disable_rt2880_cp_int(unsigned int IP_X)
{
	unsigned long int_status;
	int_status = read_32bit_cp0_register(CP0_STATUS);
	int_status = int_status & ~(IP_X);
	write_32bit_cp0_register(CP0_STATUS, int_status);
}

void __init arch_init_irq(void)
{
	int i;

	/*
	 * Mask out all interrupt by writing "1" to all bit position in
	 * the interrupt reset reg.
	 */
#if 1
	int mips_cp0_cause, mips_cp0_status;
        mips_cp0_cause = read_32bit_cp0_register(CP0_CAUSE);
        mips_cp0_status = read_32bit_cp0_register(CP0_STATUS);
        printk("cause = %x, status = %x\n", mips_cp0_cause, mips_cp0_status);
        mips_cp0_status= mips_cp0_status& ~(CAUSEF_IP0|CAUSEF_IP1|CAUSEF_IP2|CAUSEF_IP3|CAUSEF_IP4|CAUSEF_IP5|CAUSEF_IP6|CAUSEF_IP7);
        write_32bit_cp0_register(CP0_STATUS, mips_cp0_status);
#endif
	
	memset(irq_desc, 0, sizeof(irq_desc));

	for (i = 0; i <= SURFBOARDINT_END; i++) {
		set_irq_chip(i, &surfboard_irq_type);
	}

	/* Enable global interrupt bit */
//	surfboard_hw0_icregs->intDisable = 0xffffffff;
	surfboard_hw0_icregs->intEnable = M_SURFBOARD_GLOBAL_INT;

#ifdef CONFIG_RALINK_GPIO
	ralink_gpio_init_irq();
#endif

#ifdef CONFIG_KGDB
	if (remote_debug) {
		set_debug_traps();
		breakpoint();
	}
#endif
}

void rt2880_irqdispatch(void)
{
#if 1
	unsigned long mips_cp0_status, mips_cp0_cause, irq_x, irq, i, pci_status=0;

	mips_cp0_cause = read_32bit_cp0_register(CP0_CAUSE);
	mips_cp0_status = read_32bit_cp0_register(CP0_STATUS);
	irq_x = mips_cp0_cause & mips_cp0_status & 0xfc00;
	irq_x >>= 10; //start from HW_INT#0
	/* from low to high priority */
	/*
	   irq = 0;
	   for (i = 0; i< 6; i++) {
	   if(irq_x & 0x1)
	   {
	   if(irq != 0)
	   do_IRQ(irq, regs);
	   else
	   surfboard_hw0_irqdispatch(regs);
	   }
	   irq++;
	   irq_x >>= 1;
	   }
	   */
	/* from high to low priority */
	irq = 5;
	pci_order^=1;

#ifdef CONFIG_RAETH_ROUTER
//	if(irq_x & 0x08)
//		do_IRQ(irq, regs);
#else
	//if(irq_x & 0x10)
	//	do_IRQ(irq, regs);
#endif

	for (i = 0; i< 6; i++) {
		if(irq_x & 0x20)
		{
			disable_rt2880_cp_int(1<<(irq+10));
			if(irq > 2)
				do_IRQ(irq);
			else if(irq == 2){
			 //disable_rt2880_cp_int(CAUSEF_IP4);
/*FIXME*/
#if defined(CONFIG_RT2880_FPGA) || defined(CONFIG_RT2880_ASIC)
			 pci_status = RALINK_PCI_PCIINT_ADDR;
#endif
			 if(pci_order ==0) { 
#if defined(CONFIG_RT2880_ASIC) || \
	defined (CONFIG_RT2883_ASIC) || \
	defined (CONFIG_RT3052_ASIC) 
				if(pci_status &0x40000)
#elif defined(CONFIG_RT2880_FPGA) || \
	defined (CONFIG_RT2883_FPGA) || \
	defined(CONFIG_RT3052_FPGA)
				if(pci_status &0x80000)
#endif
					do_IRQ(2);
				else // if(pci_status & 0x40000)
					do_IRQ(15);
			 } else {
#if defined(CONFIG_RT2880_ASIC)  || \
	defined (CONFIG_RT2883_ASIC) || \
	defined (CONFIG_RT3052_ASIC)
				if(pci_status &0x80000)
#elif defined(CONFIG_RT2880_FPGA) || \
	defined (CONFIG_RT2883_FPGA) || \
	defined(CONFIG_RT3052_FPGA)
				if(pci_status &0x40000)
#endif
					do_IRQ(15);
				else // if(pci_status & 0x80000)
					do_IRQ(2);
			 }

			 //enable_rt2880_cp_int(CAUSEF_IP4);
			}
			else {
				surfboard_hw0_irqdispatch();
			}
			enable_rt2880_cp_int(1<<(irq+10));
		}
		irq--;
		irq_x <<= 1;
	}
#else
	unsigned long int_status, int_cause;
	u32 ill_acc_val;
	
	int_cause = read_32bit_cp0_register(CP0_CAUSE);
	ill_acc_val = RALINK_SDRAM_ILL_ACC_ADDR;
			
	if(ill_acc_val != 0)
	{
		printk("\n RALINK_SDRAM_ILL_ACC_ADDR=[0x%08X]\n",ill_acc_val);
		ill_acc_val = RALINK_SDRAM_ILL_ACC_TYPE;
		RALINK_SDRAM_ILL_ACC_ADDR = 0xFFFFFFFF;
		printk("\n RALINK_SDRAM_ILL_ACC_TYPE=[0x%08X]\n",ill_acc_val);
			
		//surfboard_hw0_icregs->irq0Status = 0;
		//surfboard_hw0_icregs->irq1Status = 0;
		return;
				
	}

	if ((int_cause & CAUSEF_IP2))
	{
		
		surfboard_hw0_irqdispatch();	
	}	
	else
	{

		if(( int_cause & CAUSEF_IP5 ))
			do_IRQ(3); /* dispatch gmac irq here */	
		else if(( int_cause & CAUSEF_IP4 ))
		{//PCI slot
			int_status = RALINK_PCI_PCIRAW_ADDR;
			RALINK_PCI_PCIRAW_ADDR = 0xFFFFFFFF;

			if(RALINK_PCI_PCIRAW_FAIL_STATUS & int_status)
			{
				printk("\n PCI fail status occur!! Status=[0x%08X] \n",int_status);
				return;
			}
			
			if((int_status & 0x000C0000))
				do_IRQ(2); /* dispatch gmac irq here */		
		}
	}	
#endif
	
	return;
}
asmlinkage void plat_irq_dispatch(void)
{
        unsigned int pending = read_c0_status() & read_c0_cause() & ST0_IM;
        if (pending & CAUSEF_IP7)
                mips_timer_interrupt();
	else
		rt2880_irqdispatch();
}
