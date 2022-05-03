// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "rotateorder3_vivado.h"

#define DRV_NAME	"rotateorder3_vivado"

/* <<--regs-->> */
#define ROTATEORDER3_NBATCHES_REG 0x90
#define ROTATEORDER3_COSA1_REG 0x8c
#define ROTATEORDER3_COSA3_REG 0x88
#define ROTATEORDER3_COSA2_REG 0x84
#define ROTATEORDER3_NCHANNELS_REG 0x80
#define ROTATEORDER3_SINB3_REG 0x7c
#define ROTATEORDER3_SINB2_REG 0x78
#define ROTATEORDER3_SINB1_REG 0x74
#define ROTATEORDER3_SINA2_REG 0x70
#define ROTATEORDER3_SINA3_REG 0x6c
#define ROTATEORDER3_SINA1_REG 0x68
#define ROTATEORDER3_COSG3_REG 0x64
#define ROTATEORDER3_COSG2_REG 0x60
#define ROTATEORDER3_COSG1_REG 0x5c
#define ROTATEORDER3_NSAMPLES_REG 0x58
#define ROTATEORDER3_SING1_REG 0x54
#define ROTATEORDER3_SING2_REG 0x50
#define ROTATEORDER3_SING3_REG 0x4c
#define ROTATEORDER3_COSB1_REG 0x48
#define ROTATEORDER3_COSB2_REG 0x44
#define ROTATEORDER3_COSB3_REG 0x40

struct rotateorder3_vivado_device {
	struct esp_device esp;
};

static struct esp_driver rotateorder3_driver;

static struct of_device_id rotateorder3_device_ids[] = {
	{
		.name = "SLD_ROTATEORDER3_VIVADO",
	},
	{
		.name = "eb_060",
	},
	{
		.compatible = "sld,rotateorder3_vivado",
	},
	{ },
};

static int rotateorder3_devs;

static inline struct rotateorder3_vivado_device *to_rotateorder3(struct esp_device *esp)
{
	return container_of(esp, struct rotateorder3_vivado_device, esp);
}

static void rotateorder3_prep_xfer(struct esp_device *esp, void *arg)
{
	struct rotateorder3_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->nBatches, esp->iomem + ROTATEORDER3_NBATCHES_REG);
	iowrite32be(a->cosA1, esp->iomem + ROTATEORDER3_COSA1_REG);
	iowrite32be(a->cosA3, esp->iomem + ROTATEORDER3_COSA3_REG);
	iowrite32be(a->cosA2, esp->iomem + ROTATEORDER3_COSA2_REG);
	iowrite32be(a->nChannels, esp->iomem + ROTATEORDER3_NCHANNELS_REG);
	iowrite32be(a->sinB3, esp->iomem + ROTATEORDER3_SINB3_REG);
	iowrite32be(a->sinB2, esp->iomem + ROTATEORDER3_SINB2_REG);
	iowrite32be(a->sinB1, esp->iomem + ROTATEORDER3_SINB1_REG);
	iowrite32be(a->sinA2, esp->iomem + ROTATEORDER3_SINA2_REG);
	iowrite32be(a->sinA3, esp->iomem + ROTATEORDER3_SINA3_REG);
	iowrite32be(a->sinA1, esp->iomem + ROTATEORDER3_SINA1_REG);
	iowrite32be(a->cosG3, esp->iomem + ROTATEORDER3_COSG3_REG);
	iowrite32be(a->cosG2, esp->iomem + ROTATEORDER3_COSG2_REG);
	iowrite32be(a->cosG1, esp->iomem + ROTATEORDER3_COSG1_REG);
	iowrite32be(a->nSamples, esp->iomem + ROTATEORDER3_NSAMPLES_REG);
	iowrite32be(a->sinG1, esp->iomem + ROTATEORDER3_SING1_REG);
	iowrite32be(a->sinG2, esp->iomem + ROTATEORDER3_SING2_REG);
	iowrite32be(a->sinG3, esp->iomem + ROTATEORDER3_SING3_REG);
	iowrite32be(a->cosB1, esp->iomem + ROTATEORDER3_COSB1_REG);
	iowrite32be(a->cosB2, esp->iomem + ROTATEORDER3_COSB2_REG);
	iowrite32be(a->cosB3, esp->iomem + ROTATEORDER3_COSB3_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool rotateorder3_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct rotateorder3_vivado_device *rotateorder3 = to_rotateorder3(esp); */
	/* struct rotateorder3_vivado_access *a = arg; */

	return true;
}

static int rotateorder3_probe(struct platform_device *pdev)
{
	struct rotateorder3_vivado_device *rotateorder3;
	struct esp_device *esp;
	int rc;

	rotateorder3 = kzalloc(sizeof(*rotateorder3), GFP_KERNEL);
	if (rotateorder3 == NULL)
		return -ENOMEM;
	esp = &rotateorder3->esp;
	esp->module = THIS_MODULE;
	esp->number = rotateorder3_devs;
	esp->driver = &rotateorder3_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	rotateorder3_devs++;
	return 0;
 err:
	kfree(rotateorder3);
	return rc;
}

static int __exit rotateorder3_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct rotateorder3_vivado_device *rotateorder3 = to_rotateorder3(esp);

	esp_device_unregister(esp);
	kfree(rotateorder3);
	return 0;
}

static struct esp_driver rotateorder3_driver = {
	.plat = {
		.probe		= rotateorder3_probe,
		.remove		= rotateorder3_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = rotateorder3_device_ids,
		},
	},
	.xfer_input_ok	= rotateorder3_xfer_input_ok,
	.prep_xfer	= rotateorder3_prep_xfer,
	.ioctl_cm	= ROTATEORDER3_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct rotateorder3_vivado_access),
};

static int __init rotateorder3_init(void)
{
	return esp_driver_register(&rotateorder3_driver);
}

static void __exit rotateorder3_exit(void)
{
	esp_driver_unregister(&rotateorder3_driver);
}

module_init(rotateorder3_init)
module_exit(rotateorder3_exit)

MODULE_DEVICE_TABLE(of, rotateorder3_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("rotateorder3_vivado driver");
