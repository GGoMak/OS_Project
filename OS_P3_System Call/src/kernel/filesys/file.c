#include <filesys/inode.h>
#include <proc/proc.h>
#include <device/console.h>
#include <mem/palloc.h>

int file_open(struct inode *inode, int flags, int mode)
{
	int fd;
	struct ssufile **file_cursor = cur_process->file;

	for(fd = 0; fd < NR_FILEDES; fd++)
	{
		if(file_cursor[fd] == NULL)
		{
			if( (file_cursor[fd] = (struct ssufile *)palloc_get_page()) == NULL)
				return -1;
			break;
		}	
	}
	
	inode->sn_refcount++;

	file_cursor[fd]->inode = inode;
	file_cursor[fd]->pos = 0;

	if(flags & O_APPEND){
		file_cursor[fd]->pos = file_cursor[fd]->inode->sn_size; 
	}
	else if(flags & O_TRUNC){
		int size = file_cursor[fd]->inode->sn_size;
		
		for(int i = 0; i < size; i++){
			file_write(file_cursor[fd]->inode, size, '\0', 1);
		}

		file_cursor[fd]->inode->sn_size = 0;
	}

	file_cursor[fd]->flags = flags;
	file_cursor[fd]->unused = 0;

	return fd;
}

int file_write(struct inode *inode, size_t offset, void *buf, size_t len)
{
	return inode_write(inode, offset, buf, len);
}

int file_read(struct inode *inode, size_t offset, void *buf, size_t len)
{
	return inode_read(inode, offset, buf, len);
}
