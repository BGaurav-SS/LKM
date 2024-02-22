#include <linux/init.h>             //Macro for using functions like __init() and __exit()
#include <linux/module.h>           //Core header for loading LKMs into the kernel
#include <linux/kernel.h>           //Contains types, macros, functions for the kernel

MODULE_LICENSE("GPL");              //The license type -- this affects runtime behavior
MODULE_AUTHOR("Gaurav Bhattarai");  //Information about the author
MODULE_DESCRIPTION("Sample LKM.");  //Description of the module
MODULE_VERSION("One and Only");     //Version info

static char *name = "World" ;                            //Declared as static because the variables in kernel modules should not be made global as long as possible.
module_param(name, charp, S_IRUGO);                      //Name of variable and its type. You should always try to give the variables an initial default value.
MODULE_PARM_DESC(name, "Input from the command line.");  //Describes the parameter. It is used specifically for documentation purpose.



/*
    This is the initialization function. The __init macro declares the function as removable from memory once it returns. 
*/
static int __init hello_init(void){
    printk (KERN_INFO "Printed from __init function. Hello %s.\n", name);
    return 0;
}


static void __exit hello_exit(void){
    printk (KERN_INFO "Have a great day %s. Bye bye.\n", name);
}

module_init(hello_init);
module_exit(hello_exit);