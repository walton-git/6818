#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define GPIOC_PHY_ADDR 0xC001C000
#define GPIOC_PHY_ADDR_SIZE 0x1000

#define BEEP_S_OPEN _IO('B', 0X00)
#define BBEP_S_CLOSE _IO('B', 0X01)

/*
1.定义字符设备
*/
static struct cdev beep_drv_cdev;

/*
3.定义设备号
*/
unsigned int major = 0; 
unsigned int minor = 0;

dev_t beep_drv_num;

struct class *beep_drv_class;
struct device *beep_drv_device;

struct resource *phy_addr_res;
 
void __iomem *gpio_vir_addr_base;

int beep_drv_open(struct inode *inode, struct file *filp)
{
	printk(KERN_WARNING "beep drv open\n");
	return 0;
}

ssize_t beep_drv_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
	int ret;
	char kbuf[1] = {0};
	
	if(size != 1)
		return -EINVAL;
	
	if((readl(gpio_vir_addr_base) >> 14) & 0x01)
		kbuf[0] = 1;
	else
		kbuf[0] = 0;
	
	ret = copy_to_user(user_buf, kbuf, 1);
	
	return ret;
}

ssize_t beep_drv_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
	int ret,temp;
	char kbuf[1] = {0};
	
	if(size != 1)
		return -EINVAL;
	
	ret = copy_from_user(kbuf, user_buf, 1);

	if(kbuf[0]){  //输出高电平
		temp = readl(gpio_vir_addr_base);
		temp |= (1<<14);
		writel(temp, gpio_vir_addr_base);
	} else {     //输出低电平
		temp = readl(gpio_vir_addr_base);
		temp &= ~(1<<14);
		writel(temp, gpio_vir_addr_base);
	}

	return ret;
}

long beep_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	int temp;
	switch(cmd){
	case  BEEP_S_OPEN:
		temp = readl(gpio_vir_addr_base);
		temp |= (1<<14);
		writel(temp, gpio_vir_addr_base);
		break;
		
	case BBEP_S_CLOSE:
		temp = readl(gpio_vir_addr_base);
		temp &= ~(1<<14);
		writel(temp, gpio_vir_addr_base);
		break;
	}
	
	printk("<4>" "beep end\n");
	
	return 0;
}

int beep_drv_release(struct inode *inode, struct file *filp)
{
	int temp;
	
	temp = readl(gpio_vir_addr_base);
	temp &= ~(1<<14);
	writel(temp, gpio_vir_addr_base);
		
	printk(KERN_WARNING "beep drv close\n");
	return 0;
}

/*
2.定义文件操作集
*/
static const struct file_operations beep_drv_fops = {
	.owner   = THIS_MODULE,
	.open    = beep_drv_open,
	.read    = beep_drv_read,
	.write   = beep_drv_write,
	.unlocked_ioctl = beep_drv_ioctl,
	.release = beep_drv_release
};

static int __init beep_drv_init(void)
{
	int ret,temp;
	
	/*申请设备号*/
	beep_drv_num = MKDEV(major, minor);
	if(major)
		ret = register_chrdev_region(beep_drv_num, 1, "beep_drv_num");
	else
		ret = alloc_chrdev_region(&beep_drv_num, 0, 1, "beep_drv_num");
	
	if(ret < 0){
		printk("<4>" "register chrdev fail\n");
		return ret;
	}
	
	/*4.字符设备初始化*/
	cdev_init(&beep_drv_cdev, &beep_drv_fops);
	
	/*5.将字符设备加入内核*/
	ret = cdev_add(&beep_drv_cdev, beep_drv_num, 1);
	if(ret < 0){
		printk("<4>" "cdev add fail\n");
		goto cdev_add_error;
	}
	
	/*6.创建class*/
	beep_drv_class = class_create(THIS_MODULE, "beep_drv");
	if(beep_drv_class == NULL){
		printk("<4>" "class add fail\n");
		ret = -EFAULT;
		goto class_create_error;
	}
	
	/*7.创建device*/
	beep_drv_device = device_create(beep_drv_class, NULL, beep_drv_num, NULL, "beep_drv");
	if(beep_drv_device == NULL){
		printk("<4>" "device add fail\n");
		ret = -EFAULT;
		goto device_create_error;
	}
#if 0	
	/*申请物理内存区*/
	phy_addr_res = request_mem_region(GPIOC_PHY_ADDR, GPIOC_PHY_ADDR_SIZE, "GPIOC");
	if(phy_addr_res == NULL){
		printk("<4>" "phy_addr_res request fail\n");
		ret = -EFAULT;
		goto request_phy_addr_error;
	}
#endif	
	/*动态映射物理内存区，得到虚拟地址*/
	gpio_vir_addr_base = ioremap(GPIOC_PHY_ADDR, GPIOC_PHY_ADDR_SIZE);
	if(gpio_vir_addr_base == NULL){
		printk("<4>" "gpio_vir_addr_base remap fail\n");
		ret = -EFAULT;
		goto gpio_vir_addr_base_error;
	}
	
	temp = readl(gpio_vir_addr_base+0x20);
	temp &= ~(3<<28);
	temp |= (1<<28);
	writel(temp, gpio_vir_addr_base+0x20);
	
	temp = readl(gpio_vir_addr_base+0x04);
	temp |= (1<<14);
	writel(temp, gpio_vir_addr_base+0x04);	
	
	temp = readl(gpio_vir_addr_base);
	temp &= ~(1<<14);
	writel(temp, gpio_vir_addr_base);

	printk("<4>" "beep drv init\n");
	
	return 0;
	
gpio_vir_addr_base_error:
	release_mem_region(GPIOC_PHY_ADDR, GPIOC_PHY_ADDR_SIZE);
#if 0	
request_phy_addr_error:
	device_destroy(beep_drv_class, beep_drv_num);
#endif	
device_create_error:
	class_destroy(beep_drv_class);
	
class_create_error:
	cdev_del(&beep_drv_cdev);
	
cdev_add_error:
	unregister_chrdev_region(beep_drv_num, 1);
	printk("<4>" "beep drv init fail\n");
	return ret;
}

static void __exit beep_drv_exit(void)
{
	/*接触动态映射*/
	iounmap(gpio_vir_addr_base);
	
	/*释放物理内存区*/
	release_mem_region(GPIOC_PHY_ADDR, GPIOC_PHY_ADDR_SIZE);
	
	/*销毁device*/
	device_destroy(beep_drv_class, beep_drv_num);
	
	/*销毁class*/
	class_destroy(beep_drv_class);
	
	/*将字符设备从内核删除*/
	cdev_del(&beep_drv_cdev);
	
	/*注销设备号*/
	unregister_chrdev_region(beep_drv_num, 1);
	
	printk("<4>" "beep drv exit\n");
} 

module_init(beep_drv_init);
module_exit(beep_drv_exit);

MODULE_AUTHOR("ayl0521@sina.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("6818 beep derv");
