#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "ultra_drv"
#define GPIOC_CFG_ULTRA_TRIG (PAD_GPIO_B + 28)
#define GPIOC_CFG_ULTRA_ECHO (PAD_GPIO_B + 29)

static int echo_data_in(void)
{
	gpio_direction_input(GPIOC_CFG_ULTRA_ECHO);
	return gpio_get_value(GPIOC_CFG_ULTRA_ECHO);
}

static int ultra_read_data(void)
{
	unsigned int time_cnt = 0;
	unsigned int distance = 0;
	int ret = 0;
	
	ret = echo_data_in();

	if(ret){
		while(echo_data_in() && ((time_cnt++) < 50000))
		{
			udelay(10);
		}
		
		distance = ((time_cnt * 3) / 2);
	}

	return (distance);
}

int ultra_drv_open(struct inode *inode, struct file *filp)
{
	printk("<4>" "ultra drv open\n");
	
	return 0;
}

ssize_t ultra_drv_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
	int ret = 0;
	unsigned int distance = 0;
	
	if(size != 4)
		return -EINVAL;
	
	gpio_direction_output(GPIOC_CFG_ULTRA_TRIG, 1);
	udelay(20);
	gpio_set_value(GPIOC_CFG_ULTRA_TRIG, 0);
	
	distance = ultra_read_data();
	
	ret = copy_to_user((int *)user_buf, &distance, sizeof(distance));
	
	return 0;
}

static struct file_operations ultra_drv_fops = {
	.owner = THIS_MODULE,
	.open  = ultra_drv_open,
	.read  = ultra_drv_read
};

static struct miscdevice ultra_drv_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &ultra_drv_fops	
}; 

static int __init ultra_drv_init(void)
{
	int ret = 0;
	
	gpio_free(GPIOC_CFG_ULTRA_TRIG);
	
	ret = gpio_request(GPIOC_CFG_ULTRA_TRIG, "ultra_drv");
	if(ret < 0){
		printk("<4>" "gpio_request %s %d fail\n", DEVICE_NAME, GPIOC_CFG_ULTRA_TRIG);
		return ret;
	}

	ret = misc_register(&ultra_drv_miscdev);
	if(ret < 0){
		printk("<4>" "misc_register fail\n");
		return ret;
	}
	
	printk("<4>" "ultra drv init\n");
	
	return 0;
}

static void __exit ultra_drv_exit(void)
{
	gpio_free(GPIOC_CFG_ULTRA_TRIG);
	
	misc_deregister(&ultra_drv_miscdev);
	
	printk("<4>" "ultra drv exit\n");
}

module_init(ultra_drv_init);
module_exit(ultra_drv_exit);

MODULE_AUTHOR("ayl0521@sina.com");
MODULE_DESCRIPTION("ultrasonic drv");
MODULE_LICENSE("GPL");
