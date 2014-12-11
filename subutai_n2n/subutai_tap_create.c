/**
 * (C) 2014-09 - 	Ozlem Ceren Sahin <ocrnshn@gmail.com>
 *           		Emin Inal <emininal@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not see see <http://www.gnu.org/licenses/>
 *
 * Code contributions courtesy of:
 * Ozlem Ceren Sahin <ocrnshn@gmail.com>
 * Emin Inal <emininal@gmail.com>
 *
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
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>


int fd_sock;
struct sockaddr_un addr;
	
/**
*
* End function, it called when pressing ctrl + c or sudo kill
* It deletes socket files created before, closes the opened socket
* and exits properly.
*
*/
void endProcess()
{
	unlink(addr.sun_path);
	close(fd_sock);
	exit(0);
}

/**
*
* Signal Handler function, it catches SIGINT occured by ctrl + c and
* it catches SIGTERM occured by sudo kill
*
*/
void signalHandler(int signal)
{
	if(signal == SIGINT || signal == SIGTERM)
	{
		endProcess();
	}
}

int tuntap_open(char *dev, char *device_mac, char *address_mode, char *device_ip, char* device_mask, int mtu)
{
	printf("Tuntap opening is starting with these parameters: %s %s %s %s %s %d\n",
				dev, device_mac, address_mode, device_ip, device_mask, mtu);
	struct ifreq ifr;
	int fd, msgsock;
	char *tuntap_device = "/dev/net/tun";

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
	
	if(system(buf) == -1)
	{
		printf("Error on system call for ip assignment");
	}
	
	char socket_path[50];
	strcpy(socket_path, "/var/run/n2n/sockets/socket_");
	strcat(socket_path, device_ip);
	
	unlink(socket_path);
	
	if ((fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket opening problem");
		exit(1);
	}
	
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, socket_path);
	
	if (bind(fd_sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)))
	{
		perror("Binding stream socket");
		exit(1);
	}
	
	listen(fd_sock, 1);
	
	while(1)
	{
		msgsock = accept(fd_sock, 0, 0);
		if (msgsock == -1)
			perror("error on message socket");
	
	
		int k = ancil_send_fd(msgsock,fd);
		if(k == 0)
		{
			printf("Ancillary send is successful.\n");
		}
		else
		{
			printf("Ancillary send is NOT successful!!\n");
		}
	
	
	
	}
	
	
	return(fd);
}


int main(int argc, char** argv)
{
	char *dev = "edge0"; 
	char *device_mac = ""; //"5a:96:79:71:dc:91";
	char *address_mode = "dhcp";
	char *device_ip = "10.11.12.13";
	char* device_mask = "255.255.255.0";
	int mtu = 1400;
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
			case '?':
				if(optopt == 'a' || optopt == 'd' )
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
	
	signal(SIGINT, signalHandler); /* for ctrl + c */
	signal(SIGTERM, signalHandler); /* for sudo kill */

	tuntap_open(dev, device_mac, address_mode, device_ip, device_mask, mtu);
	while(1) {}
}



