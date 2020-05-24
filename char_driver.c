/*
 * char_driver.c -- This is a sample program for character device driver program
 *
 * Written on Wednesday, 13 May 2020.
 */

/*Kernel does not have access to standard c libraries*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/* module_param(variable, type, perm)
 * module_param is used for accepting command line arguments in kernel modules
 * module_param does not declare the variables. So decalred variables along with type
 * should be passed. module_param variables are created under sysfs. "perm" is the
 * permission for sysfs.
 * Ex:insmod char_driver.ko "hello_print=4"
 * charp is the type for char to pointer
*/

static int hello_print = 1;
module_param(hello_print, int, S_IRUGO);
MODULE_PARM_DESC(hello_print, "This parameter prints the hello world");

struct mydevice {
    struct cdev dev_cdev;
    char *data;
};

struct mydevice *chardevices;
dev_t dev;

int dev_open(struct inode *dev_inode, struct file *dev_file);
ssize_t dev_read(struct file *fp, char *userbuff, size_t size, loff_t *offset);
ssize_t dev_write(struct file *fp, const char *userbuff, size_t size, loff_t *offset);

struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open  = dev_open,
    .read  = dev_read,
    .write = dev_write,
};

int dev_open(struct inode *dev_inode, struct file *dev_file)
{
    printk(KERN_ALERT "Open function\n");
    return 0;
}

/* Specifying variable as __user indicated the kernel that the data is in user space
*  This can avoid serious mistakes by some softwares like coverity.
*  Read reads the kernel data and copy into the user buffer.
*  Write writes the user buffer data into kernel
*  Hence const used in write call.
*/

ssize_t dev_read(struct file *fp, char __user *userbuff, size_t size, loff_t *offset)
{
    printk(KERN_ALERT "Read function\n");
    copy_to_user(userbuff, chardevices->data, size);
    return 0;
}

ssize_t dev_write(struct file *fp, const char __user *userbuff, size_t size, loff_t *offset)
{
    printk(KERN_ALERT "Write function\n");
    copy_from_user(chardevices->data, userbuff, size);
    printk(KERN_ALERT "Data from user = %s", chardevices->data);
    return 0;
}


/* Loadable modules: Modules that are dynamically loaded adfter the machine is up
 * Built-in-Drivers: Modules are built into linux kernel. These modules loads while kernel is loading.
 * Specifying __init to the function will let the kernel know that the function is used only
 * for initialization. Hence once the module is loaded kernel remove the space used for init function.
 * This is applicable only for built-in-drivers. For loadable modules this will get ignored.
*/
static int __init char_module_init(void)
{
//    dev_t dev;
    while(hello_print) {
	printk(KERN_ALERT "Hello World!!!\n");
	hello_print--;
    }

    /* Major number and minor number are used to identify the device driver and the device
    * associated to that driver.
    * We can create multiple device with same major number and minor number. It is user responsibility
    * to take care of that.
    * alloc_chrdev_region just registers the driver with the kernel, but it wont allocate the cdev structure.
    * Due to this you can still see the device in /proc/devices and create a device too.
    * A driver needs to register with cdev structure to access all the operations.
    * second argument in alloc_chrdev_region denotes the base minor number.
    * third argument in alloc_chrdev_region denotes the number of devices you can create.
    * You can still create the device nodes with less that base number or more than the number of
    * devices you specified.
    * Ex: (alloc_chrdev_region(&dev, 2, 6, "CHAR DEVICE")
    * This can create a total of 6 devices with device number starting from 2
    * However user can still create 7 devices or device minor number starting with 0. But the kernel
    * does not operate function operations like open, User always get -1 as FD
    */
    
    if (alloc_chrdev_region(&dev, 0, 1, "CHAR DEVICE") != 0) {
	printk(KERN_ALERT  "Char device registration failed\n");
	return -1;
    } else {
	printk(KERN_ALERT "Device major number %d minor number %d\n", MAJOR(dev), MINOR(dev));
    }

    chardevices = kmalloc(sizeof(struct mydevice), GFP_KERNEL);
    chardevices->data = kmalloc(128, GFP_KERNEL);
    cdev_init(&(chardevices->dev_cdev), &dev_fops);

    if (cdev_add(&(chardevices->dev_cdev), dev, 1) != 0 ) {
	printk(KERN_ALERT "Fail to add the character device\n");
	kfree(chardevices->data);
	kfree(chardevices);
	unregister_chrdev_region(0, 1);
	return -1;
    }
    
    return 0;
}

static void __exit char_module_exit(void)
{
    cdev_del(&(chardevices->dev_cdev));
    kfree(chardevices->data);
    kfree(chardevices);
    unregister_chrdev_region(dev, 1);
    
    printk(KERN_ALERT"BYE!!! BYE!!!\n");
}
module_init(char_module_init);
module_exit(char_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("JANARDHAN");
MODULE_DESCRIPTION("This is a sample characer device driver program");
