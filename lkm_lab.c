#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yars");
MODULE_DESCRIPTION("OS Lab kernel module.");
MODULE_VERSION("0.1");

static struct task_struct *task_struct = 0;
static struct dentry *mod_dir = 0;

static int __init lkm_module_init(void) {
	task_struct = current;

	mod_dir = debugfs_create_dir("lkm_lab", 0);
	if (!mod_dir) {
		printk(KERN_ALERT "lkm_lab: failed to create /sys/kernel/debug/lkm_lab\n");
		return -EPERM;
	}

	debugfs_create_u32("pid_node", 0666, mod_dir, (u32*) &task_struct->pid);
	return 0;
}

static void __exit lkm_module_exit(void) {
	if (mod_dir) debugfs_remove_recursive(mod_dir);
	printk(KERN_INFO "lkm_module exited\n");
}

module_init(lkm_module_init);
module_exit(lkm_module_exit);
