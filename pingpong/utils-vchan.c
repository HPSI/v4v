#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

#define __USE_GNU
#include <dlfcn.h>

#include <libxenvchan.h>

#include "utils-vchan.h"

struct socket_data sk_data;

unsigned long inet_addr(char *cp) {                                                                                                                                     
        int ret = -1;

        if(!strcmp(cp,"147.102.4.234"))
                ret = 1;
        if(!strcmp(cp,"147.102.4.235"))
                ret = 2;
        if(!strcmp(cp,"147.102.4.236"))
                ret = 3;
        if(!strcmp(cp,"147.102.4.237"))
                ret = 4;
        if(!strcmp(cp,"147.102.4.238"))
                ret = 5;
        if(!strcmp(cp,"147.102.4.239"))
                ret = 6;
        if(!strcmp(cp,"147.102.4.240"))
                ret = 7;
        if(!strcmp(cp,"147.102.4.241"))
                ret = 8;
        if(!strcmp(cp,"147.102.4.242"))
                ret = 9;
        if(!strcmp(cp,"147.102.4.243"))
                ret = 10;
        if(!strcmp(cp,"147.102.4.244"))
                ret = 11;
        if(!strcmp(cp,"147.102.4.245"))
                ret = 12;
        if(!strcmp(cp,"147.102.4.246"))
                ret = 13;
        if(!strcmp(cp,"147.102.4.247"))
                ret = 14;
        if(!strcmp(cp,"147.102.4.248"))
                ret = 15;
        if(!strcmp(cp,"147.102.4.249"))
                ret = 16;
//printf("paparia:%d \n", ret);

        return ret;

}

int socket(int domain, int type, int protocol) {
	static int (*socket_real)(int, int, int)=NULL;
	int sock_ret;
        int ret = 0;

	sk_data.real_socket_fd = 0;
	if (domain == PF_UNIX || domain == PF_FILE){ 
		socket_real=dlsym(RTLD_NEXT,"socket");
		
		sock_ret = sk_data.real_socket_fd = socket_real(domain, type, protocol);

		return sock_ret;
	}


#if 0
	sk_data = (struct socket_data *)malloc(sizeof(struct socket_data));	
printf(":( sk_data: 0x%x\n", sk_data);

	if (!sk_data) {
		printf("Error allocating socket_data\n");
		return -1;
	}
#endif

	if (type == SOCK_STREAM) {
		sk_data.datagram = 0;
	}
	else {
		sk_data.datagram = 1;
	}
		sk_data.ring_size = protocol ? protocol : 2 * 524288;
		sk_data.real_socket_fd = 0xf00d;


	return sk_data.real_socket_fd;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        int ret = -1;
        struct sockaddr_in addr_in;     

        memcpy(&addr_in, addr, addrlen);
#if 0

	if (addr_in.sin_addr.s_addr == 2) {
		printf("EIMAI O SERVER\n");
		sk_data.ctrl = libxenvchan_server_init(NULL, DOMID_CLIENT, XS_PATH, sk_data.ring_size, sk_data.ring_size);
		sk_data.is_server = 1;
	}
	else {
		printf("EIMAI O CLIENT\n");
		sk_data.ctrl = libxenvchan_client_init(NULL, DOMID_SERVER, XS_PATH);
		sk_data.is_server = 0;
	}

        if (!(sk_data.ctrl)) {
                perror("libxenvchan_*_init");
		goto out;
        }

	sk_data.ctrl->blocking = 1;
#endif

	sprintf(sk_data.xs_path, "/local/domain/%d/memory", addr_in.sin_addr.s_addr); 
	ret = 0;

out:
        return ret;
}

int listen(int sockfd, int backlog) {

	return 0;
}

