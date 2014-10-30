/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
*
* File Name         : al303x.c //(edit)
* Authors            : Star-wu - Application Team
* Version            : V 0.2
* Date                : 06/24/2011
* Description       : al303x Digital Ambient Light Sensor API //(edit)
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/i2c-dev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/earlysuspend.h>
#define al300x_MAJOR   249
#define al300x_MINOR   4
#define G_MAX			8000
#define FUZZ			0
#define FLAT			0
/* al303x light sensor registers */

//(edit)#define CTRL_REG            0x00    /* power mode control reg */
#define CTRL_REG            0x80    /* power mode control reg */ //(edit)
//(edit)#define TIME_CTRL_REG       0x01    /* Timing Control reg */
//(edit)#define ALS_CTRL_REG        0x02    /*ALS Control reg */
//(edit)#define INT_STATUS_REG      0x03    /* Interrupt Status reg */
//(edit)#define DATA_REG            0x05    /* Data reg */
#define DATA_REG            0x88    /* Data reg */ //(edit)
//(edit)#define ALS_WINDOW_REG      0x08    /*ALS Window*/
#define NAME			"al300x"  //?? NAME of device shoudl be setup in BSP file. //(edit)
#define DEBUG 0
#define D printk("line=%d,func=%s\n",__LINE__,__func__);


#define al300x_IOCTL_BASE 'l'
/* The following define the IOCTL command values via the ioctl macros */
#define AL300X_SET_POWER_MODE		_IOW(al300x_IOCTL_BASE, 1, int)
#define AL300X_SET_ALS_LEVEL		_IOW(al300x_IOCTL_BASE, 2, int)
#define AL300X_READ_DATA_VALUE	    _IOW(al300x_IOCTL_BASE, 3, int)


/*add by mark*/
#include <mach/sys_config.h>
#include "LTR-303ALS.h"
//add end


struct al300x_data {
	struct i2c_client *client;
//	struct lsm303dlh_acc_platform_data *pdata;

	struct mutex lock;

	struct delayed_work input_work;
	struct input_dev *input_dev;

	int hw_initialized;
	atomic_t enabled;
	int on_before_suspend;
	struct early_suspend early_suspend;
	u8 shift_adj;
	u8 resume_state[5];
};

static struct al300x_data *acc;
static void al300x_early_suspend(struct early_suspend *h);
static void al300x_late_resume(struct early_suspend *h);
 /*
static int lux_table[64]={1,1,1,2,2,2,3,4,4,5,6,7,9,11,13,16,19,22,27,32,39,46,56,67,80,96,
                          116,139,167,200,240,289,346,416,499,599,720,864,1037,1245,1495,
                          1795,2154,2586,3105,3728,4475,5372,6449,7743,9295,11159,13396,
                          16082,19307,23178,27826,33405,40103,48144,57797,69386,83298,
                          100000};
*/                        
static int lux_table[64]={50,150,200,300,500,700,900,1000,1100,1200,1300,1400,1500,1600,13,16,19,22,27,32,39,46,56,67,80,96,
                          116,139,167,200,240,289,346,416,499,599,720,864,1037,1245,1495,
                          1795,2154,2586,3105,3728,4475,5372,6449,7743,9295,11159,13396,
                          16082,19307,23178,27826,33405,40103,48144,57797,69386,83298,
                          100000};                          
                          


struct al300x_data *light_data;

static char al300x_i2c_write(unsigned char reg_addr,
				    unsigned char *data,
				    unsigned char len);
/*
static char al300x_i2c_read(unsigned char reg_addr,
				   unsigned char *data,
				   unsigned char len);
*/
/*  i2c write routine for al300x digital accscope */
static char al300x_i2c_write(unsigned char reg_addr,
                unsigned char *data,  unsigned char len)
{
    int dummy;
	int i;
	if (light_data->client == NULL)  /*  No global client pointer? */
		return -1;
	for (i = 0; i < len; i++)
		{
		dummy = i2c_smbus_write_byte_data(light_data->client,
			                      reg_addr++, data[i]);
		if (dummy)
			{
			#if DEBUG
			printk(KERN_ERR "i2c write error\n");
			#endif
			return dummy;
			}
		}
	return 0;
}
/*
//  i2c read routine for al300x digital ambient Light Sensor 
static char al300x_i2c_read(unsigned char reg_addr,	
                    unsigned char *data, unsigned char len)
{
    int dummy = 0;
	int i = 0;

	if (light_data->client == NULL)  //  No global client pointer? 
		return -1;
	while (i < len)
		{
		dummy = i2c_smbus_read_word_data(light_data->client, reg_addr++);
		if (dummy >= 0)
			{
			data[i] = dummy & 0x00ff;
			i++;
			}
		else {
			#if DEBUG
			printk(KERN_ERR" i2c read error\n ");
			#endif
			return dummy;
			}
		dummy = len;
		}
	return dummy;
}
*/

