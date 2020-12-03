/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <devicetree.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <console/console.h>
#include <console/tty.h>
#include <string.h>


/* 1000 msec = 1 sec */
#define SLEEP_TIME  100
#define UART_DEVICE_NAME CONFIG_UART_CONSOLE_ON_DEV_NAME
//#define UART_DEVICE_NAME "UART_1"

const struct device *uart_dev;
static volatile bool data_transmitted;
static int char_sent;
static const char fifo_data[] = "Prueba Tx.\r\n";
static int tx_data_idx;
static volatile bool data_transmitted;

#define DATA_SIZE	(sizeof(fifo_data) - 1)

// Init
void uart_init()
{
struct uart_config uart_cfg;
	int ret;
	uart_dev = device_get_binding(UART_DEVICE_NAME);
	ret = uart_config_get(uart_dev, &uart_cfg);
	if (!ret) {
		printk("\n======== [%s] ========\n", UART_DEVICE_NAME);
		printk("[%s] uart_config.baudrate=%d\n", UART_DEVICE_NAME, uart_cfg.baudrate);
		printk("[%s] uart_config.parity=%d\n", UART_DEVICE_NAME, uart_cfg.parity);
		printk("[%s] uart_config.stop_bits=%d\n", UART_DEVICE_NAME, uart_cfg.stop_bits);
		printk("[%s] uart_config.data_bits=%d\n", UART_DEVICE_NAME, uart_cfg.data_bits);
		printk("[%s] uart_config.flow_ctrl=%d\n\n\n", UART_DEVICE_NAME, uart_cfg.flow_ctrl);
	}
}




static void uart_fifo_callback(const struct device *dev, void *user_data)
{
	
	ARG_UNUSED(user_data);
	if (!uart_irq_update(dev)) {
		printk("retval should always be 1\n");
		return;
	}
	
	uart_irq_tx_enable(uart_dev);
	if (uart_irq_tx_ready(dev) && tx_data_idx < DATA_SIZE) {

		if (uart_fifo_fill(dev,
				   (uint8_t *)&fifo_data[tx_data_idx++], 1) > 0) {
			data_transmitted = true;
			char_sent++;
		}
		
		if (tx_data_idx == DATA_SIZE) {
			uart_irq_tx_disable(dev);
		}
	}
}

// Main
void main(void)
{

	printk("Hello World! \n");
	
	uart_init();
	

	while (1) {

		k_msleep(SLEEP_TIME);

		char_sent = 0;
		
		/* Verify uart_irq_callback_set() */
		uart_irq_callback_set(uart_dev, uart_fifo_callback);
		
		/* Enable Tx/Rx interrupt before using fifo */
		/* Verify uart_irq_tx_enable() */
		uart_irq_tx_enable(uart_dev);
		
		k_sleep(K_MSEC(500));
		
		tx_data_idx = 0;
		
		/* Verify uart_irq_tx_disable() */
		uart_irq_tx_disable(uart_dev);
	}
}




