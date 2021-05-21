/*
  USB Driver for GSM modems

  Copyright (C) 2005  Matthias Urlichs <smurf@smurf.noris.de>

  This driver is free software; you can redistribute it and/or modify
  it under the terms of Version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Portions copied from the Keyspan driver by Hugh Blemings <hugh@blemings.org>

  History: see the git log.

  Work sponsored by: Sigos GmbH, Germany <info@sigos.de>

  This driver exists because the "normal" serial driver doesn't work too well
  with GSM modems. Issues:
  - data loss -- one single Receive URB is not nearly enough
  - nonstandard flow (zte devices) control
  - controlling the baud rate doesn't make sense

  This driver is named "zte" because the most common device it's
  used for is a PC-Card (with an internal OHCI-USB interface, behind
  which the GSM interface sits), made by zte Inc.

  Some of the "one port" devices actually exhibit multiple USB instances
  on the USB bus. This is not a bug, these ports are used for different
  device features.
*/
#define DRIVER_VERSION "v2.0"
#define DRIVER_AUTHOR "Matthias Urlichs <smurf@smurf.noris.de>"
#define DRIVER_DESC "USB Driver for GSM modems"

//add by zhaoming-------start
#if ! defined(LINUX_VERSION_CODE)  
#include <linux/version.h>
#endif

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif
//add by zhaoming-------stop

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)//use 2.6.22 sourcecode
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct usb_serial_port *port, struct file *filp);
static void zte_close(struct usb_serial_port *port, struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static void zte_rx_throttle(struct usb_serial_port *port);
static void zte_rx_unthrottle(struct usb_serial_port *port);
static int  zte_write_room(struct usb_serial_port *port);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  zte_chars_in_buffer(struct usb_serial_port *port);
static int  zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void zte_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void zte_break_ctl(struct usb_serial_port *port, int break_state);
static int  zte_tiocmget(struct usb_serial_port *port, struct file *file);
static int  zte_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	   
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.throttle          = zte_rx_throttle,
	.unthrottle        = zte_rx_unthrottle,
	.ioctl             = zte_ioctl,
	.set_termios       = zte_set_termios,
	.break_ctl         = zte_break_ctl,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	char in_buffer[N_IN_URB][IN_BUFLEN];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	char out_buffer[N_OUT_URB][OUT_BUFLEN];

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister (&zte_driver);
	usb_serial_deregister (&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void zte_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __FUNCTION__);

	zte_send_setup(port);
}

static int zte_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(port);
}

static int zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (this_urb->status == -EINPROGRESS) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

	if (urb->status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __FUNCTION__, urb->status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && urb->status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __FUNCTION__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;

	usb_serial_port_softint(port);
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __FUNCTION__);
	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (urb->status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __FUNCTION__, urb->status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (urb->status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__FUNCTION__, err);
	}
}

static int zte_write_room(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status != -EINPROGRESS)
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status == -EINPROGRESS)
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_open(struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	zte_send_setup(port);

	return (0);
}

static void zte_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		zte_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, zte_outdat_callback);
		}
	}
}

static int zte_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}

	zte_setup_urbs(serial);

	return (0);
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)//use 2.6.23 sourcecode

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct usb_serial_port *port, struct file *filp);
static void zte_close(struct usb_serial_port *port, struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static void zte_rx_throttle(struct usb_serial_port *port);
static void zte_rx_unthrottle(struct usb_serial_port *port);
static int  zte_write_room(struct usb_serial_port *port);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  zte_chars_in_buffer(struct usb_serial_port *port);
static int  zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void zte_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void zte_break_ctl(struct usb_serial_port *port, int break_state);
static int  zte_tiocmget(struct usb_serial_port *port, struct file *file);
static int  zte_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.throttle          = zte_rx_throttle,
	.unthrottle        = zte_rx_unthrottle,
	.ioctl             = zte_ioctl,
	.set_termios       = zte_set_termios,
	.break_ctl         = zte_break_ctl,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	char in_buffer[N_IN_URB][IN_BUFLEN];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	char out_buffer[N_OUT_URB][OUT_BUFLEN];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister (&zte_driver);
	usb_serial_deregister (&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void zte_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __FUNCTION__);

	zte_send_setup(port);
}

