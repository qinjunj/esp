// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "rotateorder0_vivado.h"

#define DRV_NAME	"rotateorder0_vivado"

/* <<--regs-->> */
#define ROTATEORDER0_NBATCHES_REG 0x48
#define ROTATEORDER0_NCHANNELS_REG 0x44
#define ROTATEORDER0_NSAMPLES_REG 0x40

struct rotateorder0_vivado_device {
	struct esp_device esp;
};

static struct esp_driver rotateorder0_driver;

static struct of_device_id rotateorder0_device_ids[] = {
	{
		.name = "SLD_ROTATEORDER0_VIVADO",
	},
	{
		.name = "eb_066",
	},
	{
		.compatible = "sld,rotateorder0_vivado",
	},
	{ },
};

static int rotateorder0_devs;

static inline struct rotateorder0_vivado_device *to_rotateorder0(struct esp_device *esp)
{
	return container_of(esp, struct rotateorder0_vivado_device, esp);
}

static void rotateorder0_prep_xfer(struct esp_device *esp, void *arg)
{
	struct rotateorder0_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->nBatches, esp->iomem + ROTATEORDER0_NBATCHES_REG);
	iowrite32be(a->nChannels, esp->iomem + ROTATEORDER0_NCHANNELS_REG);
	iowrite32be(a->nSamples, esp->iomem + ROTATEORDER0_NSAMPLES_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool rotateorder0_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct rotateorder0_vivado_device *rotateorder0 = to_rotateorder0(esp); */
	/* struct rotateorder0_vivado_access *a = arg; */

	return true;
}

static int rotateorder0_probe(struct platform_device *pdev)
{
	struct rotateorder0_vivado_device *rotateorder0;
	struct esp_device *esp;
	int rc;

	rotateorder0 = kzalloc(sizeof(*rotateorder0), GFP_KERNEL);
	if (rotateorder0 == NULL)
		return -ENOMEM;
	esp = &rotateorder0->esp;
	esp->module = THIS_MODULE;
	esp->number = rotateorder0_devs;
	esp->driver = &rotateorder0_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	rotateorder0_devs++;
	return 0;
 err:
	kfree(rotateorder0);
	return rc;
}

static int __exit rotateorder0_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct rotateorder0_vivado_device *rotateorder0 = to_rotateorder0(esp);

	esp_device_unregister(esp);
	kfree(rotateorder0);
	return 0;
}

static struct esp_driver rotateorder0_driver = {
	.plat = {
		.probe		= rotateorder0_probe,
		.remove		= rotateorder0_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = rotateorder0_device_ids,
		},
	},
	.xfer_input_ok	= rotateorder0_xfer_input_ok,
	.prep_xfer	= rotateorder0_prep_xfer,
	.ioctl_cm	= ROTATEORDER0_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct rotateorder0_vivado_access),
};

static int __init rotateorder0_init(void)
{
	return esp_driver_register(&rotateorder0_driver);
}

static void __exit rotateorder0_exit(void)
{
	esp_driver_unregister(&rotateorder0_driver);
}

module_init(rotateorder0_init)
module_exit(rotateorder0_exit)

MODULE_DEVICE_TABLE(of, rotateorder0_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("rotateorder0_vivado driver");