int al300x_set_power_mode(char mode)
{
	int res = 0;
	res = i2c_smbus_read_word_data(light_data->client, CTRL_REG);
//(edit)	if ((res & 0x00FF) == 0x0003) {
	if ((res & 0x0001) == 0x0000) { //(edit)
	   res = al300x_i2c_write(CTRL_REG, &mode, 1);
	}
	return res;
}

int al300x_set_als_level(char level)
{
	int res = 0;
	unsigned char gain_level; //(edit)
	res = i2c_smbus_read_word_data(light_data->client, CTRL_REG);
//(edit)	if ((res & 0x00FF) == 0x0003) {
	if ((res & 0x00FF) == 0x0000) { //(edit)
//(edit)	   res = al300x_i2c_write(ALS_CTRL_REG, &level, 1);
		gain_level = res & 0xE3; //(edit)
		gain_level += (level << 2); //(edit)
		res = al300x_i2c_write(CTRL_REG, &gain_level, 1); //(edit)
	}
	return res;
}

/* Device Initialization  */
static int device_init(void)
{	
    int res;
	unsigned char buf[10];
//(edit)	buf[0] = 0x00;
//(edit)	buf[1] = 0x11;
//(edit)	buf[2] = 0xa0;
	buf[0] = 0x01; //(edit)
//mark	buf[0] = 0x0D;
	
//(edit)	res = al300x_i2c_write(CTRL_REG, &buf[0], 3);
		res = al300x_i2c_write(CTRL_REG, &buf[0], 1); //(edit)
//(edit)	buf[8] = 0x00;
//(edit)	res = al300x_i2c_write(ALS_WINDOW_REG, &buf[8], 1);
	return res;
}

