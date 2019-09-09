# include <linux/module.h>
# include <linux/fs.h>
# include <linux/dcache.h>
# include <linux/pagemap.h>
# include <linux/mount.h>
# include <linux/init.h>
# include <linux/namei.h>
//current_fsuid函数：
//current_fsgid函数：
# include <linux/cred.h>
//加入misc机制
# include <linux/kfifo.h>


//每个文件系统需要一个MAGIC number
# define MYFS_MAGIC 0X64668735
# define MYFS "myfs"

static struct vfsmount * myfs_mount;
static int myfs_mount_count;

DEFINE_KFIFO(mydemo_fifo,char,64);

int g_val;


//*****************************************************************************
//									底层创建函数
//*****************************************************************************
static struct inode * myfs_get_inode(struct super_block * sb, int mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if(inode)
	{
		inode -> i_mode = mode;
		//@i_uid：user id
		inode->i_uid  = current_fsuid();
		//@i_gid：group id组标识符
		inode->i_gid  = current_fsgid();
		//@i_size：文件长度
		inode -> i_size = VMACACHE_SIZE;
		//@i_blocks：指定文件按块计算的长度
		inode -> i_blocks = 0;
		//@i_atime：最后访问时间
		//@i_mtime：最后修改时间
		//@i_ctime：最后修改inode时间
		inode -> i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

		switch(mode & S_IFMT)
		{
			default:
				init_special_inode(inode,mode,dev);
				break;
			case S_IFREG:
				printk("creat a file\n");
				break;
			case S_IFDIR:
				printk("creat a content\n");
				//inode_operations
				inode -> i_op = &simple_dir_inode_operations;
				//file_operation	
				inode -> i_fop = &simple_dir_operations;
				//@：文件的链接计数，使用stat命令可以看到Links的值，硬链接数目
				//inode -> i_nlink++;
				inc_nlink(inode);
				break;			
		}
	}
	return inode;
}



//把创建的inode和dentry连接起来
static int myfs_mknod(struct inode * dir, struct dentry * dentry, int mode, dev_t dev)
{
	struct inode * inode;
	int error = -EPERM;

	if(dentry -> d_inode)
		return -EPERM;

	inode = myfs_get_inode(dir->i_sb, mode, dev);
	if(inode)
	{
		d_instantiate(dentry,inode);
		dget(dentry);
		error = 0;

	}
	return error;
}

//************************************************************************
//							创建目录，文件
//************************************************************************

static int myfs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
{
	int res;

	res = myfs_mknod(dir, dentry, mode|S_IFDIR, 0);
	if(!res)
	{
		inc_nlink(dir);

	}
	return res;
}

static int myfs_creat(struct inode * dir, struct dentry * dentry, int mode)
{
	return myfs_mknod(dir, dentry, mode|S_IFREG, 0);
}


//************************************************************************
//							　　　注册信息
//************************************************************************

/*

*/
static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	//这个结构体如下：
	//struct tree_descr { const char *name; const struct file_operations *ops; int mode; };
	static struct tree_descr debug_files[] = {{""}};

	return simple_fill_super(sb,MYFS_MAGIC,debug_files);
}




/*
这个函数是按照内核代码中的样子改的，是struct dentry *类型，这里是一个封装，这里可以返回好几种函数：
　－　mount_single
　－　mount_bdev　
　－　mount_nodev
*/

static struct dentry *myfs_get_sb(struct file_system_type *fs_type, int flags,
		       const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, myfs_fill_super);
}



/*********************************************************************
								文件操作部分
*********************************************************************/
//对应于打开aufs文件的方法
static int myfs_file_open(struct inode *inode, struct file *file)
{
	printk("已打开文件");

	return 0;
}
//对应于读取的aufs文件的读取方法
static ssize_t myfs_file_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int actual_readed;
	int ret;

	ret = kfifo_to_user(&mydemo_fifo,buf, count, &actual_readed);
	if(ret)
		return -EIO;

	printk("%s,actual_readed=%d,pos=%lld\n",__func__,actual_readed,*ppos);

	return actual_readed;
}
//对应于写入的aufs文件的写入方法
static ssize_t myfs_file_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{	
	unsigned int actual_write;
	int ret;

	ret = kfifo_from_user(&mydemo_fifo, buf, count, &actual_write);
	if(ret)
		return -EIO;

	printk("%s: actual_write=%d,ppos=%lld\n",__func__,actual_write,*ppos);

	return actual_write;

}