static int zte_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(port);
}

static int zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __FUNCTION__, status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __FUNCTION__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __FUNCTION__);
	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __FUNCTION__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__FUNCTION__, err);
	}
}

static int zte_write_room(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_open(struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	zte_send_setup(port);

	return (0);
}

static void zte_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		zte_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, zte_outdat_callback);
		}
	}
}

static int zte_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}

	zte_setup_urbs(serial);

	return (0);
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)//use 2.6.24 sourcecode

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct usb_serial_port *port, struct file *filp);
static void zte_close(struct usb_serial_port *port, struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static void zte_rx_throttle(struct usb_serial_port *port);
static void zte_rx_unthrottle(struct usb_serial_port *port);
static int  zte_write_room(struct usb_serial_port *port);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  zte_chars_in_buffer(struct usb_serial_port *port);
static int  zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void zte_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void zte_break_ctl(struct usb_serial_port *port, int break_state);
static int  zte_tiocmget(struct usb_serial_port *port, struct file *file);
static int  zte_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.throttle          = zte_rx_throttle,
	.unthrottle        = zte_rx_unthrottle,
	.ioctl             = zte_ioctl,
	.set_termios       = zte_set_termios,
	.break_ctl         = zte_break_ctl,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	char in_buffer[N_IN_URB][IN_BUFLEN];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	char out_buffer[N_OUT_URB][OUT_BUFLEN];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister (&zte_driver);
	usb_serial_deregister (&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void zte_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __FUNCTION__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(port->tty->termios, old_termios);
	zte_send_setup(port);
}

static int zte_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(port);
}

static int zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __FUNCTION__, status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __FUNCTION__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __FUNCTION__);
	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __FUNCTION__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__FUNCTION__, err);
	}
}

static int zte_write_room(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_open(struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	zte_send_setup(port);

	return (0);
}

static void zte_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		zte_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, zte_outdat_callback);
		}
	}
}

static int zte_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}

	zte_setup_urbs(serial);

	return (0);
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)//use 2.6.25 sourcecode

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct usb_serial_port *port, struct file *filp);
static void zte_close(struct usb_serial_port *port, struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static void zte_rx_throttle(struct usb_serial_port *port);
static void zte_rx_unthrottle(struct usb_serial_port *port);
static int  zte_write_room(struct usb_serial_port *port);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  zte_chars_in_buffer(struct usb_serial_port *port);
static int  zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void zte_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void zte_break_ctl(struct usb_serial_port *port, int break_state);
static int  zte_tiocmget(struct usb_serial_port *port, struct file *file);
static int  zte_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.throttle          = zte_rx_throttle,
	.unthrottle        = zte_rx_unthrottle,
	.ioctl             = zte_ioctl,
	.set_termios       = zte_set_termios,
	.break_ctl         = zte_break_ctl,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8 *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8 *out_buffer[N_OUT_URB];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister (&zte_driver);
	usb_serial_deregister (&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void zte_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void zte_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __FUNCTION__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(port->tty->termios, old_termios);
	zte_send_setup(port);
}

static int zte_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(port);
}

static int zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __FUNCTION__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __FUNCTION__, status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __FUNCTION__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __FUNCTION__);
	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __FUNCTION__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__FUNCTION__, err);
	}
}

static int zte_write_room(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int zte_open(struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __FUNCTION__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__FUNCTION__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	zte_send_setup(port);

	return (0);
}

static void zte_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		mutex_lock(&serial->disc_mutex);
		if (!serial->disconnected)
			zte_send_setup(port);
		mutex_unlock(&serial->disc_mutex);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, zte_outdat_callback);
		}
	}
}

