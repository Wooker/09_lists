#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

// Define the kobject
static struct kobject *my_kobject;

// Custom structure to hold module information
struct module_info {
    struct list_head list;
    char name[MODULE_NAME_LEN];
};

// Linked list head for modules
static LIST_HEAD(module_list);

// Comparison function for sorting module names
static int compare_modules(void *priv, const struct list_head *a, const struct list_head *b) {
    struct module_info *mod_a = list_entry(a, struct module_info, list);
    struct module_info *mod_b = list_entry(b, struct module_info, list);
    
    return strcasecmp(mod_a->name, mod_b->name);
}



// Sysfs file show function
static ssize_t my_sys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    struct module_info *info;
    int count = 0;
    
    // Sort the module names in alphabetical order
    list_sort(NULL, &module_list, compare_modules);
    
    // Output the module names
    list_for_each_entry(info, &module_list, list) {
        count += snprintf(buf + count, PAGE_SIZE - count, "%s\n", info->name);
    }
    
    return count;
}

// Sysfs file attribute structure
static struct kobj_attribute my_kobject_attr = {
    .attr = {
        .name = "my_sys",
        .mode = S_IRUGO,
    },
    .show = my_sys_show,
};


// Function to initialize the module
static int __init my_module_init(void) {
    struct module *mod;
    struct module_info *info;
    
    // Create the kobject
    my_kobject = kobject_create_and_add("my_kobject", kernel_kobj);
    if (!my_kobject)
        return -ENOMEM;
        
    // Traverse the module list and store names in module_info
    list_for_each_entry(mod, THIS_MODULE->list.prev, list) {
        info = kmalloc(sizeof(struct module_info), GFP_KERNEL);
        if (!info)
            return -ENOMEM;
        strncpy(info->name, mod->name, MODULE_NAME_LEN);
        list_add_tail(&info->list, &module_list);
    }
    
    // Create the sysfs file
    if (sysfs_create_file(my_kobject, &my_kobject_attr.attr)) {
        kobject_put(my_kobject);
        return -ENOMEM;
    }
    
    return 0;
}

// Function to cleanup the module
static void __exit my_module_exit(void) {
    struct module_info *info, *tmp;
    
    // Remove the sysfs file
    sysfs_remove_file(my_kobject, &my_kobject_attr.attr);
    
    // Free the module_info structures
    list_for_each_entry_safe(info, tmp, &module_list, list) {
        list_del(&info->list);
        kfree(info);
    }
    
    // Remove the kobject
    kobject_put(my_kobject);
}

// Register the module initialization and cleanup functions
module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zakhar Semenov");
MODULE_DESCRIPTION("Solution");