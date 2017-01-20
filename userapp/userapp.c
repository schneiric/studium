#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/exampledriver"

//~ write(int ,char *,int);
//~ read(int ,char *,int );
//~ close(int );

int main() {
	int i,fd;	//i=numbers of bytes writen, filedescription
	char ch, write_buf[100], read_buf[100]; //choise for reading or writing
	 
	fd = open(DEVICE, O_RDWR);	//open for reading and writing
	 
	if(fd == -1){
		printf("file %s either does not exist or has been locked by another process\n",DEVICE);
		exit(-1);
	} 
	printf ("r = read from device\nw = write to device\nenter command: ");
	scanf("%c", &ch);
	
	switch (ch) {
				case 'w':
					printf ("enter data: ");
						scanf (" %[^\n]", write_buf);
						write(fd, write_buf, sizeof(write_buf));
						break;
				case 'r':
						read(fd, read_buf, sizeof(read_buf));
						printf ("device: %s\n", read_buf);
						break;
				default:
						printf("command not recognized\n");
						break;
	}
	close(fd);
	 
	return 0;
}