static int zte_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	u8 *buffer;

	dbg("%s", __FUNCTION__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__FUNCTION__, err);
	}

	zte_setup_urbs(serial);

	return (0);

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return 1;
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				free_page((unsigned long)portdata->in_buffer[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				kfree(portdata->out_buffer[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)//use 2.6.26 sourcecode

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct usb_serial_port *port, struct file *filp);
static void zte_close(struct usb_serial_port *port, struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static void zte_rx_throttle(struct usb_serial_port *port);
static void zte_rx_unthrottle(struct usb_serial_port *port);
static int  zte_write_room(struct usb_serial_port *port);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count);

static int  zte_chars_in_buffer(struct usb_serial_port *port);
static int  zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void zte_set_termios(struct usb_serial_port *port,
				struct ktermios *old);
static void zte_break_ctl(struct usb_serial_port *port, int break_state);
static int  zte_tiocmget(struct usb_serial_port *port, struct file *file);
static int  zte_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.throttle          = zte_rx_throttle,
	.unthrottle        = zte_rx_unthrottle,
	.ioctl             = zte_ioctl,
	.set_termios       = zte_set_termios,
	.break_ctl         = zte_break_ctl,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8 *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8 *out_buffer[N_OUT_URB];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister (&zte_driver);
	usb_serial_deregister (&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __func__);
}

static void zte_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __func__);
}

static void zte_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __func__);
}

static void zte_set_termios(struct usb_serial_port *port,
			struct ktermios *old_termios)
{
	dbg("%s", __func__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(port->tty->termios, old_termios);
	zte_send_setup(port);
}

static int zte_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	/* FIXME: what locks portdata fields ? */
	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(port);
}

static int zte_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/* Write */
static int zte_write(struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __func__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __func__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __func__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __func__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port =  urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __func__, status, endpoint);
	} else {
		tty = port->tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __func__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->open_count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __func__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __func__);

	port =  urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port =  urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __func__);
	dbg("%s: urb %p port %p has data %p", __func__,urb,port,portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __func__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __func__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __func__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __func__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__func__, err);
	}
}

