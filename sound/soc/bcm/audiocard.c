#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>

//TODO: May include dma, irq headers, etc. (see bf5xx-ad193x.c)

#include "../codecs/ad193x.h"

static struct snd_soc_card bf5xx_ad193x;

static int snd_rpi_audiocard_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret;

	/* set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 24576000, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* set codec DAI slots, 8 channels, all channels are enabled */
	ret = snd_soc_dai_set_tdm_slot(codec_dai, 0xFF, 0xFF, 8, 32);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_tdm_slot(cpu_dai, 0xFF, 0xFF, 8, 32);
	if (ret < 0)
		return ret;

	return 0;
}

#define SND_RPI_AUDIOCARD_DAIFMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF | \
				SND_SOC_DAIFMT_CBM_CFM)

static struct snd_soc_dai_link snd_rpi_audiocard_dai[] = {
	{
		.name = "AudioCard",
		.stream_name = "AudioCard HiFi",
		.cpu_dai_name = "bcm2708-i2s.0",
		.codec_dai_name ="ad193x-hifi",
		.platform_name = "bcm2708-i2s.0",
		.codec_name = "spi0.5",
		.dai_fmt = SND_RPI_AUDIOCARD_DAIFMT,
		.init = snd_rpi_audiocard_init,
	},
};

static struct snd_soc_card bf5xx_ad193x = {
	.name = "snd_rpi_audiocard",
	.owner = THIS_MODULE,
	.dai_link = snd_rpi_audiocard_dai,
	.num_links = ARRAY_SIZE(snd_rpi_audiocard_dai),
};

static struct platform_device *bfxx_ad193x_snd_device;

static int __init bf5xx_ad193x_init(void)
{
	int ret;

	bfxx_ad193x_snd_device = platform_device_alloc("soc-audio", -1);
	if (!bfxx_ad193x_snd_device)
		return -ENOMEM;

	platform_set_drvdata(bfxx_ad193x_snd_device, &bf5xx_ad193x);
	ret = platform_device_add(bfxx_ad193x_snd_device);

	if (ret)
		platform_device_put(bfxx_ad193x_snd_device);

	return ret;
}

static void __exit bf5xx_ad193x_exit(void)
{
	platform_device_unregister(bfxx_ad193x_snd_device);
}

module_init(bf5xx_ad193x_init);
module_exit(bf5xx_ad193x_exit);

/* Module information */
MODULE_AUTHOR("Barry Song");
MODULE_DESCRIPTION("ALSA SoC AD193X board driver");
MODULE_LICENSE("GPL");
