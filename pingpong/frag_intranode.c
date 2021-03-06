#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <arpa/inet.h>
#include "utils.h"
#define FLAG_STREAM 0x0
#define FLAG_DGRAM 0x1
#define FLAG_SERVER 0x2
#define FLAG_CLIENT 0x0
#define FLAG_UNIX 0x0
#define FLAG_V4V 0x4
#define SERVER_PORT 1500
#define CLIENT_PORT 1600
#define AF_V4V 12345
#define ALIGN 4096
void print(unsigned char* buf, uint32_t size);
int validate_data(unsigned char* reader, unsigned char* writer, uint32_t size);
void initialize_data(unsigned char *buf, uint32_t size);

/*global variables*/
int initial_data_size = -1;
int last_data_size = -1;
int print_enabled = 0;
int validate_enabled = 0;
int family = AF_INET;
int type = -1;
int protocol = 0;
int partner = -1;
int mode = 0;
int data_size = -1;
int backlog = 5;
unsigned long my_address = -1;
unsigned long partner_address = -1;
unsigned char *reader, *writer;
int iteration_num = -1;

void* aligned_malloc(size_t size) {
	void *ptr;
	void *p = malloc(size+ALIGN-1+sizeof(void*));
	if(p!=NULL) {
		ptr = (void*) (((unsigned int)p + sizeof(void*) + ALIGN -1) & ~(ALIGN-1));
		*((void**)((unsigned int)ptr - sizeof(void*))) = p;
		return ptr;
	}
	return NULL;
}

void aligned_free(void *p) {
	void *ptr = *((void**)((unsigned int)p - sizeof(void*)));
	free(ptr);
	return;
}


void client() {
	int flags = 0;
	int fd, ret, temp, i, cliLen, serLen;
	struct sockaddr_in client, server;
	
	printf("in client\n");
	/*socket*/
	fd = socket(family, type, protocol);
	if(fd<0) {
		perror("socket");
		exit(-1);
	}
	/*bind*/
	client.sin_family = family;
	client.sin_addr.s_addr = my_address;
	client.sin_port = htons(CLIENT_PORT);
	ret = bind(fd, (struct sockaddr *)&client, sizeof(client));
	if (ret<0) {
		perror("bind");
		exit(-1);
	}
	cliLen  = sizeof(client);
	/*connect*/
	server.sin_family = family;
	server.sin_port = htons(SERVER_PORT);	
	server.sin_addr.s_addr = partner_address;
	ret = connect(fd, (struct sockaddr *)&server, sizeof(server));
	if (ret<0) {
		perror("connect");
		exit(-1);
	}
	serLen = sizeof(server);
	for(data_size = initial_data_size; data_size <= last_data_size; data_size<<=1 ) {
	/*write*/
		writer = (unsigned char*) aligned_malloc(data_size);
		reader = (unsigned char*) aligned_malloc(data_size);
		initialize_data(writer, data_size);
		if (print_enabled) {
        	printf("%s: I m going to write \n", __func__);
        	print(writer, data_size);
        }     
		for(i = 0; i < iteration_num; i++) {
			//ret = write(fd, writer, data_size);
			ret = sendto(fd, writer, data_size, flags, (struct sockaddr*) &server, (socklen_t) serLen);
			if (ret<0) {
				perror("write");
				exit(-1);
			}
			printf("%s : I wrote %d bytes\n", __func__, ret); 
			//ret = read(fd, reader, data_size);
    			ret = recvfrom(fd, reader, data_size, flags, (struct sockaddr*) &server, (socklen_t *) &serLen);
			if (ret<0) {
				perror("read");
				exit(-1);
			}
			printf("%s : I read %d bytes\n", __func__, ret);
			ret = validate_data(reader, writer, data_size);
			printf("i=%d, %d ",i, data_size);
		}
		aligned_free(reader);	
		aligned_free(writer);
		if (ret==0) printf("%d ", data_size);
	}
	printf("\n");
	if (ret == 0)
		printf("OK!\n");
	close(fd);
	return;
}


void server_stream() {
        int fd, ret, temp, new_fd, cliLen, i, serLen;
        struct sockaddr_in client, server;
	int flags = 0;

        printf("In server_stream\n");
        /*socket*/
        fd = socket(family, type, protocol);
        if (fd<0) {
                perror("socket");
                exit(-1);
        }
        server.sin_family = family;
        server.sin_addr.s_addr = my_address;
        server.sin_port = htons(SERVER_PORT);
	serLen = sizeof(server);
        /*bind*/
        ret = bind(fd, (struct sockaddr *) &server, sizeof(server));
        if (ret<0) {
                perror("bind");
                exit(-1);
        }
	/*listen*/
	ret = listen(fd, backlog);
	if (ret<0) {
		perror("listen");
		exit(-1);
	}
	/*accept*/
	client.sin_family = family;
	cliLen = sizeof(client);
	ret = accept(fd, (struct sockaddr *)&client, &cliLen);
	if (ret<0) {
		perror("accept");
		exit(-1);
	} 
	new_fd = ret;
	/*read*/
	for(data_size = initial_data_size; data_size<= last_data_size; data_size<<=1) {
    		reader = (unsigned char*) aligned_malloc(data_size);
		for(i=0; i < iteration_num; i++) {
			//ret = read(new_fd, reader, data_size);
    	    		ret = recvfrom(new_fd, reader, data_size, flags, (struct sockaddr *) &client, (socklen_t *) &cliLen);
        		if (ret<0) {
                		perror("read");
                		exit(-1);
        		}
       			if (print_enabled) {
                		printf("%s: I have read :\n", __func__);
                		print(reader, data_size);
        		}
        		/*write*/
			//ret = write(new_fd, reader, data_size);
        		ret = sendto(new_fd, reader, data_size, flags, (struct sockaddr*) &client, (socklen_t) cliLen);
        		if (ret<0) {
                		perror("write");
                		exit(-1);
        		}
		}
		aligned_free(reader);
	}	
	sleep(2);
	close(fd);
	close(new_fd);
        return;
}

