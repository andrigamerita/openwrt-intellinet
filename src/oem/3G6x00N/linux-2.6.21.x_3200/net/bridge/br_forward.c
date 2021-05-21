/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
static int lanBlockPorts[4]= {255, 255, 255, 255};
extern int igmpsnoopenabled;
extern int blockLanInterface;
extern int IGMPL2ProxyOpened;
extern int ipv6BrEnabled;

/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{
	
	// { RexHua. Add for Block Lan access
	#if defined(_LAN_WAN_ACCESS_)

	//        if(!strcmp(p->dev->name, "ra1") || !strcmp(p->dev->name, "ra2") || !strcmp(p->dev->name, "ra3"))
		if( ( (p->port_no == lanBlockPorts[0]) && (blockLanInterface & 0x01) ) || ( (p->port_no == lanBlockPorts[1]) && (blockLanInterface & 0x02) ) || ( (p->port_no == lanBlockPorts[2]) && (blockLanInterface & 0x04) ) || ( (p->port_no == lanBlockPorts[3]) && (blockLanInterface & 0x08) ))
		{
	        	if(!strcmp(skb->dev->name, "br0"))
					return 1;
				else	
					return 0; 
		}       

	#endif
	// RexHua
	return (skb->dev != p->dev && p->state == BR_STATE_FORWARDING);
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except gso */
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
		kfree_skb(skb);
	else {
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else {
			skb_push(skb, ETH_HLEN);

			dev_queue_xmit(skb);
		}
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;
	static int tmpLanBlockPorts[3]= {255, 255, 255};
	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;
	list_for_each_entry_rcu(p, &br->port_list, list) {
		// { RexHua, For Block LAN Access

			#if defined(_LAN_WAN_ACCESS_)
				    if(!strcmp(p->dev->name, "ra0"))
						tmpLanBlockPorts[0] = p->port_no;
					else if(!strcmp(p->dev->name, "ra1"))
						tmpLanBlockPorts[1] = p->port_no;			
					else if(!strcmp(p->dev->name, "ra2"))
						tmpLanBlockPorts[2] = p->port_no;
					else if(!strcmp(p->dev->name, "ra3"))
						tmpLanBlockPorts[3] = p->port_no;
			#endif
	}
			#if defined(_LAN_WAN_ACCESS_)
				lanBlockPorts[0] = tmpLanBlockPorts[0];
				lanBlockPorts[1] = tmpLanBlockPorts[1];
				lanBlockPorts[2] = tmpLanBlockPorts[2];
				lanBlockPorts[3] = tmpLanBlockPorts[3];
			#endif

		// } RexHua 

	list_for_each_entry_rcu(p, &br->port_list, list) {
		int outFlag=1;

	#if defined(_IPV6_BRIDGE_) || defined(IGMP_SNOOPING)
			if(!strcmp(p->dev->name, _wanIfx_))
	#ifdef _IPV6_BRIDGE_
	                        if(!ipv6BrEnabled || (eth_hdr(skb)->h_proto != htons(ETH_P_IPV6))) // Filter Non IPv6 Flood
	#endif
	#ifdef IGMP_SNOOPING
	                        if(!IGMPProxyOpened || !MULTICAST_MAC(skb->mac.ethernet->h_dest)
						|| !(skb->mac.ethernet->h_proto == htons(ETH_P_IP)) || !(skb->nh.iph->protocol == IPPROTO_IGMP))
	#endif
				{
					outFlag = 0;
				}
	#endif
	
	

#if 0
//#ifdef _IPV6_BRIDGE_
		// Block IPv6 straming frame to wlan
		if( outFlag && ipv6BrEnabled && (eth_hdr(skb)->h_proto == htons(ETH_P_IPV6)))
			if(!memcmp(p->dev->name, "ra", 2) || !memcmp(p->dev->name, "wds", 3))
			{
				struct ipv6hdr *ipv6h;
				ipv6h = skb->nh.ipv6h;

//				printk("ipv6 addr %x:%x\n", ipv6h->daddr.s6_addr[0], ipv6h->daddr.s6_addr[1]);
				if((ipv6h->daddr.s6_addr[0] == 0xff) && (ipv6h->daddr.s6_addr[1] & 0x10))
					outFlag = 0;
			}
#endif
		
		if (should_deliver(p, skb) && outFlag) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
