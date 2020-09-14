#ifndef KERNEL_HEADER
#define KERNEL_HEADER

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#define USE_PROC 1

struct mydevice {
    struct cdev dev_cdev;
    char *data;
};

int dev_open(struct inode *dev_inode, struct file *dev_file);
ssize_t dev_read(struct file *fp, char *userbuff, size_t size, loff_t *offset);
ssize_t dev_write(struct file *fp, const char *userbuff, size_t size, loff_t *offset);


struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open  = dev_open,
    .read  = dev_read,
    .write = dev_write,
};

#if (USE_PROC == 1)

int myproc_open(struct inode *dev_inode, struct file *dev_file);
ssize_t myproc_read(struct file *fp, char __user *userbuff, size_t size, loff_t *offset);
ssize_t myproc_write(struct file *fp, const char __user *userbuff, size_t size, loff_t *offset);

/* Use the following structure for kernel 5.0 version onwards.
 * Kernel changed proc fops from standard file_operatioins to struct proc_ops
 */
#ifdef KERNEL_5
struct proc_ops proc_fops = {
    .proc_open  = myproc_open,
    .proc_write = myproc_write,
    .proc_read  = myproc_read,
};
#else
struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .write = myproc_write,
    .open  = myproc_open,
    .read  = myproc_read,
};
#endif

#endif

#endif
