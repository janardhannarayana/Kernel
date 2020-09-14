/*
 * char_driver.c -- This is a sample program for character device driver program
 *
 * Written on Wednesday, 13 May 2020.
 */

/*Kernel does not have access to standard c libraries*/
#include "char_driver.h"

/* module_param(variable, type, perm)
 * module_param is used for accepting command line arguments in kernel modules
 * module_param does not declare the variables. So decalred variables along with type
 * should be passed. module_param variables are created under sysfs. "perm" is the
 * permission for sysfs.
 * Ex:insmod char_driver.ko "hello_print=4"
 * charp is the type for char to pointer
 */

static int num_dev = 1;
module_param(num_dev, int, S_IRUGO);
MODULE_PARM_DESC(num_dev, "This parameter specifies the number of device nodes this driver support");

struct mydevice *chardevices = NULL;
dev_t dev;

#if (USE_PROC == 1)
struct proc_dir_entry *parent = NULL;
struct proc_dir_entry *proc_entry = NULL;
static int proc_ite = 5000;
/*
 * create_proc_read_entry is deprecated. So we are not going use that API for
 * proc creation.
 * proc_create is the new api for creating the proc entry.
 */

int myproc_open(struct inode *dev_inode, struct file *dev_file)
{
    printk(KERN_ALERT "Proc open function\n");
    return 0;
}

ssize_t myproc_read(struct file *fp, char __user *userbuff, size_t size, loff_t *offset)
{
    int i = 0;
    printk(KERN_ALERT "PROC READ FILE\n");

    for (i = 0; i < proc_ite; i++)
	printk(KERN_ALERT "i = %d\n", i);
    
    /* Returns the number of bytes we filled */
    return 0;
}

ssize_t myproc_write(struct file *fp, const char __user *userbuff, size_t size, loff_t *offset)
{
    char proc_buff[size+1];

    printk(KERN_ALERT "PROC WRITE FILE\n");
    
    memset(proc_buff, 0, size+1);

    copy_from_user(proc_buff, userbuff, size);

    proc_buff[size] = '\0';

    kstrtoint(proc_buff, 0, &proc_ite);
    /*User write function writes repeatedly until it completes. So return the size requested. this is only for testing*/
    return size;
}

#endif

int dev_open(struct inode *dev_inode, struct file *dev_file)
{
    printk(KERN_ALERT "Open function\n");
    /*
     * Open function alone contains inode structure.
     * inode structure contains the corresponding cdev structure.
     * using container_of macro we can find the the struct mydevice base address
     * by passing cdev address as a parameter.
     * This will be saved in file structure and further used in other functions
     * So that multiple device nodes can access their own data structure
     */
    dev_file->private_data = container_of(dev_inode->i_cdev, struct mydevice, dev_cdev);
    printk("private data = %p\n", dev_file->private_data);
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
    struct mydevice *device = fp->private_data;
    printk(KERN_ALERT "Read function\n");
    copy_to_user(userbuff, device->data, size);
    return 0;
}

ssize_t dev_write(struct file *fp, const char __user *userbuff, size_t size, loff_t *offset)
{
    struct mydevice *device = fp->private_data;
    printk(KERN_ALERT "Write function\n");
    copy_from_user(device->data, userbuff, size);
    printk(KERN_ALERT "Data from user = %s", chardevices->data);
    return 0;
}


/* Loadable modules: Modules that are dynamically loaded after the machine is up
 * Built-in-Drivers: Modules are built into linux kernel. These modules loads while kernel is loading.
 * Specifying __init to the function will let the kernel know that the function is used only
 * for initialization. Hence once the module is loaded kernel remove the space used for init function.
 * This is applicable only for built-in-drivers. For loadable modules this will get ignored.
 */
static int __init char_module_init(void)
{
    int i = 0;

    printk(KERN_ALERT "Sample charachter driver initialization\n");

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

    if (alloc_chrdev_region(&dev, 0, num_dev, "CHAR DEVICE") != 0) {
	printk(KERN_ALERT  "Char device registration failed\n");
	return -1;
    } else {
	printk(KERN_ALERT "Device major number %d minor number %d\n", MAJOR(dev), MINOR(dev));
    }

    chardevices = kmalloc(sizeof(struct mydevice) * num_dev, GFP_KERNEL);

    /* Based on number device nodes creating, that many cdev should be added with different
     * minor numbers. Device nodes which creates using this particular major and minor number
     * will have the different cdev structures allocated
     */
    for(i = 0; i < num_dev; i++) {
	int mydev = MKDEV(MAJOR(dev), i);
	(chardevices + i)->data = kmalloc(128, GFP_KERNEL);
	memcpy((chardevices + i)->data, "Default string\n", 15);
	(chardevices + i)->data[15] = '\0';
	cdev_init(&((chardevices + i)->dev_cdev), &dev_fops);
	cdev_add(&((chardevices + i)->dev_cdev), mydev, 1);
    }

#if (USE_PROC == 1)
    parent = proc_mkdir("char_device", NULL);

    proc_entry = proc_create("dev_proc", 0666, parent, &proc_fops);
    
#endif

    return 0;
}

static void __exit char_module_exit(void)
{
    int i = 0;

#if (USE_PROC == 1)
    proc_remove(proc_entry);
    proc_remove(parent);
#endif
    for(i = 0; i < num_dev; i++) {
	cdev_del(&((chardevices + i)->dev_cdev));
	kfree((chardevices + i)->data);
	kfree(chardevices + i);
    }
    unregister_chrdev_region(dev, num_dev);

    printk(KERN_ALERT"BYE!!! BYE!!!\n");
}
module_init(char_module_init);
module_exit(char_module_exit);

/* Without license when we load the driver in kernel, kernel shows a tainted kernel message */
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("JANARDHAN");
MODULE_DESCRIPTION("This is a sample characer device driver program");