void server_dgram() {
	int fd, ret, temp, i, cliLen, serLen;
	struct sockaddr_in client, server;
	int flags = 0;

	printf("In server_dgram\n");
	/*socket*/
	fd = socket(family, type, protocol);
	if (fd<0) {
		perror("socket");
		exit(-1);
	} 
	server.sin_family = family;
	server.sin_addr.s_addr = my_address;
	server.sin_port = htons(SERVER_PORT);
	serLen = sizeof(server);
	/*bind*/
	ret = bind(fd, (struct sockaddr *) &server, sizeof(server));
	if (ret<0) {
		perror("bind");
		exit(-1); 
	}

	/***********************/
	client.sin_family = family;
	client.sin_addr.s_addr = partner_address;
        client.sin_port = htons(CLIENT_PORT);
	cliLen = sizeof(client);
        ret = connect(fd, (struct sockaddr *) &client, sizeof(client));
        if (ret<0) {
                perror("connect");
                exit(-1);
        }
	/***********************/
	for(data_size=initial_data_size; data_size<=last_data_size; data_size<<=1) {
	/*read*/
		reader = (unsigned char*) aligned_malloc(data_size);
		for(i=0; i<iteration_num; i++) {
			//ret = read(fd, reader, data_size);
			ret = recvfrom(fd, reader, data_size, flags, (struct sockaddr *) &client, (socklen_t *) &cliLen);
			if (ret<0) {
				perror("read");
				exit(-1);
			}	
			printf("%s : I read %d bytes\n", __func__, ret);
			if (print_enabled) {
				printf("%s: I have read :\n", __func__);
				print(reader, data_size);
			}
			/*write*/
			//ret  = write(fd, reader, data_size);
			ret = sendto(fd, reader, data_size, flags, (struct sockaddr *) &client, (socklen_t) cliLen);
			if (ret<0) {
				perror("write");
				exit(-1);
			}
			printf("%s : I wrote %d bytes\n", __func__, ret);
		}
		aligned_free(reader);
	}
	close(fd);
	return;
}

void print_usage() {

	return;
}


void print(unsigned char *buf, uint32_t size) {
	int i;
	for(i=0; i<size; i++) 
		printf("%d ", buf[i]);
	printf("\n\n");
	return;
}


int validate_data(unsigned char* reader, unsigned char* writer, uint32_t size) {
	int i;
	for(i=0; i<size; i++)
		if(reader[i] != writer[i]) {
			printf("Data corruption: %d, %d, %d,\n", reader[i], writer[i], i);
			return(-1);
		}
	return 0;
} 

void initialize_data(unsigned char *buf, uint32_t size) {
	int i;
	for(i=0; i<size; i++)
		buf[i] = (unsigned char) rand();
	return;
}


int main(int argc, char** argv) {
	int c;
	uint32_t ring_size;
	int temp;

	while ((c = getopt(argc, argv, "o:m:scdtxr:b:e:n:vph")) != -1)
		switch(c) {
		case 'o':/*partners id*/
			partner_address = inet_addr(optarg);
			printf("Partner id = %d\n", partner_address);
			break;	
		case 'm':/*my id*/
			my_address = inet_addr(optarg);
			printf("My id = %d\n", my_address);
			break;
		case 's':
			mode |= FLAG_SERVER;
			break;
		case 'c':
			mode |= FLAG_CLIENT;
			break;
		case 'd':
			mode |= FLAG_DGRAM;
			type = SOCK_DGRAM;
			break;
		case 't':
			mode |= FLAG_STREAM;
			type = SOCK_STREAM;
			break;
		case 'x':
			my_address = partner_address;
			break;
		case 'r':
			ring_size = atoi(optarg);
			protocol = ring_size;
			break;
		case 'b':
			initial_data_size = atoi(optarg);
			break;
		case 'e':
			last_data_size = atoi(optarg);
		case 'n':
			iteration_num = atoi(optarg);
			break;
		case 'v':
			validate_enabled = 1;
			break;
		case 'p':
			print_enabled = 1;
			break;
		default:
			printf("Unknown option -%c\n", c);
		case 'h':
			print_usage();
			exit(-1);
			break;
		}
	
	temp = mode&FLAG_SERVER;
	printf("temp = %d\n", temp);
	if (temp) {
		temp = mode&FLAG_DGRAM;
		printf("in here %d\n", temp);
		if (temp) 
			server_dgram();
		else
			server_stream();
	}
	else
		client();

	return 0;
}
