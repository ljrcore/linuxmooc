//-------------------------------------------------------------------
//	dram.c
//
//	This module implements a Linux character-mode device-driver
//	for the processor's installed physical memory.  It utilizes
//	the kernel's 'kmap()' function, as a uniform way to provide  
//	access to all the memory-zones (including the "high memory"
//	on systems with more than 896MB of installed physical ram). 
//	The access here is 'read-only' because we deem it too risky 
//	to the stable functioning of our system to allow every user
//	the unrestricted ability to arbitrarily modify memory-areas
//	which might contain some "critical" kernel data-structures.
//	We implement an 'llseek()' method so that users can readily 
//	find out how much physical processor-memory is installed. 
//
//	NOTE: Developed and tested with Linux kernel version 2.6.10
//
//	programmer: ALLAN CRUSE
//	written on: 30 JAN 2005
//	revised on: 28 JAN 2008 -- for Linux kernel version 2.6.22.5
//	revised on: 06 FEB 2008 -- for machines having 4GB of memory
//-------------------------------------------------------------------

#include <linux/module.h>	// for module_init() 
#include <linux/highmem.h>	// for kmap(), kunmap()
#include <linux/uaccess.h>	// for copy_to_user() 

char modname[] = "dram";	// for displaying driver's name
int my_major = 85;		// note static major assignment 
unsigned long dram_size;		// total bytes of system memory

loff_t my_llseek( struct file *file, loff_t offset, int whence );
ssize_t my_read( struct file *file, char *buf, size_t count, loff_t *pos );

struct file_operations 
my_fops =	{
		owner:		THIS_MODULE,
		llseek:		my_llseek,
		read:		my_read,
		};

static int __init dram_init( void )
{
	printk( "<1>\nInstalling \'%s\' module ", modname );
	printk( "(major=%d)\n", my_major );
   
	dram_size = 0x25f5ffff8;
	printk( "<1>  ramtop=%08lX (%lu MB)\n", dram_size, dram_size >> 20 );
	return 	register_chrdev( my_major, modname, &my_fops );
}

static void __exit dram_exit( void )
{
	unregister_chrdev( my_major, modname );
	printk( "<1>Removing \'%s\' module\n", modname );
}

ssize_t my_read( struct file *file, char *buf, size_t count, loff_t *pos )
{
	struct page	*pp;
	void		*from;
	int		page_number, page_indent, more;
	
	// we cannot read beyond the end-of-file
	if ( *pos >= dram_size ) return 0;

	// determine which physical page to temporarily map
	// and how far into that page to begin reading from 
	page_number = *pos / PAGE_SIZE;
	page_indent = *pos % PAGE_SIZE;
	
	// map the designated physical page into kernel space
	/*If kerel vesion is 2.6.32 or later, please use pfn_to_page() to get page, and include
	    asm-generic/memory_model.h*/

       pp = pfn_to_page( page_number);
	
	from = kmap( pp ) + page_indent;
	
	// cannot reliably read beyond the end of this mapped page
	if ( page_indent + count > PAGE_SIZE ) count = PAGE_SIZE - page_indent;

	// now transfer count bytes from mapped page to user-supplied buffer 	
	more = copy_to_user( buf, from, count );
	
	// ok now to discard the temporary page mapping
	kunmap( pp );
	
	// an error occurred if less than count bytes got copied
	if ( more ) return -EFAULT;
	
	// otherwise advance file-pointer and report number of bytes read
	*pos += count;
	return	count;
}

loff_t my_llseek( struct file *file, loff_t offset, int whence )
{
	loff_t	newpos = -1;

	switch( whence )
		{
		case 0: newpos = offset; break;			// SEEK_SET
		case 1: newpos = file->f_pos + offset; break; 	// SEEK_CUR
		case 2: newpos = dram_size + offset; break; 	// SEEK_END
		}

	if (( newpos < 0 )||( newpos > dram_size )) return -EINVAL;
	file->f_pos = newpos;
	return	newpos;
}

MODULE_LICENSE("GPL");
module_init( dram_init );
module_exit( dram_exit );

