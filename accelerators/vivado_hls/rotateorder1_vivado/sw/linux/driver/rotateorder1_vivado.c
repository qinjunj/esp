// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "rotateorder1_vivado.h"

#define DRV_NAME	"rotateorder1_vivado"

/* <<--regs-->> */
#define ROTATEORDER1_NSAMPLES_REG 0x40

struct rotateorder1_vivado_device {
	struct esp_device esp;
};

static struct esp_driver rotateorder1_driver;

static struct of_device_id rotateorder1_device_ids[] = {
	{
		.name = "SLD_ROTATEORDER1_VIVADO",
	},
	{
		.name = "eb_060",
	},
	{
		.compatible = "sld,rotateorder1_vivado",
	},
	{ },
};

static int rotateorder1_devs;

static inline struct rotateorder1_vivado_device *to_rotateorder1(struct esp_device *esp)
{
	return container_of(esp, struct rotateorder1_vivado_device, esp);
}

static void rotateorder1_prep_xfer(struct esp_device *esp, void *arg)
{
	struct rotateorder1_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->nSamples, esp->iomem + ROTATEORDER1_NSAMPLES_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool rotateorder1_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct rotateorder1_vivado_device *rotateorder1 = to_rotateorder1(esp); */
	/* struct rotateorder1_vivado_access *a = arg; */

	return true;
}

static int rotateorder1_probe(struct platform_device *pdev)
{
	struct rotateorder1_vivado_device *rotateorder1;
	struct esp_device *esp;
	int rc;

	rotateorder1 = kzalloc(sizeof(*rotateorder1), GFP_KERNEL);
	if (rotateorder1 == NULL)
		return -ENOMEM;
	esp = &rotateorder1->esp;
	esp->module = THIS_MODULE;
	esp->number = rotateorder1_devs;
	esp->driver = &rotateorder1_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	rotateorder1_devs++;
	return 0;
 err:
	kfree(rotateorder1);
	return rc;
}

static int __exit rotateorder1_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct rotateorder1_vivado_device *rotateorder1 = to_rotateorder1(esp);

	esp_device_unregister(esp);
	kfree(rotateorder1);
	return 0;
}

static struct esp_driver rotateorder1_driver = {
	.plat = {
		.probe		= rotateorder1_probe,
		.remove		= rotateorder1_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = rotateorder1_device_ids,
		},
	},
	.xfer_input_ok	= rotateorder1_xfer_input_ok,
	.prep_xfer	= rotateorder1_prep_xfer,
	.ioctl_cm	= ROTATEORDER1_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct rotateorder1_vivado_access),
};

static int __init rotateorder1_init(void)
{
	return esp_driver_register(&rotateorder1_driver);
}

static void __exit rotateorder1_exit(void)
{
	esp_driver_unregister(&rotateorder1_driver);
}

module_init(rotateorder1_init)
module_exit(rotateorder1_exit)

MODULE_DEVICE_TABLE(of, rotateorder1_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("rotateorder1_vivado driver");
