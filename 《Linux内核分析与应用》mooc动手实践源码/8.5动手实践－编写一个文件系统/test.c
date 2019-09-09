# include <stdio.h>
# include <fcntl.h>
# include <unistd.h>
# include <malloc.h>
# include <string.h>

# define FILE_NAME "/myfs/First/one"

int main() 
{
	char buffer[64];
	int fd;
	int ret;
	size_t len;

	char message[] = "I am myfs";
	char *read_buffer;

	len = sizeof(message);

	fd = open(FILE_NAME,O_RDWR);
	if(fd<0) 
	{
		printf("wrong\n");
		return -1;
	}

	//向设备写数据
	ret = write(fd,message,len);
	if(ret != len)
	{
		printf("wrongd\n");
		return -1;
	}

	read_buffer = malloc(2*len);
	memset(read_buffer,0,2*len);
	//关闭设备

	ret = read(fd,read_buffer,2*len);
	printf("read %d bytes\n",ret);
	printf("read buffer=%s\n",read_buffer);

	close(fd);

	return 0;
}