/* lignt sensor data readout */
//int al300x_read_data_values(unsigned int *data)
int al300x_read_data_values(unsigned char *data)
{		
	int val;
	unsigned char rd_data[12]; //(edit)
	unsigned char als_gain_fac = 0, als_int_fac = 0; //(edit)
	unsigned short ch0_reading, ch1_reading, als_ratio, calc_lux;

	/* ------ (edit)
	* #bytes		Reg_addr				array_element
	* 1. 			0x80 (ALS_CONTR)		[0]
	* 2. 			0x81 (NA)				[1]
	* 3. 			0x82 (NA)				[2]
	* 4. 			0x83 (NA)				[3]
	* 5. 			0x84 (NA)				[4]
	* 6. 			0x85 (ALS_MEAS_RATE)	[5]
	* 7. 			0x86 (PART_ID)			[6]
	* 8. 			0x87 (MANUFAC_ID)		[7]
	* 9. 			0x88 (ALS_DATA_CH1_0)	[8]
	* A. 			0x89 (ALS_DATA_CH1_1)	[9]
	* B. 			0x8A (ALS_DATA_CH0_0)	[A]
	* C. 		0x8B (ALS_DATA_CH0_1)	[B]
	(edit) ------ */

//(edit)	val = i2c_smbus_read_byte_data(light_data->client, DATA_REG);
//	val = i2c_smbus_read_i2c_block_data(light_data->client, DATA_REG, 0x0C, rd_data); //(edit)
	val = i2c_smbus_read_i2c_block_data(light_data->client, CTRL_REG, 0x0C, rd_data);

//(edit)	val &= 0x3f;
//(edit)>
	if ((rd_data[0] & 0x1C) == 0x00) { 			//gain 1x
		als_gain_fac = 1;
	} else if ((rd_data[0] & 0x1C) == 0x04) {		//gain 2x
		als_gain_fac = 2;
	} else if ((rd_data[0] & 0x1C) == 0x08) {		//gain 4x
		als_gain_fac = 4;
	} else if ((rd_data[0] & 0x1C) == 0x0C) {		//gain 8x
		als_gain_fac = 8;
	} else if ((rd_data[0] & 0x18) == 0x0C) {		//gain 48x
		als_gain_fac = 48;
	} else if ((rd_data[0] & 0x1C) == 0x1C) {		//gain 96x
		als_gain_fac = 96;
	}

	if ((rd_data[5] & 0x38) == 0x00) {			//100ms
		als_int_fac = 10;
	} else if ((rd_data[5] & 0x38) == 0x08) {		//50ms
		als_int_fac = 5;
	} else if ((rd_data[5] & 0x38) == 0x10) {		//200ms
		als_int_fac = 20;
	} else if ((rd_data[5] & 0x38) == 0x18) {		//400ms
		als_int_fac = 40;
	} else if ((rd_data[5] & 0x38) == 0x20) {		//150ms
		als_int_fac = 15;
	} else if ((rd_data[5] & 0x38) == 0x28) {		//250ms
		als_int_fac = 25;
	} else if ((rd_data[5] & 0x38) == 0x30) {		//300ms
		als_int_fac = 30;
	} else if ((rd_data[5] & 0x38) == 0x38) {		//350ms
		als_int_fac = 35;
	}

	ch0_reading = rd_data[0x0B];
	ch0_reading <<= 8;
	ch0_reading += rd_data[0x0A];

	ch1_reading = rd_data[0x09];
	ch1_reading <<= 8;
	ch1_reading += rd_data[0x08];

	if ((ch0_reading + ch1_reading) == 0) {
		als_ratio = 1000;
	} else {
		als_ratio = (ch1_reading * 1000) / (ch0_reading + ch1_reading);
	}

	if (als_ratio < 450) {
		calc_lux = ((17743 * ch0_reading) + (11059 * ch1_reading)) / (als_gain_fac * (als_int_fac * 1000));
	} else if ((als_ratio >= 450) && (als_ratio < 640)) {
		calc_lux = ((42785 * ch0_reading) - (19548 * ch1_reading)) / (als_gain_fac * (als_int_fac * 1000));
	} else if ((als_ratio >= 640) && (als_ratio < 850)) {
		calc_lux = ((5926 * ch0_reading) + (1185 * ch1_reading)) / (als_gain_fac * (als_int_fac * 1000));
	} else {
		calc_lux = 0;
	}

	if ((calc_lux >= 0) && (calc_lux <= 1)) {
		val = 0;
	} else if (calc_lux == 2) {
		val = 3;
	} else if (calc_lux == 3) {
		val = 6;
	} else if (calc_lux == 4) {
		val = 7;
	} else if (calc_lux == 5) {
		val = 9;
	} else if (calc_lux == 6) {
		val = 10;
	} else if ((calc_lux >= 7) && (calc_lux < 9)) {
		val = 11;
	} else if ((calc_lux >= 9) && (calc_lux < 11)) {
		val = 12;
	} else if ((calc_lux >= 11) && (calc_lux < 13)) {
		val = 13;
	} else if ((calc_lux >= 13) && (calc_lux < 19)) {
		val = 15;
	} else if ((calc_lux >= 19) && (calc_lux < 22)) {
		val = 16;
	} else if ((calc_lux >= 22) && (calc_lux < 27)) {
		val = 17;
	} else if ((calc_lux >= 27) && (calc_lux < 32)) {
		val = 18;
	} else if ((calc_lux >= 32) && (calc_lux < 39)) {
		val = 19;
	} else if ((calc_lux >= 39) && (calc_lux < 46)) {
		val = 20;
	} else if ((calc_lux >= 46) && (calc_lux < 56)) {
		val = 21;
	} else if ((calc_lux >= 56) && (calc_lux < 67)) {
		val = 22;
	} else if ((calc_lux >= 67) && (calc_lux < 80)) {
		val = 23;
	} else if ((calc_lux >= 80) && (calc_lux < 96)) {
		val = 24;
	} else if ((calc_lux >= 96) && (calc_lux < 116)) {
		val = 25;
	} else if ((calc_lux >= 116) && (calc_lux < 139)) {
		val = 26;
	} else if ((calc_lux >= 139) && (calc_lux < 167)) {
		val = 27;
	} else if ((calc_lux >= 167) && (calc_lux < 200)) {
		val = 28;
	} else if ((calc_lux >= 200) && (calc_lux < 240)) {
		val = 29;
	} else if ((calc_lux >= 240) && (calc_lux < 289)) {
		val = 30;
	} else if ((calc_lux >= 289) && (calc_lux < 346)) {
		val = 31;
	} else if ((calc_lux >= 346) && (calc_lux < 416)) {
		val = 32;
	} else if ((calc_lux >= 416) && (calc_lux < 499)) {
		val = 33;
	} else if ((calc_lux >= 499) && (calc_lux < 599)) {
		val = 34;
	} else if ((calc_lux >= 599) && (calc_lux < 720)) {
		val = 35;
	} else if ((calc_lux >= 720) && (calc_lux < 864)) {
		val = 36;
	} else if ((calc_lux >= 864) && (calc_lux < 1037)) {
		val = 37;
	} else if ((calc_lux >= 1037) && (calc_lux < 1245)) {
		val = 38;
	} else if ((calc_lux >= 1245) && (calc_lux < 1495)) {
		val = 39;
	} else if ((calc_lux >= 1495) && (calc_lux < 1795)) {
		val = 40;
	} else if ((calc_lux >= 1795) && (calc_lux < 2145)) {
		val = 41;
	} else if ((calc_lux >= 2145) && (calc_lux < 2586)) {
		val = 42;
	} else if ((calc_lux >= 2586) && (calc_lux < 3105)) {
		val = 43;
	} else if ((calc_lux >= 3105) && (calc_lux < 3728)) {
		val = 44;
	} else if ((calc_lux >= 3728) && (calc_lux < 4475)) {
		val = 45;
	} else if ((calc_lux >= 4475) && (calc_lux < 5372)) {
		val = 46;
	} else if ((calc_lux >= 5372) && (calc_lux < 6449)) {
		val = 47;
	} else if ((calc_lux >= 6449) && (calc_lux < 7743)) {
		val = 48;
	} else if ((calc_lux >= 7743) && (calc_lux < 9295)) {
		val = 49;
	} else if ((calc_lux >= 9295) && (calc_lux < 11159)) {
		val = 50;
	} else if ((calc_lux >= 11159) && (calc_lux < 13396)) {
		val = 51;
	} else if ((calc_lux >= 13396) && (calc_lux < 16082)) {
		val = 52;
	} else if ((calc_lux >= 16082) && (calc_lux < 19307)) {
		val = 53;
	} else if ((calc_lux >= 19307) && (calc_lux < 23178)) {
		val = 54;
	} else if ((calc_lux >= 23178) && (calc_lux < 27826)) {
		val = 55;
	} else if ((calc_lux >= 27826) && (calc_lux < 33405)) {
		val = 56;
	} else if ((calc_lux >= 33405) && (calc_lux < 40103)) {
		val = 57;
	} else if ((calc_lux >= 40103) && (calc_lux < 48144)) {
		val = 58;
	} else if ((calc_lux >= 48144) && (calc_lux < 57797)) {
		val = 59;
//	} else if ((calc_lux >= 57797) && (calc_lux <= 65000)) {
	} else if ((calc_lux >= 57797) && (calc_lux <= 65535)) {
		val = 60;
	}
//(edit)<
	*data = (lux_table[val]);
	return val;

}

