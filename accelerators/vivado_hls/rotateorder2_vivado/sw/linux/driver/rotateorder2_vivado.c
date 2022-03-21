// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "rotateorder2_vivado.h"

#define DRV_NAME	"rotateorder2_vivado"

/* <<--regs-->> */
#define ROTATEORDER2_NBATCHES_REG 0x48
#define ROTATEORDER2_CHANNELS_REG 0x44
#define ROTATEORDER2_NSAMPLES_REG 0x40

struct rotateorder2_vivado_device {
	struct esp_device esp;
};

static struct esp_driver rotateorder2_driver;

static struct of_device_id rotateorder2_device_ids[] = {
	{
		.name = "SLD_ROTATEORDER2_VIVADO",
	},
	{
		.name = "eb_054",
	},
	{
		.compatible = "sld,rotateorder2_vivado",
	},
	{ },
};

static int rotateorder2_devs;

static inline struct rotateorder2_vivado_device *to_rotateorder2(struct esp_device *esp)
{
	return container_of(esp, struct rotateorder2_vivado_device, esp);
}

static void rotateorder2_prep_xfer(struct esp_device *esp, void *arg)
{
	struct rotateorder2_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->nBatches, esp->iomem + ROTATEORDER2_NBATCHES_REG);
	iowrite32be(a->channels, esp->iomem + ROTATEORDER2_CHANNELS_REG);
	iowrite32be(a->nSamples, esp->iomem + ROTATEORDER2_NSAMPLES_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool rotateorder2_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct rotateorder2_vivado_device *rotateorder2 = to_rotateorder2(esp); */
	/* struct rotateorder2_vivado_access *a = arg; */

	return true;
}

static int rotateorder2_probe(struct platform_device *pdev)
{
	struct rotateorder2_vivado_device *rotateorder2;
	struct esp_device *esp;
	int rc;

	rotateorder2 = kzalloc(sizeof(*rotateorder2), GFP_KERNEL);
	if (rotateorder2 == NULL)
		return -ENOMEM;
	esp = &rotateorder2->esp;
	esp->module = THIS_MODULE;
	esp->number = rotateorder2_devs;
	esp->driver = &rotateorder2_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	rotateorder2_devs++;
	return 0;
 err:
	kfree(rotateorder2);
	return rc;
}

static int __exit rotateorder2_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct rotateorder2_vivado_device *rotateorder2 = to_rotateorder2(esp);

	esp_device_unregister(esp);
	kfree(rotateorder2);
	return 0;
}

static struct esp_driver rotateorder2_driver = {
	.plat = {
		.probe		= rotateorder2_probe,
		.remove		= rotateorder2_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = rotateorder2_device_ids,
		},
	},
	.xfer_input_ok	= rotateorder2_xfer_input_ok,
	.prep_xfer	= rotateorder2_prep_xfer,
	.ioctl_cm	= ROTATEORDER2_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct rotateorder2_vivado_access),
};

static int __init rotateorder2_init(void)
{
	return esp_driver_register(&rotateorder2_driver);
}

static void __exit rotateorder2_exit(void)
{
	esp_driver_unregister(&rotateorder2_driver);
}

module_init(rotateorder2_init)
module_exit(rotateorder2_exit)

MODULE_DEVICE_TABLE(of, rotateorder2_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("rotateorder2_vivado driver");
