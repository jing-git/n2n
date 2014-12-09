/*
 * test.cpp
 *
 *  Created on: Dec 2, 2014
 *      Author: emin
 */


#include <net/if.h>
#include <linux/if_tun.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ancillary.h"
#include <sys/un.h>
#include <sys/socket.h>

#define NAME "/tmp/socket"

#define DATA "The sea is calm tonight, the tide is full . . ."

int tuntap_open(char *dev, char *device_mac, char *address_mode, char *device_ip, char* device_mask, int mtu)
{

	printf("Tuntap opening is starting with these parameters: %s %s %s %s %s %d\n",
				dev, device_mac, address_mode, device_ip, device_mask, mtu);
	struct ifreq ifr;
	int fd, fd_sock;
	char *tuntap_device = "/dev/net/tun";
	struct sockaddr_un addr;

#define N2N_LINUX_SYSTEMCMD_SIZE 128

	char buf[N2N_LINUX_SYSTEMCMD_SIZE];
	int rc;

	fd = open(tuntap_device, O_RDWR);

	if(fd < 0) {
		printf("ERROR: ioctl() [%s][%d]\n", strerror(errno), errno);
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP|IFF_NO_PI; /* Want a TAP device for layer 2 frames. */
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	rc = ioctl(fd, TUNSETIFF, (void *)&ifr);

	if(rc < 0) {
		printf("error on ioctl\n");
		close(fd);
		return -1;
	}

	if ( device_mac && device_mac[0] != '\0' )
	{
		snprintf(buf, sizeof(buf), "/sbin/ifconfig %s hw ether %s",
				ifr.ifr_name, device_mac );
		system(buf);
	}

	if ( 0 == strncmp( "dhcp", address_mode, 5 ) )
	{
		snprintf(buf, sizeof(buf), "/sbin/ifconfig %s %s mtu %d up",
				ifr.ifr_name, device_ip, mtu);
	}
	else
	{
		snprintf(buf, sizeof(buf), "/sbin/ifconfig %s %s netmask %s mtu %d up",
				ifr.ifr_name, device_ip, device_mask, mtu);
	}

	system(buf);



	fd_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd_sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, NAME);

	if (connect(fd_sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
		close(fd_sock);
		perror("connecting stream socket");
		exit(1);
	}


	int k = ancil_send_fd(fd_sock,fd);
	if(k == 0)
	{
		printf("Ancillary send is successful.\n");
	}
	else
	{
		printf("Ancillary send is NOT successful!!\n");
	}

	return(fd);
}


int main(int argc, char** argv)
{

	char *dev = "edge0"; 
	char *device_mac = "5a:96:79:71:dc:91";
	char *address_mode = "dhcp";
	char *device_ip = "10.11.12.13";
	char* device_mask = "255.255.255.0";
	int mtu = 1500;
	int c;
	while ((c = getopt (argc, argv, "d:m:a:s:M:")) != -1)
		switch (c)
		{
	    	case 'a':
				address_mode = "static";
				device_ip = optarg;
				break;
	    	case 'd':
				dev = optarg;
				break;
	    	case 'm':
				device_mac = optarg;
				break;
			case 's':
				device_mask = optarg;
				break;
			case 'M':
				mtu = atoi(optarg);
				break;
	    	case '?':
				if(optopt == 'a' || optopt == 'd' || optopt == 'm' || optopt == 's' || optopt == 'M' )
		  			fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
		  			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
		  			fprintf (stderr,
		           		"Unknown option character `\\x%x'.\n",
		           		optopt);
				return 1;
	      	default:
				abort ();
		}

	tuntap_open(dev, device_mac, address_mode, device_ip, device_mask, mtu);
	while(1) {}
}