static void al300x_report_values(struct al300x_data *acc,
//					unsigned int *data)
					int data)
{
	
//	input_report_abs(acc->input_dev, ABS_MISC, *data);
	input_report_abs(acc->input_dev, ABS_MISC, data);

	input_sync(acc->input_dev);

}


static int al300x_enable(struct al300x_data *acc)
{
	int err;
	

	if (!atomic_cmpxchg(&acc->enabled, 0, 1)) {

		err = device_init();
		if (err < 0) {
			atomic_set(&acc->enabled, 0);
			return err;
		}
		schedule_delayed_work(&acc->input_work,
				      msecs_to_jiffies(200));
	}

	return 0;
}

static int al300x_disable(struct al300x_data *acc)
{
	if (atomic_cmpxchg(&acc->enabled, 1, 0)) {
		cancel_delayed_work_sync(&acc->input_work);
		
	}

	return 0;
}

/*  read command for al300x device file  */
static ssize_t al300x_read(struct file *file, char __user *buf,
                              size_t count, loff_t *offset)
{
   #if DEBUG	
   unsigned char data[1];
// unsigned int data[1];		//2013-06-11
   #endif
   if (light_data->client == NULL)
   	return -1;
   #if DEBUG
   al300x_read_data_values(&data[0]);
   #endif
   return 0;
}

