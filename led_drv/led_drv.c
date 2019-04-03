#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioctl.h>

#define IOCTL_SUPPORT 0

#if defined(IOCTL_SUPPORT)
struct gpio_set{
	int gpio_num;
	int gpio_val;
}gpio_ctrl_func;

static char gpio_state[4] = {0};

#define GPIOC_S_STATE _IOW('G', 0X00, struct gpio_set)
#define GPIOC_G_STATE _IOR('G', 0X01, char[4])
#define GPIOC_S_OPEN _IO('G', 0X03)
#define GPIOC_G_CLOSE _IO('G', 0X04)
#endif

/*
1.定义字符设备
*/
static struct cdev led_drv_cdev;

/*
3.申请设备号
*/
static dev_t led_drv_num;
static unsigned int led_drv_major = 0;
static unsigned int led_drv_minor = 0;

struct class *led_drv_class;
struct device *led_drv_device;

struct resource *phy_mem_res;
void __iomem *gpioc_vir_mem;
void __iomem *gpiocaltfn0_vir_mem;
void __iomem *gpiocaltfn1_vir_mem;
void __iomem *gpiocoutenb_vir_mem;
void __iomem *gpiocout_vir_mem;
/*
2.定义并初始化文件操作集
*/
int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("<4>" "led drv open\n");
	return 0;
}

ssize_t led_drv_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
	int ret;
	char __user kbuf[4]= {0};
	
	if(size != 4)
		return -EINVAL;
	
	kbuf[0] |= (7<<1);
	if((*((unsigned int *)(gpiocout_vir_mem))) & (1 << 7))
		kbuf[0] |= 0;
	else
		kbuf[0] |= 1;
	
	kbuf[1] |= (8<<1);
	if((*((unsigned int *)(gpiocout_vir_mem))) & (1 << 8))
		kbuf[1] |= 0;
	else
		kbuf[1] |= 1;
	
	kbuf[2] |= (12<<1);
	if((*((unsigned int *)(gpiocout_vir_mem))) & (1 << 12))
		kbuf[2] |= 0;
	else
		kbuf[2] |= 1;
	
	kbuf[3] |= (17<<1);
	if((*((unsigned int *)(gpiocout_vir_mem))) & (1 << 17))
		kbuf[3] |= 0;
	else
		kbuf[3] |= 1;

	ret = copy_to_user(user_buf, kbuf, 4);
	
	return ret;
}

ssize_t led_drv_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
	int ret;
	char __user kbuf[2];
	
	ret = copy_from_user(kbuf, user_buf, size);
	if(size != 2)
		return -EINVAL;
	
	switch(kbuf[0]){
		case 7:
			if(kbuf[1] == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<7);
			else if(kbuf[1] == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<7);
			else
				return -EINVAL;
			break;
			
		case 8:
			if(kbuf[1] == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<8);
			else if(kbuf[1] == 1)	
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<8);
			else
				return -EINVAL;
			break;
			
		case 12:
			if(kbuf[1] == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<12);
			else if(kbuf[1] == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<12);
			else
				return -EINVAL;
			break;
			
		case 17:
			if(kbuf[1] == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<17);
			else if(kbuf[1] == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<17);
			else
				return -EINVAL;	
			break;
			
		default:
			break;
	}
	
	return ret;
}

#if defined(IOCTL_SUPPORT)
long led_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	long ret = 0;
	struct gpio_set gpio_ctrl;

	switch(cmd){
	case GPIOC_S_STATE:
		ret = copy_from_user(&gpio_ctrl, (struct gpio_set *)args, sizeof(struct gpio_set));

		switch(gpio_ctrl.gpio_num){
		case 7:
			if(gpio_ctrl.gpio_val == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<7);
			else if(gpio_ctrl.gpio_val == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<7);
			else
				return -EINVAL;
			break;
			
		case 8:
			if(gpio_ctrl.gpio_val == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<8);
			else if(gpio_ctrl.gpio_val == 1)	
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<8);
			else
				return -EINVAL;
			break;
			
		case 12:
			if(gpio_ctrl.gpio_val == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<12);
			else if(gpio_ctrl.gpio_val == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<12);
			else
				return -EINVAL;
			break;
			
		case 17:
			if(gpio_ctrl.gpio_val == 0)
				*((unsigned int *)(gpiocout_vir_mem)) |= (1<<17);
			else if(gpio_ctrl.gpio_val == 1)
				*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<17);
			else
				return -EINVAL;	
			break;
		}
		break;
		
	case GPIOC_G_STATE:
		gpio_state[0] = !((readl(gpiocout_vir_mem)>>7)  & 0x01);
		gpio_state[1] = !((readl(gpiocout_vir_mem)>>8)  & 0x01);
		gpio_state[2] = !((readl(gpiocout_vir_mem)>>12) & 0x01);
		gpio_state[3] = !((readl(gpiocout_vir_mem)>>17) & 0x01);
		
		ret = copy_to_user((char *)args, gpio_state, sizeof(gpio_state));
		break;
		
	case GPIOC_S_OPEN:
		*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<7);
		*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<8);
		*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<12);
		*((unsigned int *)(gpiocout_vir_mem)) &= ~(1<<17);
		break;
		
	case GPIOC_G_CLOSE:
		*((unsigned int *)(gpiocout_vir_mem)) |= (1<<7);
		*((unsigned int *)(gpiocout_vir_mem)) |= (1<<8);
		*((unsigned int *)(gpiocout_vir_mem)) |= (1<<12);
		*((unsigned int *)(gpiocout_vir_mem)) |= (1<<17);
		break;
	}

	return ret;
}
#endif