static struct file_system_type my_fs_type = {
	.owner 		= THIS_MODULE,
	.name 		= MYFS,
	.mount 		= myfs_get_sb,
	.kill_sb 	= kill_litter_super 
};


static struct file_operations myfs_file_operations = {
    .open = myfs_file_open,
    .read = myfs_file_read,
    .write = myfs_file_write,
};



//*****************************************************************************
//					
//*****************************************************************************


static int myfs_creat_by_name(const char * name, mode_t mode,
				struct dentry * parent, struct dentry ** dentry)
{
	int error = 0;

	if(!parent)
	{
		if(myfs_mount && myfs_mount -> mnt_sb)
		{
			parent = myfs_mount->mnt_sb->s_root;
		}
	}

	if(!parent)
	{
		printk("can't find a parent");
		return -EFAULT;
	}

	*dentry = NULL;

	inode_lock(d_inode(parent));
	*dentry = lookup_one_len(name,parent,strlen(name));
	if(!IS_ERR(*dentry))
	{
		if((mode & S_IFMT) == S_IFDIR)
		{
			error = myfs_mkdir(parent->d_inode, *dentry, mode);
		}
		else
		{
			error = myfs_creat(parent->d_inode, *dentry, mode);
		}
	}
	//error是０才对
	if (IS_ERR(*dentry)) {
		error = PTR_ERR(*dentry);
	}
	inode_unlock(d_inode(parent));

	return error;
}


struct dentry * myfs_creat_file(const char * name, mode_t mode,
				struct dentry * parent, void * data,
				struct file_operations * fops)
{
	struct dentry * dentry = NULL;
	int error;

	printk("myfs:creating file '%s'\n",name);

	error = myfs_creat_by_name(name, mode, parent, &dentry);

	if(error)
	{
		dentry = NULL;
		goto exit;
	}

	if(dentry->d_inode)
	{
		if(data)
			dentry->d_inode->i_private = data;
		if(fops)
			dentry->d_inode->i_fop = fops;
	}

exit:
	return dentry;
}

struct dentry * myfs_creat_dir(const char * name, struct dentry * parent)
{
	//使用man creat查找
	//@S_IFREG：表示一个目录
	//@S_IRWXU：user (file owner) has read,  write,  and  execute　permission
	//@S_IRUGO：用户读｜用户组读｜其他读
	return myfs_creat_file(name, S_IFDIR|S_IRWXU|S_IRUGO, parent, NULL, NULL);
}



//*************************************************************************
//								模块注册退出
//*************************************************************************	

static int __init myfs_init(void)
{
	int retval;
	struct dentry * pslot;

	//将文件系统登录到系统中去
	retval = register_filesystem(&my_fs_type);

	if(!retval)
	{
		//创建super_block根dentry的inode
		myfs_mount = kern_mount(&my_fs_type);
		//如果装载错误就卸载文件系统
		if(IS_ERR(myfs_mount))
		{
			printk("--ERROR:aufs could not mount!--\n");
			unregister_filesystem(&my_fs_type);
			return retval;
		}
	}

	pslot = myfs_creat_dir("First", NULL);
	//@S_IFREG：表示一个文件
	//@S_IRUGO：用户读｜用户组读｜其他读
	myfs_creat_file("one", S_IFREG|S_IRUGO|S_IWUSR, pslot, NULL, &myfs_file_operations);
	myfs_creat_file("two", S_IFREG|S_IRUGO|S_IWUSR, pslot, NULL, &myfs_file_operations);

	pslot = myfs_creat_dir("Second", NULL);
	myfs_creat_file("one", S_IFREG|S_IRUGO|S_IWUSR, pslot, NULL, &myfs_file_operations);
	myfs_creat_file("two", S_IFREG|S_IRUGO|S_IWUSR, pslot, NULL, &myfs_file_operations);

	return retval;
}

static void __exit myfs_exit(void)
{
	//退出函数中卸载super_block根dentry的inode
	simple_release_fs(&myfs_mount,&myfs_mount_count);
	//卸载文件系统
	unregister_filesystem(&my_fs_type);
	//aufs_mount = NULL;
}

module_init(myfs_init);
module_exit(myfs_exit);
MODULE_LICENSE("GPL");