/*
 * ASoC Driver for AudioCard
 *
 * Author:	Henrik Langer <henni19790@googlemail.com>
 *		Copyright 2015
 *              based on RaspiDAC3 driver by Jan Grulich <jan@grulich.eu>
 *		based on BlackFin-AD193x driver by Barry Song <Barry.Song@analog.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
 
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>

//TODO: May include dma, irq headers, etc. (see bf5xx-ad193x.c)

#include "../codecs/ad193x.h"

/* sound card init */
static int snd_rpi_audiocard_init(struct snd_soc_pcm_runtime *rtd)
{

	return 0;
}

/* set hw parameters */
static int snd_rpi_raspidac3_hw_params(struct snd_pcm_substream *substream,
				       struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	unsigned int sample_bits =
		snd_pcm_format_physical_width(params_format(params));

	return snd_soc_dai_set_bclk_ratio(cpu_dai, sample_bits * 2);
}

/* startup */
static int snd_rpi_raspidac3_startup(struct snd_pcm_substream *substream) {
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	snd_soc_update_bits(codec, PCM512x_GPIO_CONTROL_1, 0x08,0x08);
	tpa6130a2_stereo_enable(codec, 1);
	return 0;
}

/* shutdown */
static void snd_rpi_raspidac3_shutdown(struct snd_pcm_substream *substream) {
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	snd_soc_update_bits(codec, PCM512x_GPIO_CONTROL_1, 0x08,0x00);
	tpa6130a2_stereo_enable(codec, 0);
}

/* machine stream operations */
static struct snd_soc_ops snd_rpi_raspidac3_ops = {
	.hw_params = snd_rpi_raspidac3_hw_params,
	.startup = snd_rpi_raspidac3_startup,
	.shutdown = snd_rpi_raspidac3_shutdown,
};

/* interface setup */
static struct snd_soc_dai_link snd_rpi_audiocard_dai[] = {
	{
		.name = "AudioCard",
		.stream_name = "AudioCard HiFi",
		.cpu_dai_name = "bcm2708-i2s.0",
		.codec_dai_name ="ad193x-hifi",
		.platform_name = "bcm2708-i2s.0",
		.codec_name = "spi0.5",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF | \
				SND_SOC_DAIFMT_CBM_CFM,
		.ops = &snd_rpi_raspidac3_ops,
		.init = snd_rpi_audiocard_init,
	},
};

/* audio machine driver */
static struct snd_soc_card bf5xx_ad193x = {
	.name = "snd_rpi_audiocard",
	.owner = THIS_MODULE,
	.dai_link = snd_rpi_audiocard_dai,
	.num_links = ARRAY_SIZE(snd_rpi_audiocard_dai),
};

/* sound card disconnect */
static int snd_rpi_raspidac3_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_card(&snd_rpi_raspidac3);
}

static const struct of_device_id raspidac3_of_match[] = {
	{ .compatible = "jg,raspidacv3", },
	{},
};
MODULE_DEVICE_TABLE(of, raspidac3_of_match);

/* sound card platform driver */
static struct platform_driver snd_rpi_raspidac3_driver = {
	.driver = {
		.name   = "snd-rpi-raspidac3",
		.owner  = THIS_MODULE,
		.of_match_table = raspidac3_of_match,
	},
	.probe          = snd_rpi_raspidac3_probe,
	.remove         = snd_rpi_raspidac3_remove,
};

module_platform_driver(snd_rpi_raspidac3_driver);

/* Module information */
MODULE_AUTHOR("Barry Song");
MODULE_DESCRIPTION("ALSA SoC AD193X board driver");
MODULE_LICENSE("GPL");