static int zte_write_room(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);


	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct usb_serial_port *port)
{
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		/* FIXME: This locking is insufficient as this_urb may
		   go unused during the test */
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_open(struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __func__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __func__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__func__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	port->tty->low_latency = 1;

	zte_send_setup(port);

	return (0);
}

static void zte_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __func__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		mutex_lock(&serial->disc_mutex);
		if (!serial->disconnected)
			zte_send_setup(port);
		mutex_unlock(&serial->disc_mutex);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __func__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, zte_outdat_callback);
		}
	}
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
*/
static int zte_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;
	int ifNum = serial->interface->cur_altsetting->desc.bInterfaceNumber;
	dbg("%s", __func__);

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,ifNum,NULL,0,USB_CTRL_SET_TIMEOUT);
	}

	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	u8 *buffer;

	dbg("%s", __func__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__func__, i);
			return (1);
		}

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__func__, err);
	}

	zte_setup_urbs(serial);

	return (0);

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return 1;
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				free_page((unsigned long)portdata->in_buffer[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				kfree(portdata->out_buffer[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)//use 2.6.27 sourcecode
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static void zte_close(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static int  zte_write_room(struct tty_struct *tty);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count);
static int  zte_chars_in_buffer(struct tty_struct *tty);
static void zte_set_termios(struct tty_struct *tty,
			struct usb_serial_port *port, struct ktermios *old);
static int  zte_tiocmget(struct tty_struct *tty, struct file *file);
static int  zte_tiocmset(struct tty_struct *tty, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct tty_struct *tty, struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.set_termios       = zte_set_termios,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

static int debug;

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8 *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8 *out_buffer[N_OUT_URB];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister(&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister(&zte_driver);
	usb_serial_deregister(&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	dbg("%s", __func__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(tty->termios, old_termios);
	zte_send_setup(tty, port);
}

static int zte_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct tty_struct *tty, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	/* FIXME: what locks portdata fields ? */
	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(tty, port);
}

/* Write */
static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __func__, count);

	i = 0;
	left = count;
	for (i = 0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __func__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy(this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __func__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __func__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port =  urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __func__, status, endpoint);
	} else {
		tty = port->port.tty;
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else {
			dbg("%s: empty read urb received", __func__);
		}

		/* Resubmit urb so we continue receiving */
		if (port->port.count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __func__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __func__);

	port =  urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port =  urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __func__);
	dbg("%s: urb %p port %p has data %p", __func__, urb, port, portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __func__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __func__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->port.tty && !C_CLOCAL(port->port.tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->port.tty);
		} else {
			dbg("%s: type %x req %x", __func__,
				req_pkt->bRequestType, req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __func__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__func__, err);
	}
}

static int zte_write_room(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);


	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		/* FIXME: This locking is insufficient as this_urb may
		   go unused during the test */
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_open(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __func__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (!urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __func__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__func__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (!urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	if (tty)
		tty->low_latency = 1;

	zte_send_setup(tty, port);

	return 0;
}

static void zte_close(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __func__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		mutex_lock(&serial->disc_mutex);
		if (!serial->disconnected)
			zte_send_setup(tty, port);
		mutex_unlock(&serial->disc_mutex);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->port.tty = NULL;	/* FIXME */
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __func__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb(serial,
					port->bulk_in_endpointAddress,
					USB_DIR_IN, port,
					portdata->in_buffer[j],
					IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb(serial,
					port->bulk_out_endpointAddress,
					USB_DIR_OUT, port,
					portdata->out_buffer[j],
					OUT_BUFLEN, zte_outdat_callback);
		}
	}
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
*/
static int zte_send_setup(struct tty_struct *tty,
						struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;
	int ifNum = serial->interface->cur_altsetting->desc.bInterfaceNumber;
	dbg("%s", __func__);

	portdata = usb_get_serial_port_data(port);

	if (tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
			usb_rcvctrlpipe(serial->dev, 0),
			0x22, 0x21, val, ifNum, NULL, 0, USB_CTRL_SET_TIMEOUT);
	}
	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	u8 *buffer;

	dbg("%s", __func__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__func__, i);
			return 1;
		}

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (!port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__func__, err);
	}
	zte_setup_urbs(serial);
	return 0;

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return 1;
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				free_page((unsigned long)
					portdata->in_buffer[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				kfree(portdata->out_buffer[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
//#else

//#endif
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)//use 2.6.28 sourcecode
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static void zte_close(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static int  zte_write_room(struct tty_struct *tty);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count);
static int  zte_chars_in_buffer(struct tty_struct *tty);
static void zte_set_termios(struct tty_struct *tty,
			struct usb_serial_port *port, struct ktermios *old);
static int  zte_tiocmget(struct tty_struct *tty, struct file *file);
static int  zte_tiocmset(struct tty_struct *tty, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct tty_struct *tty, struct usb_serial_port *port);

/* Vendor and product IDs */
#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	 
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.set_termios       = zte_set_termios,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

static int debug;

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 1
#define IN_BUFLEN 4096
#define OUT_BUFLEN 128

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8 *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8 *out_buffer[N_OUT_URB];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");

	return 0;

failed_driver_register:
	usb_serial_deregister(&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister(&zte_driver);
	usb_serial_deregister(&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	dbg("%s", __func__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(tty->termios, old_termios);
	zte_send_setup(tty, port);
}

static int zte_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct tty_struct *tty, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	/* FIXME: what locks portdata fields ? */
	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(tty, port);
}

/* Write */
static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __func__, count);

	i = 0;
	left = count;
	for (i = 0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __func__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy(this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __func__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __func__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port =  urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __func__, status, endpoint);
	} else {
		tty = tty_port_tty_get(&port->port);
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else 
			dbg("%s: empty read urb received", __func__);
		tty_kref_put(tty);

		/* Resubmit urb so we continue receiving */
		if (port->port.count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __func__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __func__);

	port =  urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port =  urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __func__);
	dbg("%s: urb %p port %p has data %p", __func__, urb, port, portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __func__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __func__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (old_dcd_state && !portdata->dcd_state) {
				struct tty_struct *tty =
						tty_port_tty_get(&port->port);
				if (tty && !C_CLOCAL(tty))
					tty_hangup(tty);
				tty_kref_put(tty);
			}
		} else {
			dbg("%s: type %x req %x", __func__,
				req_pkt->bRequestType, req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __func__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__func__, err);
	}
}

static int zte_write_room(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);


	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		/* FIXME: This locking is insufficient as this_urb may
		   go unused during the test */
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_open(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __func__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (!urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __func__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__func__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (!urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	if (tty)
		tty->low_latency = 1;

	zte_send_setup(tty, port);

	return 0;
}

static void zte_close(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __func__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		mutex_lock(&serial->disc_mutex);
		if (!serial->disconnected)
			zte_send_setup(tty, port);
		mutex_unlock(&serial->disc_mutex);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	tty_port_tty_set(&port->port, NULL);
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __func__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb(serial,
					port->bulk_in_endpointAddress,
					USB_DIR_IN, port,
					portdata->in_buffer[j],
					IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb(serial,
					port->bulk_out_endpointAddress,
					USB_DIR_OUT, port,
					portdata->out_buffer[j],
					OUT_BUFLEN, zte_outdat_callback);
		}
	}
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
*/
static int zte_send_setup(struct tty_struct *tty,
						struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;
	int ifNum = serial->interface->cur_altsetting->desc.bInterfaceNumber;
	dbg("%s", __func__);

	portdata = usb_get_serial_port_data(port);

	if (tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
			usb_rcvctrlpipe(serial->dev, 0),
			0x22, 0x21, val, ifNum, NULL, 0, USB_CTRL_SET_TIMEOUT);
	}
	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	u8 *buffer;

	dbg("%s", __func__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__func__, i);
			return 1;
		}

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (!port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__func__, err);
	}
	zte_setup_urbs(serial);
	return 0;

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return 1;
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				free_page((unsigned long)
					portdata->in_buffer[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				kfree(portdata->out_buffer[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
//#else

//#endif

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)//use 2.6.29 sourcecode
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

/* Function prototypes */
static int  zte_open(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static void zte_close(struct tty_struct *tty, struct usb_serial_port *port,
							struct file *filp);
static int  zte_startup(struct usb_serial *serial);
static void zte_shutdown(struct usb_serial *serial);
static int  zte_write_room(struct tty_struct *tty);

static void zte_instat_callback(struct urb *urb);

static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count);
static int  zte_chars_in_buffer(struct tty_struct *tty);
static void zte_set_termios(struct tty_struct *tty,
			struct usb_serial_port *port, struct ktermios *old);
static int  zte_tiocmget(struct tty_struct *tty, struct file *file);
static int  zte_tiocmset(struct tty_struct *tty, struct file *file,
				unsigned int set, unsigned int clear);
static int  zte_send_setup(struct tty_struct *tty, struct usb_serial_port *port);

#define zte_VENDOR_ID			0x19d2

static struct usb_device_id zte_ids[] = {
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0001, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0003, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0004, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0005, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0006, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0007, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0008, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0009, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0010, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0011, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0012, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0013, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0014, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0015, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0016, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0017, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0018, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0019, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0020, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0021, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0022, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0023, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0024, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0025, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0026, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0027, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0028, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0029, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0030, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0031, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0032, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0033, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0034, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0035, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0036, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0037, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0038, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0039, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0040, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0041, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0042, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0043, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0044, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0045, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0046, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0047, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0048, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0049, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0050, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0051, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0052, 0xff, 0xff, 0xff) },
//   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0053, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0054, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0055, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0056, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0057, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0058, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0059, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0060, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0061, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0062, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0063, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0064, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0065, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0066, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0067, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0068, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0069, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0070, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0071, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0072, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0073, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0074, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0075, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0076, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0077, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0078, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0079, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0080, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0081, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0082, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0083, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0084, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0085, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0086, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0087, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0088, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0089, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0090, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0091, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0092, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0093, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0094, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0095, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0096, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0097, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0098, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x0099, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2002, 0xff, 0xff, 0xff) },
   { USB_DEVICE_AND_INTERFACE_INFO(zte_VENDOR_ID, 0x2003, 0xff, 0xff, 0xff) },	
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, zte_ids);

static struct usb_driver zte_driver = {
	.name       = "zte",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = zte_ids,
	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver zte_1port_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"zte1",
	},
	.description       = "GSM modem (1-port)",
	.usb_driver        = &zte_driver,
	.id_table          = zte_ids,
	.num_ports         = 1,
	.open              = zte_open,
	.close             = zte_close,
	.write             = zte_write,
	.write_room        = zte_write_room,
	.chars_in_buffer   = zte_chars_in_buffer,
	.set_termios       = zte_set_termios,
	.tiocmget          = zte_tiocmget,
	.tiocmset          = zte_tiocmset,
	.attach            = zte_startup,
	.shutdown          = zte_shutdown,
	.read_int_callback = zte_instat_callback,
};

static int debug;

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 4
#define IN_BUFLEN 4096
#define OUT_BUFLEN 4096

struct zte_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	u8 *in_buffer[N_IN_URB];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	u8 *out_buffer[N_OUT_URB];
	unsigned long out_busy;		/* Bit vector of URBs in use */

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

/* Functions used by new usb-serial code. */
static int __init zte_init(void)
{
	int retval;
	retval = usb_serial_register(&zte_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&zte_driver);
	if (retval)
		goto failed_driver_register;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");

	return 0;

failed_driver_register:
	usb_serial_deregister(&zte_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit zte_exit(void)
{
	usb_deregister(&zte_driver);
	usb_serial_deregister(&zte_1port_device);
}

module_init(zte_init);
module_exit(zte_exit);

static void zte_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	dbg("%s", __func__);
	/* Doesn't support zte setting */
	tty_termios_copy_hw(tty->termios, old_termios);
	zte_send_setup(tty, port);
}

static int zte_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int value;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int zte_tiocmset(struct tty_struct *tty, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	/* FIXME: what locks portdata fields ? */
	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return zte_send_setup(tty, port);
}

/* Write */
static int zte_write(struct tty_struct *tty, struct usb_serial_port *port,
			const unsigned char *buf, int count)
{
	struct zte_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;

	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __func__, count);

	i = 0;
	left = count;
	for (i = 0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
		if (test_and_set_bit(i, &portdata->out_busy)) {
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
		}
		dbg("%s: endpoint %d buf %d", __func__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
		memcpy(this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d)", this_urb, err);
			clear_bit(i, &portdata->out_busy);
			continue;
		}
		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	dbg("%s: wrote (did %d)", __func__, count);
	return count;
}

static void zte_indat_callback(struct urb *urb)
{
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int status = urb->status;

	dbg("%s: %p", __func__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port =  urb->context;

	if (status) {
		dbg("%s: nonzero status: %d on endpoint %02x.",
		    __func__, status, endpoint);
	} else {
		tty = tty_port_tty_get(&port->port);
		if (urb->actual_length) {
			tty_buffer_request_room(tty, urb->actual_length);
			tty_insert_flip_string(tty, data, urb->actual_length);
			tty_flip_buffer_push(tty);
		} else 
			dbg("%s: empty read urb received", __func__);
		tty_kref_put(tty);

		/* Resubmit urb so we continue receiving */
		if (port->port.count && status != -ESHUTDOWN) {
			err = usb_submit_urb(urb, GFP_ATOMIC);
			if (err)
				printk(KERN_ERR "%s: resubmit read urb failed. "
					"(%d)", __func__, err);
		}
	}
	return;
}

static void zte_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	int i;

	dbg("%s", __func__);

	port =  urb->context;

	usb_serial_port_softint(port);

	portdata = usb_get_serial_port_data(port);
	for (i = 0; i < N_OUT_URB; ++i) {
		if (portdata->out_urbs[i] == urb) {
			smp_mb__before_clear_bit();
			clear_bit(i, &portdata->out_busy);
			break;
		}
	}
}

static void zte_instat_callback(struct urb *urb)
{
	int err;
	int status = urb->status;
	struct usb_serial_port *port =  urb->context;
	struct zte_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

	dbg("%s", __func__);
	dbg("%s: urb %p port %p has data %p", __func__, urb, port, portdata);

	if (status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __func__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __func__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (old_dcd_state && !portdata->dcd_state) {
				struct tty_struct *tty =
						tty_port_tty_get(&port->port);
				if (tty && !C_CLOCAL(tty))
					tty_hangup(tty);
				tty_kref_put(tty);
			}
		} else {
			dbg("%s: type %x req %x", __func__,
				req_pkt->bRequestType, req_pkt->bRequest);
		}
	} else
		dbg("%s: error %d", __func__, status);

	/* Resubmit urb so we continue receiving IRQ data */
	if (status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err)
			dbg("%s: resubmit intr urb failed. (%d)",
				__func__, err);
	}
}

static int zte_write_room(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);


	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && !test_bit(i, &portdata->out_busy))
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_chars_in_buffer(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct zte_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i = 0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		/* FIXME: This locking is insufficient as this_urb may
		   go unused during the test */
		if (this_urb && test_bit(i, &portdata->out_busy))
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __func__, data_len);
	return data_len;
}

static int zte_open(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	struct zte_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __func__);

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) {
		urb = portdata->in_urbs[i];
		if (!urb)
			continue;
		if (urb->dev != serial->dev) {
			dbg("%s: dev %p != %p", __func__,
				urb->dev, serial->dev);
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_clear_halt(urb->dev, urb->pipe);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			dbg("%s: submit urb %d failed (%d) %d",
				__func__, i, err,
				urb->transfer_buffer_length);
		}
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (!urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */
	}

	if (tty)
		tty->low_latency = 1;

	zte_send_setup(tty, port);

	return 0;
}

static void zte_close(struct tty_struct *tty,
			struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;

	dbg("%s", __func__);
	portdata = usb_get_serial_port_data(port);

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		mutex_lock(&serial->disc_mutex);
		if (!serial->disconnected)
			zte_send_setup(tty, port);
		mutex_unlock(&serial->disc_mutex);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	tty_port_tty_set(&port->port, NULL);
}

/* Helper functions used by zte_setup_urbs */
static struct urb *zte_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __func__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void zte_setup_urbs(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = zte_setup_urb(serial,
					port->bulk_in_endpointAddress,
					USB_DIR_IN, port,
					portdata->in_buffer[j],
					IN_BUFLEN, zte_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = zte_setup_urb(serial,
					port->bulk_out_endpointAddress,
					USB_DIR_OUT, port,
					portdata->out_buffer[j],
					OUT_BUFLEN, zte_outdat_callback);
		}
	}
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
*/
static int zte_send_setup(struct tty_struct *tty,
						struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct zte_port_private *portdata;
	int ifNum = serial->interface->cur_altsetting->desc.bInterfaceNumber;
	dbg("%s", __func__);

	portdata = usb_get_serial_port_data(port);

	if (tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
			usb_rcvctrlpipe(serial->dev, 0),
			0x22, 0x21, val, ifNum, NULL, 0, USB_CTRL_SET_TIMEOUT);
	}
	return 0;
}

