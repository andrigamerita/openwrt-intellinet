/*
 * Compatibility header for Sierra Wireless DirectIP driver 
 *
 * Copyright (C) 2009 Matthew Safar, Rory Filer
 *                    <linux@sierrawireless.com>
 *
 * IMPORTANT DISCLAIMER: This header is not commercially supported by
 * Sierra Wireless. Use at your own risk.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* 
 * Older kernel versions do not have certain tools/utilitty routines implemented
 * yet. In order to backport sierra_net to those kernels, this .h file gets
 * included and provides an implementation.
 */

static inline void skb_reset_mac_header(struct sk_buff *skb)
{
	skb->mac.raw = skb->data;
}

static inline void skb_set_transport_header(struct sk_buff *skb, const int offset)
{
	skb->h.raw = skb->data + offset;
}

static inline void skb_set_network_header(struct sk_buff *skb, const int offset)
{
	skb->nh.raw = skb->data + offset;
}

static inline struct iphdr * ip_hdr(const struct sk_buff *skb)
{
	return skb->nh.iph;
}

static inline int ip_hdrlen(const struct sk_buff *skb)
{
	return ip_hdr(skb)->ihl*4;
}

static inline struct udphdr * udp_hdr(const struct sk_buff *skb)
{
	return skb->h.uh;
}
