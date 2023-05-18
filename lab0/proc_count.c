#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>

static struct proc_dir_entry *entry;

static int show(struct seq_file *m, void *v) {
	int num_of_process = 0;
	struct task_struct *t;
	for_each_process(t) {
		num_of_process++;
	}
	seq_printf(m,"%d\n", num_of_process);
	return 0;
}


static int __init proc_count_init(void)
{
	entry = proc_create_single("count", 0644, NULL, show);
	pr_info("proc_count: init\n");
	return 0;
}

static void __exit proc_count_exit(void)
{
	proc_remove(entry);
	pr_info("proc_count: exit\n");
}

module_init(proc_count_init);
module_exit(proc_count_exit);

MODULE_AUTHOR("Claire Brenton");
MODULE_DESCRIPTION("This kernel module creates a virtual /proc/count file that shows the current number of running processes when read or opened.");
MODULE_LICENSE("GPL");
