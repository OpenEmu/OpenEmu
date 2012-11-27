/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <asm/memory.h>

#define MMUHACK_MINOR 225
#define DEVICE_NAME "mmuhack"

#if __GNUC__ == 3
#include <linux/version.h>
static const char __module_kernel_version_gcc3[] __attribute__((__used__)) __attribute__((section(".modinfo"))) =
"kernel_version=" UTS_RELEASE;
#endif

static ssize_t mmuhack_open(struct inode *inode, struct file *filp)
{
    unsigned int *pgtable;
	unsigned int *cpt;
    int i, j;
	int ttb;
	int ret = -EFAULT;

	// get the pointer to the translation table base...
	asm volatile(
		"stmdb sp!, {r0}\n\t"
		"mrc p15, 0, r0, c2, c0, 0\n\t"
		"mov %0, r0\n\t"
		"ldmia sp!, {r0}\n\t": "=r"(ttb)
	);

	pgtable = __va(ttb);

    for (i = 0; i < 4096; i ++) if ( (pgtable[i] & 3) == 1 ) {
		cpt = __va(pgtable[i] & 0xfffffc00);

		for (j = 0; j < 256; j ++) {/*
			if ( (cpt[j] & 0xfe00000f) == 0x02000002 ) {
				// set C and B bits in upper 32MB memory area...
				printk("Set C&B bits %08x\n",cpt[j]);
				cpt[j] |= 0xFFC;
				ret = 0;
			}
					   */
			if (((cpt[j] & 0xff000000) == 0x02000000) && ((cpt[j] & 12)==0) )
			{
				//printk("Set C&B bits %08x\n",cpt[j]);
				cpt[j] |= 0xFFC;
			}
			//if ((a>=0x31 && a<=0x36) && ((cpt[i] & 12)==0))
			if (((cpt[j] & 0xff000000) == 0x03000000) && ((cpt[j] & 12)==0))
			{
				//printk("Set C&B bits %08x\n",cpt[j]);
				//printf("SDL   c and b bits not set, overwriting\n");
				cpt[j] |= 0xFFC;
			}
		}
    }

	// drain the write buffer and flush the tlb caches...
	asm volatile(
		"stmdb sp!, {r0}\n\t"
		"mov    r0, #0\n\t"
		"mcr    15, 0, r0, cr7, cr10, 4\n\t"
		"mcr    15, 0, r0, cr8, cr7, 0\n\t"
		"ldmia sp!, {r0}\n\t"
	);

	if (ret == 0)
		printk("MMU hack applied.\n");

	return 0;
}

static struct file_operations mmuhack_fops = {
    owner:      THIS_MODULE,
    open:       mmuhack_open,
};


static struct miscdevice mmuhack = {
    MMUHACK_MINOR, DEVICE_NAME, &mmuhack_fops
};

static int __init mmuhack_init(void)
{
	misc_register(&mmuhack);
/*
	printk("MMSP2 MMU Hack module.\n");
*/
    return 0;
}

static void __exit mmuhack_exit(void)
{
    misc_deregister(&mmuhack);
/*
	printk(KERN_ALERT "MMU Hack module removed.\n");
*/
}

module_init(mmuhack_init);
module_exit(mmuhack_exit);
