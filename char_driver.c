/**
 * char_driver.c -- This is a sample program for character device driver program
 *
 * Written on Wednesday, 13 May 2020.
 */

/*Kernel does not have access to standard c libraries*/
#include <linux/init.h>
#include <linux/module.h>

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


/* Loadable modules: Modules that are dynamically loaded adfter the machine is up
 * Built-in-Drivers: Modules are built into linux kernel. These modules loads while kernel is loading.
 * Specifying __init to the function will let the kernel know that the function is used only
 * for initialization. Hence once the module is loaded kernel remove the space used for init function.
 * This is applicable only for built-in-drivers. For loadable modules this will get ignored.
*/
static int __init char_module_init(void)
{
    while(hello_print) {
	printk(KERN_ALERT "Hello World!!!\n");
	hello_print--;
    }
    
    return 0;
}

static void __exit char_module_exit(void)
{
    printk(KERN_ALERT"BYE!!! BYE!!!\n");
}
module_init(char_module_init);
module_exit(char_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("JANARDHAN");
MODULE_DESCRIPTION("This is a sample characer device driver program");
