/*
 * sound\soc\sun6i\i2s\sun6i_sndi2s.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * chenpailin <chenpailin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/io.h>
#include <mach/sys_config.h>

#include "sun6i-i2s.h"
#include "sun6i-i2sdma.h"

static int i2s_used = 0;
static struct clk *xtal;
static int clk_users;
static DEFINE_MUTEX(clk_lock);

#ifdef ENFORCE_RATES
static struct snd_pcm_hw_constraint_list hw_constraints_rates = {
	.count	= ARRAY_SIZE(rates),
	.list	= rates,
	.mask	= 0,
};
#endif

static int sun6i_sndi2s_startup(struct snd_pcm_substream *substream)
{
	int ret = 0;
	#ifdef ENFORCE_RATES
		struct snd_pcm_runtime *runtime = substream->runtime;;
	#endif

	if (!ret) {
	#ifdef ENFORCE_RATES
		ret = snd_pcm_hw_constraint_list(runtime, 0,
						 SNDRV_PCM_HW_PARAM_RATE,
						 &hw_constraints_rates);
		if (ret < 0)
			return ret;
	#endif
	}
	return ret;
}

static void sun6i_sndi2s_shutdown(struct snd_pcm_substream *substream)
{
	mutex_lock(&clk_lock);
	clk_users -= 1;
	if (clk_users == 0) {
		clk_put(xtal);
		xtal = NULL;

	}
	mutex_unlock(&clk_lock);
}

static int sun6i_sndi2s_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0;
	u32 mclk = 22579200;
	unsigned long sample_rate = params_rate(params);
	
	switch(sample_rate)
	{
		case 8000:
		case 16000:
		case 32000:
		case 64000:
		case 128000:
		case 12000:
		case 24000:
		case 48000:
		case 96000:
		case 192000:
			mclk = 24576000;
			break;
	}
	
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0 , mclk, 0);
	if (ret < 0)
		return ret;
		
	ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, sample_rate);
	if (ret < 0)
		return ret;
	return 0;
}

static struct snd_soc_ops sun6i_sndi2s_ops = {
	.startup 		= sun6i_sndi2s_startup,
	.shutdown 		= sun6i_sndi2s_shutdown,
	.hw_params 		= sun6i_sndi2s_hw_params,
};

static struct snd_soc_dai_link sun6i_sndi2s_dai_link = {
	.name 			= "I2S",
	.stream_name 	= "SUN6I-I2S",
	.cpu_dai_name 	= "sun6i-i2s.0",
	.codec_dai_name = "sndi2s",
	.platform_name 	= "sun6i-i2s-pcm-audio.0",	
	.codec_name 	= "sun6i-i2s-codec.0",
	.ops 			= &sun6i_sndi2s_ops,
};

static struct snd_soc_card snd_soc_sun6i_sndi2s = {
	.name = "sndi2s",
	.owner 		= THIS_MODULE,
	.dai_link = &sun6i_sndi2s_dai_link,
	.num_links = 1,
};

static struct platform_device *sun6i_sndi2s_device;

static int __init sun6i_sndi2s_init(void)
{
	int ret = 0;
	script_item_u val;
	script_item_value_type_e  type;

	type = script_get_item("i2s_para", "i2s_used", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        printk("[I2S] type err!\n");
    }

	i2s_used = val.val;
    if (i2s_used) {
		sun6i_sndi2s_device = platform_device_alloc("soc-audio", 2);
		if(!sun6i_sndi2s_device)
			return -ENOMEM;
		platform_set_drvdata(sun6i_sndi2s_device, &snd_soc_sun6i_sndi2s);
		ret = platform_device_add(sun6i_sndi2s_device);
		if (ret) {
			platform_device_put(sun6i_sndi2s_device);
		}
	}else{
		printk("[I2S]sun6i_sndi2s cannot find any using configuration for controllers, return directly!\n");
        return 0;
	}
	return ret;
}

static void __exit sun6i_sndi2s_exit(void)
{
	if(i2s_used) {
		i2s_used = 0;
		platform_device_unregister(sun6i_sndi2s_device);
	}
}

module_init(sun6i_sndi2s_init);
module_exit(sun6i_sndi2s_exit);

MODULE_AUTHOR("ALL WINNER");
MODULE_DESCRIPTION("SUN6I_sndi2s ALSA SoC audio driver");
MODULE_LICENSE("GPL");