/*  write command for al300x device file */
static ssize_t al300x_write(struct file *file, const char __user *buf,
           size_t count, loff_t *offset)
{	
   if (light_data->client == NULL)
   	return -1;
   #if DEBUG
   printk(KERN_INFO "al300x should be accessed with ioctl command\n");
   #endif
   return 0;
}

/*  open command for al300x device file  */
static int al300x_open(struct inode *inode, struct file *file)
{	
	int err;
	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	file->private_data = light_data;

	return 0;

  // device_init();
   #if DEBUG
   printk(KERN_INFO "al300x has been opened\n");
   #endif
   return 0;
}

/*  release command for al300x device file */
static int al300x_close(struct inode *inode, struct file *file)
{	
   #if DEBUG
   printk(KERN_INFO "al300x has been closed\n");
   #endif
   return 0;
}

/*  ioctl command for al300x device file */
//static int al300x_ioctl(struct file *file,
static long al300x_ioctl(struct file *file,
                   unsigned int cmd, unsigned long arg)
{
//  	printk(KERN_INFO "into read 303\n");	////////////////////
	int err = 0;
	unsigned char data[6];
//	unsigned int data[6];		//2013-06-13
	/* check al300x_client */
	if (acc->client == NULL) {
		#if DEBUG
		printk(KERN_ERR "I2C driver not install\n");
		#endif
		return -EFAULT;
	}
	/* cmd mapping */
switch (cmd) {

	case AL300X_SET_POWER_MODE:
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
			#if DEBUG
			printk(KERN_ERR "copy_to_user error\n");
			#endif
			return -EFAULT;
		}
		err = al300x_set_power_mode(*data);
		return err;
	case AL300X_SET_ALS_LEVEL: 
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
			#if DEBUG
			printk(KERN_ERR "copy_from_user error\n");
			#endif
			return -EFAULT;
		}
		err = al300x_set_als_level(*data);
		return err;
	case AL300X_READ_DATA_VALUE:
		err = al300x_read_data_values(&data[0]);
//		printk(KERN_INFO "into case err=%d\n");	////////////////////
//		err = al300x_read_data_values(&data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
			    #if DEBUG
				printk(KERN_ERR "copy_to error\n");
				#endif
				return -EFAULT;
		}
		return err;
	default:
		return 0;
 }
}



static ssize_t ls_poll_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return 0;	
}


static ssize_t ls_poll_delay_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	

	return size;
}

static ssize_t light_enable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	
	return sprintf(buf, "%d\n",
		       (&light_data->enabled ) ? 1 : 0);
}

static ssize_t light_enable_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{

	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		printk("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}
/*
	if(new_value)
		al300x_enable(light_data);		
	else
		al300x_disable(light_data);
*/		
	return size;
}


static DEVICE_ATTR(ls_poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   ls_poll_delay_show, ls_poll_delay_store);

static struct device_attribute dev_attr_light_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	       light_enable_show, light_enable_store);

static struct attribute *light_sysfs_attrs[] = {
	&dev_attr_light_enable.attr,
	&dev_attr_ls_poll_delay.attr,
	NULL
};

static struct attribute_group light_attribute_group = {
	.attrs = light_sysfs_attrs,
};

static const struct file_operations al300x_fops = {
	.owner = THIS_MODULE,	
	.read = al300x_read,	
	.write = al300x_write,
	.open = al300x_open,
	.release = al300x_close,
	.unlocked_ioctl = al300x_ioctl,
};
static struct miscdevice al300x_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = NAME,
	.fops = &al300x_fops,
};

static void al300x_input_work_func(struct work_struct *work)
{
	struct al300x_data *acc;
	unsigned char data;
//	unsigned int data;
	int err;
//   	int poll_interval = 200;
	int poll_interval = 600;

	acc = container_of((struct delayed_work *)work,
			    struct al300x_data, input_work);

	mutex_lock(&acc->lock);
	device_init();
	err = al300x_read_data_values(&data);
	if (err < 0)
		dev_err(&acc->client->dev, "get al300x data failed\n");
	else
//		al300x_report_values(acc, &data);
		al300x_report_values(acc, err);

	schedule_delayed_work(&acc->input_work,msecs_to_jiffies(poll_interval));
	mutex_unlock(&acc->lock);
}

int al300x_input_open(struct input_dev *input)
{
	struct al300x_data *acc = input_get_drvdata(input);

	return al300x_enable(acc);
}

void al300x_input_close(struct input_dev *dev)
{
	struct al300x_data *acc = input_get_drvdata(dev);

	al300x_disable(acc);	
}

