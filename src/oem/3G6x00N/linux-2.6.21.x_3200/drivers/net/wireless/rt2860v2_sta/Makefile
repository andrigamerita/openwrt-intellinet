EXTRA_CFLAGS = -Idrivers/net/wireless/rt2860v2_sta_dpb/include

obj-$(CONFIG_RT2860V2_STA) += rt2860v2_sta.o

rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/md5.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/mlme.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/rtmp_wep.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/action.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/ba_action.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_data.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/rtmp_init.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/rtmp_tkip.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_sync.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_sanity.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_info.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_wpa.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/2860_rtmp_init.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_data_2860.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/dfs.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/dfs_mcu.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/spectrum.o


rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/assoc.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/aironet.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/auth.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/auth_rsp.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/sync.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/sanity.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/rtmp_data.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/connect.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/wpa.o


rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_linux.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_profile.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_main_dev.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/sta_ioctl.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_2860_rbus.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_eeprom.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/2860_main_dev.o
rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/os/linux/rt_ate.o
#rt2860v2_sta-objs +=  ../rt2860v2_sta_dpb/common/netif_block.o


###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
	       -Wall -Wstrict-prototypes -Wno-trigraphs 

EXTRA_CFLAGS += -DCONFIG_STA_SUPPORT -DDBG -DRALINK_ATE -DRALINK_28xx_QA -DRT2860 #-DBLOCK_NET_IF


ifeq ($(CONFIG_RT2860V2_STA_WPA_SUPPLICANT),y)
EXTRA_CFLAGS += -DWPA_SUPPLICANT_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_LED),y)
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/led.o
EXTRA_CFLAGS += -DSTA_LED
endif

ifeq ($(CONFIG_RT2860V2_STA_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_DLS),y)
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/sta/dls.c
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif


ifeq ($(CONFIG_RT2860V2_STA_WSC),y)
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/wsc.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/wsc_tlv.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/sha2.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/hmac.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/dh_key.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/evp_enc.o
EXTRA_CFLAGS += -DWSC_INCLUDED -DWSC_STA_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_DPB),y)
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_mat.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_mat_iparp.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_mat_pppoe.o
rt2860v2_sta-objs += ../rt2860v2_sta_dpb/common/cmm_mat_ipv6.o
EXTRA_CFLAGS += -DETH_CONVERT_SUPPORT -DMAT_SUPPORT
endif

