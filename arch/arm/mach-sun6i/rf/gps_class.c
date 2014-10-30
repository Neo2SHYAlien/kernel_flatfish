#include <linux/module.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/system.h>

#define GPS_RESET	GPIOA(0)
#define GPS_STANDBY	GPIOA(1)

#define GPS_MSG(...)     do {printk("[gps]: "__VA_ARGS__);} while(0)

static int standby_state;
static int reset_state;

static ssize_t standby_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	standby_state = __gpio_get_value(GPS_STANDBY);
	GPS_MSG("gps_standby = %d\n",standby_state);
	return sprintf(buf, "%d\n", standby_state);
}

static ssize_t standby_store(struct class *class,
				  struct class_attribute *attr,
				  const char *buf, size_t count)
{
	int err;
	int store_state;

	store_state = simple_strtoul(buf, 0, 10);

	switch (store_state){
	case 1:
		err = gpio_direction_output(GPS_STANDBY,1);
		if (err) {
			GPS_MSG("failed to set gps_standby to high.\n");
			return -1;
		}
		else
		{
			GPS_MSG("succeed to set gps_standby to high.\n");
		}
		break;
	case 0:
		err = gpio_direction_output(GPS_STANDBY,0);
		if (err) {
			GPS_MSG("failed to set gps_standby to low.\n");
			return -1;
		}
		else
		{
			GPS_MSG("succeed to set gps_standby to low.\n");
		}
		break;
	default:
 	break;
	}
	return count;
}

static ssize_t reset_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	reset_state = __gpio_get_value(GPS_RESET); 
	GPS_MSG("gps_reset = %d\n",reset_state);
	return sprintf(buf, "%d\n", reset_state);
}

static ssize_t reset_store(struct class *class,
				  struct class_attribute *attr,
				  const char *buf, size_t count)
{
	int err;
	int store_state;

	store_state = simple_strtoul(buf, 0, 10);

	switch (store_state){
	case 1:
		err = gpio_direction_output(GPS_RESET,1);
		if (err) {
			GPS_MSG("failed to set gps_reset to high.\n");
			return -1;
		}
		else
		{
			GPS_MSG("succeed to set gps_reset to high.\n");
		}
		break;
	case 0:
		err = gpio_direction_output(GPS_RESET,0);
		if (err) {
			GPS_MSG("failed to set gps_reset to low.\n");
			return -1;
		}
		else
		{
			GPS_MSG("succeed to set gps_reset to low.\n");
		}
		break;
	default:
 	break;
	}
	return count;
}

#if 0
static void bcm4751_config_32k_clk(void)
{
   unsigned int reg_addr, reg_val;
   unsigned int tiwl18xx_32k_gpio;

   tiwl18xx_32k_gpio = GPIOM(7);
   gpio_request(tiwl18xx_32k_gpio, NULL);
   sw_gpio_setpull(tiwl18xx_32k_gpio, 1);
   sw_gpio_setdrvlevel(tiwl18xx_32k_gpio, 3);
   sw_gpio_setcfg(tiwl18xx_32k_gpio, 0x03);

//enable clk
   reg_addr = 0xf1f01400 + 0xf0;
   reg_val = readl(reg_addr);
   writel( reg_val | (1<<31), reg_addr); 
}
#endif

static struct class_attribute gps_dev_attrs[] = {
	__ATTR(standby, 0666, standby_show, standby_store),
	__ATTR(reset, 0666, reset_show, reset_store),
	__ATTR_NULL
};


static struct class gps_class = {
	.name		= "gps",
	.class_attrs	= gps_dev_attrs,
};


static int __init gps_init(void)
{
	int error;
	error = class_register(&gps_class);
//	bcm4751_config_32k_clk();
	if (error)
	{
		GPS_MSG("failed to regist gps_class.\n");
		goto out;	
	}
	error = gpio_request(GPS_STANDBY,NULL);
	if (error)
		GPS_MSG("failed to request gps_standby.\n");
	error = gpio_direction_output(GPS_STANDBY,0);
	if (error)
		GPS_MSG("failed to set gps_standby to low.\n");
	error = gpio_request(GPS_RESET,NULL);
	if (error)
		GPS_MSG("failed to request gps_reset.\n");
	error = gpio_direction_output(GPS_RESET,1);
	if (error)
		GPS_MSG("failed to set gps_reset to high.\n");
	out:
	return error;
}

subsys_initcall(gps_init);

static void __exit gps_exit(void)
{
	class_unregister(&gps_class);
	gpio_free(GPS_STANDBY);
	gpio_free(GPS_RESET);
}
module_exit(gps_exit);