int led_drv_release(struct inode *inode, struct file *filp)
{
	*((unsigned int *)(gpiocout_vir_mem)) |= (1<<7);
	*((unsigned int *)(gpiocout_vir_mem)) |= (1<<8);
	*((unsigned int *)(gpiocout_vir_mem)) |= (1<<12);
	*((unsigned int *)(gpiocout_vir_mem)) |= (1<<17);
	
	printk("<4>" "led drv close\n");
	return 0;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = led_drv_open,
	.read = led_drv_read,
	.write = led_drv_write,
	#if defined(IOCTL_SUPPORT)
	.unlocked_ioctl = led_drv_ioctl,
	#endif
	.release = led_drv_release,
};

static int __init led_drv_init(void)
{
	int ret = 0;
	
	/*得到设备号*/
	led_drv_num = MKDEV(led_drv_major, led_drv_minor);
	
	if(led_drv_major)
		ret = register_chrdev_region(led_drv_num, 1, "led_drv_num");
	else
		ret = alloc_chrdev_region(&led_drv_num, 0, 1, "led_drv_num");
	
	if(ret < 0){
		printk("<4>" "assign led_drv_nun fail\n");
		return -ENODEV;
	}
	
	/*4.初始化字符设备*/
	cdev_init(&led_drv_cdev, &fops);

	/*5.将字符设备加入内核*/
	ret = cdev_add(&led_drv_cdev, led_drv_num, 1);
	if(ret < 0){
		unregister_chrdev_region(led_drv_num, 1);
		printk("<4>" "assign led_drv_nun fail\n");
		return -EFAULT;
	}
	
	/*6.创建class*/
	led_drv_class = class_create(THIS_MODULE, "led_drv");
	if(led_drv_class == NULL){
		unregister_chrdev_region(led_drv_num, 1);
		cdev_del(&led_drv_cdev);
		printk("<4>" "led_drv_class create fail\n");
		return -EFAULT;
	}
	
	/*7.创建device*/
	led_drv_device = device_create(led_drv_class, NULL, led_drv_num, NULL, "led_drv");
	if(led_drv_device == NULL){
		class_destroy(led_drv_class);
		cdev_del(&led_drv_cdev);
		unregister_chrdev_region(led_drv_num, 1);
		printk("<4>" "led_drv_device create fail\n");
		return -EFAULT;
	}
	
	/*8.申请物理内存区*/
	phy_mem_res = request_mem_region(0xC001C000, 0x1000, "GPIOC");
	if(phy_mem_res == NULL){
		device_destroy(led_drv_class, led_drv_num);
		class_destroy(led_drv_class);
		cdev_del(&led_drv_cdev);
		unregister_chrdev_region(led_drv_num, 1);
		printk("<4>" "phy_mem_res assign fail\n");
		return -EFAULT;
	}
	
	/*9.动态映射虚拟内存区*/
		gpioc_vir_mem = ioremap(0xC001C000, 0x1000);
		if(gpioc_vir_mem == NULL){
		release_mem_region(0xC001C000, 0x1000);
		device_destroy(led_drv_class, led_drv_num);
		class_destroy(led_drv_class);
		cdev_del(&led_drv_cdev);
		unregister_chrdev_region(led_drv_num, 1);
		printk("<4>" "phy_mem_res assign fail\n");
		return -EFAULT;
	}
	
	/*10.访问虚拟内存区*/
	gpiocaltfn0_vir_mem = gpioc_vir_mem + 0x20;
	gpiocaltfn1_vir_mem = gpioc_vir_mem + 0x24;
	gpiocoutenb_vir_mem = gpioc_vir_mem + 0x04;
	gpiocout_vir_mem    = gpioc_vir_mem + 0x00;
	
	*((unsigned int *)(gpiocaltfn0_vir_mem)) &= ~((3<<14) | (3<<16) | (3<<24));
	*((unsigned int *)(gpiocaltfn0_vir_mem)) |= ((1<<14) | (1<<16) | (1<<24));
	*((unsigned int *)(gpiocaltfn1_vir_mem)) &= ~(3<<2);
	*((unsigned int *)(gpiocaltfn1_vir_mem)) |= (1<<2);
	
	*((unsigned int *)(gpiocoutenb_vir_mem)) |= ((1<<7) | (1<<8) | (1<<12) | (1<<17));
	*((unsigned int *)(gpiocout_vir_mem)) |= ((1<<7) | (1<<8) | (1<<12) | (1<<17));
	
	printk("<4>" "led drv init\n");
	
	return 0;
}

static void __exit led_drv_exit(void)
{
	/*解除映射*/
	iounmap(gpiocout_vir_mem);
	
	/*释放物理地址*/
	release_mem_region(0xC001C000, 0x1000);
	
	/*销毁device*/
	device_destroy(led_drv_class, led_drv_num);
	
	/*销毁class*/
	class_destroy(led_drv_class);
	
	/*将字符设备从内核中删除*/
	cdev_del(&led_drv_cdev);
	
	/*释放设备号*/
	unregister_chrdev_region(led_drv_num, 1);
	
	printk("<4>" "led drv exit\n");
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_AUTHOR("ayl0521@sina.com");
MODULE_DESCRIPTION("led drv");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");
