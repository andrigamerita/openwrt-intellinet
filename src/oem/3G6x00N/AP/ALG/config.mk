include ../../define/PATH

CC := /opt/buildroot-gdb/bin/mipsel-linux-uclibc-gcc
LD := /opt/buildroot-gdb/bin/mipsel-linux-uclibc-ld

LINUX_SRC = $(LINUXDIR)

WFLAGS := -DAGGREGATION_SUPPORT -DWMM_SUPPORT  -DLINUX -Wall -Wstrict-prototypes -Wno-trigraphs 

WFLAGS += -DRALINK_ATE -DCONFIG_AP_SUPPORT  -DUAPSD_AP_SUPPORT -DDBG

#WFLAGS += -DCONFIG_5VT_ENHANCE

#kernel build options for 2.4
# move to Makefile outside LINUX_SRC := /opt/star/kernel/linux-2.4.27-star

#CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(RT2860_DIR)/include -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm926ej-s --param max-inline-insns-single=40000  -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -DMODULE $(WFLAGS) 

CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include  -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -D_BR6425N_ -D_General_ -D_FLASH_RESERVED_=66 -D_KREBOOT_ -D_PATD_ -D_WPS_INDEPEND_ -D_HW_LED_POWER_=9 -D_HW_LED_WIRELESS_=14 -D_HW_LED_USB_=7 -D_HW_LED_WPS_=11 -D_HW_BUTTON_RESET_=12 -D_HW_BUTTON_WPS_=0 -D_HW_BUTTON_SWITCH_=13 -D_IS_GATEWAY_ -D_NETBIOSNAME_ -D_MAX_RS_PASS_LEN_=65 -Os  -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding  -march=mips32 -Wa,-mips32 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/rt2880 -I$(LINUX_SRC)/include/asm-mips/mach-generic -fomit-frame-pointer -gdwarf-2 -DMODULE 

export CFLAGS