static int al300x_input_init(struct al300x_data *acc)
{
	int err;
	INIT_DELAYED_WORK(&acc->input_work, al300x_input_work_func);
	acc->input_dev = input_allocate_device();
	if (!acc->input_dev) {
		err = -ENOMEM;
		dev_err(&acc->client->dev, "input device allocate failed\n");
		goto err0;
	}

	acc->input_dev->open = al300x_input_open;
	acc->input_dev->close =al300x_input_close;

	input_set_drvdata(acc->input_dev, acc);
	
	set_bit(EV_ABS, acc->input_dev->evbit);

	input_set_capability(acc->input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(acc->input_dev, ABS_MISC, 0, 9, 0, 0);

	acc->input_dev->name = "lightsensor-level";

	err = input_register_device(acc->input_dev); //??regist a input event device
	if (err) {
		dev_err(&acc->client->dev,
			"unable to register input polled device %s\n",
			acc->input_dev->name);
		goto err1;
	}
	err = sysfs_create_group(&acc->input_dev->dev.kobj,
				 &light_attribute_group);
	if (err) {
		printk("%s: could not create sysfs group\n", __func__);
		goto err1;
	}
	return 0;

err1:
	input_free_device(acc->input_dev);
err0:
	return err;
}

static int al300x_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)					

{
//	printk("++++sam debug al300x_probe-----------------\n");
	struct al300x_data *acc;
	int err = -1;
	int tempvalue;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "client not i2c capable\n");
		err = -ENODEV;
		goto exit;
	}	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		goto exit;


	acc = kzalloc(sizeof(*acc), GFP_KERNEL);
	if (acc == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, acc);
	mutex_init(&acc->lock);
	mutex_lock(&acc->lock);
	acc->client = client;

	if (i2c_smbus_read_byte(client) < 0) {	
		#if DEBUG	
		printk(KERN_ERR "i2c_smbus_read_byte error!!\n");
		#endif	
		goto exit_kfree;
	} else {	
		#if DEBUG	
		printk(KERN_INFO "al300x Device detected!\n");	
		#endif	
	}

	/* read power mode */
	tempvalue = i2c_smbus_read_word_data(client, CTRL_REG);
	if ((tempvalue & 0x00FF) == 0x0003) {
		#if DEBUG	
		printk(KERN_INFO "The Light sensor is in idle mode!\n");	
		#endif	
		} else {
		//acc->client = NULL;	
		//goto exit_kfree;
		#if DEBUG
		printk(KERN_INFO "The Light sensor is in active mode!\n");
		#endif
	}

	err = al300x_input_init(acc);
	if (err < 0)
		goto out;
//	printk("----------al300x_input_init-----------------\n");
    light_data = acc;

	err = misc_register(&al300x_misc_device);  //?? register a charactor device 
	if (err < 0) {
		dev_err(&client->dev, "al300x_device register failed\n");
		goto err4;
	}
//	printk("----------misc_register-----------------\n");
	acc->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	acc->early_suspend.suspend = al300x_early_suspend;
	acc->early_suspend.resume = al300x_late_resume;
	register_early_suspend(&acc->early_suspend);

	/* As default, do not report information */
	atomic_set(&acc->enabled, 0);

	mutex_unlock(&acc->lock);

	dev_info(&client->dev, "al300x probe success\n");

	return 0;
	
	#if DEBUG
	printk(KERN_INFO "al300x device created successfully\n");
	#endif
	return 0;
err4:
	input_unregister_device(acc->input_dev);
	input_free_device(acc->input_dev);
out:
	#if DEBUG	
	printk(KERN_ERR "%s: Driver Initialization failed\n", __FILE__);
	#endif
exit_kfree:
    kfree(acc);
exit:	
	return 0;
}

static int __devexit al300x_remove(struct i2c_client *client)
{
	struct al300x_data *acc = i2c_get_clientdata(client);
	#if DEBUG	
	printk(KERN_INFO "al300x driver removing\n");
	#endif	

	misc_deregister(&al300x_misc_device);
	sysfs_remove_group(&acc->input_dev->dev.kobj,
			   &light_attribute_group);
	input_unregister_device(acc->input_dev);
	input_free_device(acc->input_dev);

	kfree(acc);	
	return 0;
}