static int zte_startup(struct usb_serial *serial)
{
	int i, j, err;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;
	u8 *buffer;

	dbg("%s", __func__);

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for zte_port_private (%d) failed!.",
					__func__, i);
			return 1;
		}

		for (j = 0; j < N_IN_URB; j++) {
			buffer = (u8 *)__get_free_page(GFP_KERNEL);
			if (!buffer)
				goto bail_out_error;
			portdata->in_buffer[j] = buffer;
		}

		for (j = 0; j < N_OUT_URB; j++) {
			buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
			if (!buffer)
				goto bail_out_error2;
			portdata->out_buffer[j] = buffer;
		}

		usb_set_serial_port_data(port, portdata);

		if (!port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		if (err)
			dbg("%s: submit irq_in urb failed %d",
				__func__, err);
	}
	zte_setup_urbs(serial);
	return 0;

bail_out_error2:
	for (j = 0; j < N_OUT_URB; j++)
		kfree(portdata->out_buffer[j]);
bail_out_error:
	for (j = 0; j < N_IN_URB; j++)
		if (portdata->in_buffer[j])
			free_page((unsigned long)portdata->in_buffer[j]);
	kfree(portdata);
	return 1;
}

static void zte_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct zte_port_private *portdata;

	dbg("%s", __func__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				free_page((unsigned long)
					portdata->in_buffer[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				kfree(portdata->out_buffer[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#else  //当有30时，请注释掉本行和下行if some new code added,pls remove this line and next line

#endif
//#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)//use 2.6.30 sourcecode
//#else
//#endif
//#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)//use 2.6.31 sourcecode
//#else
//#endif
