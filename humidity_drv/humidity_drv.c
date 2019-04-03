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

#define DEVICE_NAME "humidity_drv"
#define GPIOC_CFG_HUMIDITY (PAD_GPIO_C + 11)

unsigned long receive_value;
unsigned long receive_jy;

static int humidity_data_in(void)
{
	gpio_direction_input(GPIOC_CFG_HUMIDITY);
	return gpio_get_value(GPIOC_CFG_HUMIDITY);
}

static void humidity_data_out(int data_out)
{
	gpio_direction_output(GPIOC_CFG_HUMIDITY, data_out);
}

static void humidity_read_data(void)
{
	unsigned int flag = 0;
	unsigned int u32i = 0;

    receive_value = 0;
    receive_jy = 0;
		
    humidity_data_out(0);
	mdelay(20);
	humidity_data_out(1);
	udelay(40);
	
	if (humidity_data_in() == 0)
	{
        flag = 0;
		//printk("<4>" "read data start\n");
        while(humidity_data_in() == 0) 
        {
            udelay(10);
            flag++;
            if(flag > 10)
				return;
        }
		//printk("<4>" "read data 1\n");
		
        flag  = 0;
		
        while(humidity_data_in() == 1)
        {
            udelay(10);
            flag++;
            if(flag > 10)
				return;
        }
		
        flag  = 0;
        
		printk("<4>" "read data 2\n");
		
		for (u32i=0x80000000; u32i>0; u32i>>=1)
		{
		    flag = 0;
			
            while(humidity_data_in() == 0) 
            {
                udelay(10);
                flag++;
				
                if(flag > 10)
                    break;
            }
			
			//udelay(20);
			printk("<4>" "flag 1 = %d\n", flag);
			
			flag = 0;
			
			while( humidity_data_in() == 1)
			{
				udelay(10);
				flag++;
				
                if(flag > 10)
                    break;
			}
			
			//printk("<4>" "read data 4\n");
			
			if(flag > 5)
			{
				receive_value |= u32i;
			}
		}

		for (u32i=0x80; u32i>0; u32i>>=1)
		{
		    flag = 0;
			
            while(humidity_data_in() == 0)	
            {
                udelay(10);
                flag++;
				
                if(flag > 10)
                     break;
            }
			
			//printk("<4>" "read data 5\n");
			
			flag = 0;
			
			while( humidity_data_in() == 1)
			{
				udelay(10);
				flag++;
                if(flag > 10)
                    break;
			}
			
			//printk("<4>" "read data 6\n");
			
			if(flag > 5) 
			{
				receive_jy |= u32i;
			}
		}
	}
}

int humidity_drv_open(struct inode *inode, struct file *filp)
{
	printk(KERN_WARNING "humidity drv open\n");
	
	return 0;
}

ssize_t humidity_drv_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
	int ret = 0;
	unsigned char tempz = 0;
	unsigned char tempx = 0;
	unsigned char humidityz = 0;
	unsigned char humidityx = 0;
    unsigned char  ecc,jy;

    humidity_read_data();

    humidityz = (receive_value & 0xff000000)>>24;
	humidityx = (receive_value & 0x00ff0000)>>16;
	tempz = (receive_value & 0x0000ff00)>>8;
	tempx = (receive_value & 0x000000ff);
	
    jy = receive_jy & 0xff;
        
    ecc = humidityz + humidityx + tempz + tempx;
	
	printk("<4>" "jy = %d ecc = %d\n", jy, ecc);
	
    if(ecc != jy)
        return -EAGAIN;
	
	ret = copy_to_user(user_buf,&receive_value,sizeof (receive_value));
	
	return 0;
}

static int humidity_drv_close(struct inode *inode, struct file *filp)
{
	printk(KERN_WARNING "humidity drv close\n");
	
	return 0;
}

static struct file_operations humidity_drv_fops = {
	.owner = THIS_MODULE,
	.open  = humidity_drv_open,
	.read  = humidity_drv_read,
	.release = humidity_drv_close
};

static struct miscdevice humidity_drv_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &humidity_drv_fops
};

static int __init humidity_drv_init(void)
{
	int ret = 0;
	
	ret = gpio_request(GPIOC_CFG_HUMIDITY, DEVICE_NAME);
	if(ret < 0){
		printk("<4>" "gpio request %s %d fail\n", DEVICE_NAME, GPIOC_CFG_HUMIDITY);
		return ret;
	}
	
	humidity_data_out(1);
	
	ret = misc_register(&humidity_drv_miscdev);
	if(ret < 0){
		printk("<4>" "%s register miscdevice fail, gpio = %d\n", DEVICE_NAME, GPIOC_CFG_HUMIDITY);
		return ret;
	}

	printk("<4>" "humidity drv init\n");
	
	return 0;
}

static void __exit humidity_drv_exit(void)
{
	gpio_free(GPIOC_CFG_HUMIDITY);
	
	misc_deregister(&humidity_drv_miscdev);
	
	printk("<4>" "humidity drv exit\n");
}

module_init(humidity_drv_init);
module_exit(humidity_drv_exit);

MODULE_AUTHOR("ayl0521@sina.com");
MODULE_DESCRIPTION("humidity drv");
MODULE_LICENSE("GPL");