static void al300x_late_resume(struct early_suspend *h)
{
    	int res;
	unsigned char buf[1];
	buf[0] = 0x00;	
	
	res = al300x_i2c_write(CTRL_REG, &buf[0], 1);

	#if DEBUG	
	printk(KERN_INFO "al300x_resume\n");
	#endif		
	al300x_enable(light_data);
//	return 0;
}

static void al300x_early_suspend(struct early_suspend *h)
{	
    	int res;
	unsigned char buf[1];
   	al300x_disable(light_data); 
	buf[0] = 0x0b;	
	res = al300x_i2c_write(CTRL_REG, &buf[0], 1);
	#if DEBUG
	printk(KERN_INFO "al300x_suspend\n");
	#endif
//   	return res;
}

static const struct i2c_device_id al300x_id[] = {
	{"al300x", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, al300x_id);

static struct i2c_driver al300x_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = al300x_probe,
	.id_table = al300x_id,
	.address_list   = normal_i2c,
	.remove = __devexit_p(al300x_remove),
	.driver = {
		   .name = "al300x",
		   },
};

/**
 * ls_detect - Device detection callback for automatic device creation
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ls_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret;
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
            return -ENODEV;
//         printk(KERN_INFO "ltr303 i2c_check_functionality is ok\n");   
	if (sensor_config.twi_id == adapter->nr) {
		for (i2c_num = 0; i2c_num < (sizeof(i2c_address)/sizeof(i2c_address[0]));i2c_num++) {	    
			client->addr = i2c_address[i2c_num];
			//dprintk(DEBUG_INIT, "%s:addr= 0x%x,i2c_num:%d\n",__func__,client->addr,i2c_num);
			ret = i2c_smbus_read_byte_data(client,LTR303_MANUFACTURER_ID);
			//dprintk(DEBUG_INIT, "Read MID value is :%d",ret);
			if ((ret &0x00FF) == MANUID) {
				ret = i2c_smbus_read_byte_data(client,LTR303_PART_ID);
				//dprintk(DEBUG_INIT, "Read PID value is :%d",ret);
				if ((ret &0x00FF) == PARTID) {
					//printk("LS Device detected!\n" );
					strlcpy(info->type, DEVICE_NAME, I2C_NAME_SIZE);
					return 0;
				}
			}                                                           
		}
        
		pr_info("%s:LS Device not found, \
		maybe the other gsensor equipment! \n",__func__);
		return -ENODEV;
	} else {
		return -ENODEV;
	}
}

/**
 * ls_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:  
 *                    = 0; success;
 *                    < 0; err
 */
static int ls_fetch_sysconfig_para(void)
{
	int ret = -1;
	int device_used = -1;
	int twi_id = 0;
	int sensor_int = 0;
	script_item_u	val;
	script_item_value_type_e  type;	
		
//	printk(KERN_INFO "========%s===================\n", __func__);
	
	type = script_get_item("ls_para", "ls_used", &val);
 
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		pr_err("%s: type err  device_used = %d. \n", __func__, val.val);
		goto script_get_err;
	}
	device_used = val.val;
	if (1 == device_used) {
		type = script_get_item("ls_para", "ls_twi_id", &val);	
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
			pr_err("%s: type err twi_id = %d. \n", __func__, val.val);
			goto script_get_err;
		}
		twi_id = val.val;

		type = script_get_item("ls_para", "ls_int", &val);
//                printk(KERN_INFO "type=%d\n",type);	
		if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
//			printk("%s: type err twi int1 = %d. \n", __func__, val.gpio.gpio);
			goto script_get_err;
		}
		sensor_int = val.gpio.gpio;

		
		ret = 0;
		
	} else {
		pr_err("%s: ls_unused. \n",  __func__);
		ret = -1;
	}
	sensor_config.twi_id = twi_id;
	sensor_config.int1 = sensor_int;

	return ret;

script_get_err:
	pr_notice("=========script_get_err============\n");
	return ret;

}

static int __init al300x_init(void)
{
//	printk(KERN_INFO "al300x light sensor init\n");

// add by mark----
	if(ls_fetch_sysconfig_para())
	 {
//		printk("%s: err.\n", __func__);
		return -1;
	}
	al300x_driver.detect = ls_detect;
//end

	return i2c_add_driver(&al300x_driver);
}

static void __exit al300x_exit(void)
{
	i2c_del_driver(&al300x_driver);
	return;
}

module_init(al300x_init);
module_exit(al300x_exit);

MODULE_DESCRIPTION("al300x light sensor driver");
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");

