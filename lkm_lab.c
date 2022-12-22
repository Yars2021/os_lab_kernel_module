#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>

#define REG32(_name, offset)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yars");
MODULE_DESCRIPTION("OS Lab kernel module.");

static struct timer_list lkm_timer;
static struct dentry *mod_dir = 0;
static u32 arg = 0;
static u32 op_error = 0;
static bool processed = 0;

static struct debugfs_regset32 regset;
static void __iomem *addr;
static u32 lkm_task_pid = 0;
static u32 lkm_task_priority = 0;
static u32 lkm_task_state = 0;

static const struct debugfs_reg32 x86_debugfs_reg32[] = {
        {.name = "EAX", .offset = 0x00},
        {.name = "EBX", .offset = 0x04},
        {.name = "ECX", .offset = 0x08},
        {.name = "EDX", .offset = 0x0C}
};

static void monitor_regs(void) {
    register int eax asm("eax");
    register int ebx asm("ebx");
    register int ecx asm("ecx");
    register int edx asm("edx");

    addr = kmalloc(16, GFP_ATOMIC);
    regset.nregs = 4;
    regset.regs = x86_debugfs_reg32;
    regset.base = addr;

    *((int*) addr) = eax;
    *((int*)(addr + 0x4)) = ebx;
    *((int*)(addr + 0x8)) = ecx;
    *((int*)(addr + 0xC)) = edx;
}

static int monitor_task_struct(pid_t pid) {
	struct task_struct *task_struct;

	if (pid == 0) return -EINVAL;

	task_struct = pid_task(find_vpid(pid), PIDTYPE_PID);

	if (!mod_dir) {
		pr_info("lkm_lab: the directory in /sys/kernel/debug was not created\n");
		return -ENOENT;
	}

	if (IS_ERR(task_struct)) return -ESRCH;

	lkm_task_pid = (u32) task_struct->pid;
	lkm_task_priority = (u32) task_struct->prio;
	lkm_task_state = (u32) task_struct->__state;

	return 0;
}

static void process_args(struct timer_list *timer_list) {
	if (arg != 0) {
        processed = 0;
		monitor_regs();
		op_error = monitor_task_struct((pid_t) arg);
        processed = 1;
        arg = 0;
	}

	lkm_timer.expires = jiffies + (1 * HZ);
	add_timer(&lkm_timer);
}

static int __init lkm_module_init(void) {
	mod_dir = debugfs_create_dir("lkm_lab", NULL);
	if (!mod_dir) {
		pr_info("lkm_lab: unable to create /sys/kernel/debug/lkm_lab\n");
		return -EPERM;
	}

	debugfs_create_u32("arg", 0666, mod_dir, &arg);
	debugfs_create_u32("op_error", 0666, mod_dir, &op_error);
	debugfs_create_bool("processed", 0666, mod_dir, &processed);

	debugfs_create_regset32("regset", 0666, mod_dir, &regset);

	debugfs_create_u32("ts_pid_node", 0666, mod_dir, &lkm_task_pid);
	debugfs_create_u32("ts_prio_node", 0666, mod_dir, &lkm_task_priority);
	debugfs_create_u32("ts_state_node", 0666, mod_dir, &lkm_task_state);

	timer_setup(&lkm_timer, process_args, 0);
	lkm_timer.expires = jiffies + (1 * HZ);
	add_timer(&lkm_timer);

	pr_info("lkm_lab: module loaded\n");

	return 0;
}

static void __exit lkm_module_exit(void) {
    kfree(addr);
	if (mod_dir) debugfs_remove_recursive(mod_dir);
	del_timer_sync(&lkm_timer);
	pr_info("lkm_lab: module unloaded\n");
}

module_init(lkm_module_init);
module_exit(lkm_module_exit);
