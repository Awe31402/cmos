/*
 * This module is used to show the content of cmos (RTC, Real-Time Clock)
 * to proc-file /proc/cmos.
 * The purpose is to practice the ioport skill in driver programming.
 * You may "cat /proc/ioport" to checkout that 0x70 is ioaddress of RTC.
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>

char modname[] = "cmos";
unsigned char cmos[10] = {};
char *day[] = {"", "Mon", "Tue", "Wed", "THU", "FRI", "SAT", "SUN"};
char *month[] = {"", "JAN", "FEB", "MAR", "APR", "MAY",
		 "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
unsigned int rtc_base = 0x70;
int my_get_info(struct seq_file *buf, void *v)
{
	int i;
	int month_index;

	for (i = 0; i < 10; i++) {
		outb(i, rtc_base);
		wmb(); /* write memory barrier */
		cmos[i] = inb(rtc_base + 1);
		rmb(); /* memory barrier */
	}

	/*for(i = 0; i < 10; i++) {
		seq_printf(buf, (i == 0)? "%02x" : "-%02x", cmos[i]);
	}
	seq_printf(buf, "\n");*/

	seq_printf(buf, "20%02x ", cmos[9]);
	month_index = ((cmos[8] & 0xf0) >> 4) * 10 + cmos[8] & 0x0f;
	seq_printf(buf, "%s ", month[month_index]);
	seq_printf(buf, "%02x ", cmos[7]);
	seq_printf(buf, "%s ", day[cmos[6]]);
	seq_printf(buf, "%02x : ", cmos[4]); /* hour */
	seq_printf(buf, "%02x : ", cmos[2]); /* minute */
	seq_printf(buf, "%02x\n", cmos[0]);  /* second */
	return 0;
}

static int my_get_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, my_get_info, NULL);
}

static const struct file_operations my_get_info_fops = {
	.open = my_get_info_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init my_init(void)
{
	pr_debug("Intalling module: %s\n", modname);
	proc_create(modname, 0, NULL, &my_get_info_fops);
	return 0;
}

static void __exit my_exit(void)
{
	remove_proc_entry(modname, NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("Dual BSD/GPL");
