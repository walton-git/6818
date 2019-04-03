#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/ioctl.h"

int main(int argc, char **argv)
{
	int fd = 0;
	int val = 0;
	
	fd = open("/dev/humidity_drv", O_RDWR);
	if(fd < 0)
	{
		perror("open");
	}
	
	while(1)
	{
		read(fd, &val, sizeof(val));
		printf("distance = %d\n", val);	
	}

	close(fd);
	
	return 0;
}