ssize_t read(int fd, void *buf, size_t count)
{
	static int (*read_real)(int, void*, size_t)=NULL;

	if (fd == 0xf00d) {
                return libxenvchan_read(sk_data.ctrl, buf, count);
	}

	read_real=dlsym(RTLD_NEXT,"read");
	
	return read_real(fd, buf, count);
	
}
ssize_t write(int fd, const void *buf, size_t count)
{
	static int (*write_real)(int, const void*, size_t)=NULL;
	if (fd == 0xf00d) {
                return libxenvchan_write(sk_data.ctrl, buf, count);
	}

	write_real=dlsym(RTLD_NEXT,"write");
	
	return write_real(fd, buf, count);
	
}
int     dump_sockaddr(FILE *tfp, struct sockaddr_in *sinptr)
        {
        int     k;

        /*  in <in.h>
        struct  sockaddr_in  {
                short   sin_family;
                u_short sin_port;
                struct  in_addr sin_addr;
                char    sin_zero[8];
        }       */

        if ( (sinptr == (struct sockaddr_in *)NULL) ||
             (tfp == (FILE *)NULL) )
                return(-1);                     /* insurance */

#if 1
        fprintf(tfp,
        "\n  struct sockaddr_in { ");
        k = sinptr->sin_family;
                                                /* print port number (or
                                0 = "to be assigned by system" */
        fprintf(tfp,
        "\n       u_short sin_port(=%hu);", ntohs(sinptr->sin_port));

                                                /* print ip address */
        fprintf(tfp,
        "\n       struct  in_addr sin_addr.s_addr(=%u=%d.%d.%d.%d);",
                        ntohl(sinptr->sin_addr.s_addr),
                        (ntohl(sinptr->sin_addr.s_addr)), 
                        (ntohl(sinptr->sin_addr.s_addr)),
                        (ntohl(sinptr->sin_addr.s_addr)),
                        (ntohl(sinptr->sin_addr.s_addr)));
#if 0
        fprintf(tfp,
        "\n       char    sin_zero[8](=%x %x %x %x %x %x %x %x);",
                        vaxunsign(sinptr->sin_zero[0]),
                        vaxunsign(sinptr->sin_zero[1]),
                        vaxunsign(sinptr->sin_zero[2]),                                                                                                                                         
                        vaxunsign(sinptr->sin_zero[3]),
                        vaxunsign(sinptr->sin_zero[4]),
                        vaxunsign(sinptr->sin_zero[5]),
                        vaxunsign(sinptr->sin_zero[6]),
                        vaxunsign(sinptr->sin_zero[7]));
#endif
        fprintf(tfp,
        "\n  } ");

        fflush(tfp);
#endif
        return(0);
}  /* end of dump_sockaddr */

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
        struct sockaddr_in addr_in;     

	//dump_sockaddr(stdout, addr);
        memcpy(&addr_in, addr, *addrlen);
	//printf("EIMAI O SERVER\n");
	//printf("addr:%d\n", addr_in.sin_addr.s_addr);
	sk_data.ctrl = libxenvchan_server_init(NULL, addr_in.sin_addr.s_addr, sk_data.xs_path, sk_data.ring_size, sk_data.ring_size);
	sk_data.is_server = 1;

        if (!(sk_data.ctrl)) {
                perror("libxenvchan_*_init");
		exit(1);
        }

	sk_data.ctrl->blocking = 1;

	if (sockfd == 0xf00d)
		return 0xf00d;
	return 0;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        static int (*connect_real)(int, const struct sockaddr *, socklen_t)=NULL;
	int ret = 0;
        struct sockaddr_in addr_in;     


	if (addr->sa_family != AF_INET) {
                connect_real=dlsym(RTLD_NEXT,"connect");

                return connect_real(sockfd, addr, addrlen);
	}

	//dump_sockaddr(stdout, addr);
        memcpy(&addr_in, addr, addrlen);
	//printf("addr:%d\n", addr_in.sin_addr.s_addr);
#if 1
		//printf("EIMAI O CLIENT\n");
		sk_data.ctrl = libxenvchan_client_init(NULL, addr_in.sin_addr.s_addr, sk_data.xs_path);
		sk_data.is_server = 0;

        if (!(sk_data.ctrl)) {
                perror("libxenvchan_*_init");
		ret = -1;
		goto out;
        }
	sk_data.ctrl->blocking = 1;
#endif
out:

	return ret;
}

int libxenvchan_read_all(struct libxenvchan *ctrl, void *buf, int size) {
        int written = 0;
        int ret;

        while (written < size) {
                ret = libxenvchan_read(ctrl, buf + written, size - written);
                if (ret < 0) {
                        perror("read");
			goto out;
                }
                written += ret;
        }

out:
        return written;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
        int ret = -1;

        if (sk_data.datagram) {
		ret = libxenvchan_read(sk_data.ctrl, buf, len);
	}
	else 
		ret = libxenvchan_read_all(sk_data.ctrl, buf, len);

	if (ret < 0) {
		perror("read");
	}

        return ret;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
	return recv(sockfd, buf, len, flags);
}

int libxenvchan_write_all(struct libxenvchan *ctrl, const void *buf, int size) {
        int written = 0;
        int ret;

        while (written < size) {
                ret = libxenvchan_write(ctrl, buf + written, size - written);
                if (ret < 0) {
                        perror("write");
			goto out;
                }

                written += ret;
        }

out:
        return written;
}


ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
        int ret = -1;

        if (sk_data.datagram) 
		ret = libxenvchan_write(sk_data.ctrl, buf, len);
	else 
		ret = libxenvchan_write_all(sk_data.ctrl, buf, len);

	if (ret != len) {
		printf("Could not write all data: %d, ret = %d\n", (int) len, ret);
	}
	if (ret < 0) {
		perror("write");
	}

        return ret;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {

	return send(sockfd, buf, len, flags);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
	return 0;
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;

        addr_in->sin_addr.s_addr = 2;
        addr_in->sin_port = 0x4132;
        addr_in->sin_family = AF_INET;
        return 0;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;

        addr_in->sin_addr.s_addr = 2;
        addr_in->sin_port = 0x4132;
        addr_in->sin_family = AF_INET;
        return 0;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
	return 0;
}

int fcntl (int __fd, int __cmd, ...)
{
return 0;}

#if 0
struct in_addr *inet_aton(char * in)
{
        struct sockaddr_in *addr_in = malloc(sizeof(*addr_in));
        addr_in->sin_addr.s_addr = in;
return addr_in;
}
char * inet_ntoa(struct in_addr in)
{
return "192.168.129.3";
}
#endif

#if 0
int close(int fd) {
	libxenvchan_close(sk_data.ctrl);
//	free(sk_data);

	return 0;
}
#endif


