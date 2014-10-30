/*****************************add by mark*************************/
//#include <linux/leds.h>
//#include <linux/device.h>
//#include <linux/suspend.h>
//#include <mach/sys_config.h>
#define DEVICE_NAME		"al300x"
#define LTR303_SLAVE_ADDR	0x29
#define LTR303_PART_ID		0x86
#define LTR303_MANUFACTURER_ID	0x87
#define PARTID 0xA0
#define MANUID 0x05
/*
enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_REPORT_ALS_DATA = 1U << 1,
	DEBUG_REPORT_PS_DATA = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
	DEBUG_CONTROL_INFO = 1U << 4,
	DEBUG_INT = 1U << 5,
};

#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk("*ltr_303:*" fmt , ## arg)

static u32 debug_mask = 0;
*/
struct sensor_config{
	int twi_id;
	int int1;
	int int_mode;
};
static struct sensor_config sensor_config ;


/*: Addresses to scan */

static const unsigned short normal_i2c[2] = {LTR303_SLAVE_ADDR,I2C_CLIENT_END};
static int i2c_num = 0;
static const unsigned short i2c_address[] = {LTR303_SLAVE_ADDR, LTR303_SLAVE_ADDR};
/************************************add end**********************/

