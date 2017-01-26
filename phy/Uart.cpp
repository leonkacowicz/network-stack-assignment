#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <net/if.h>

#include "Uart.h"

namespace phy {
	int sd;								// Socket descriptor
	struct sockaddr_in client_addr;		// Client address
	int client_addr_len;				// Client address size
	struct sockaddr_in broadcast_addr;	// Broadcast address
	int broadcast_addr_len;				// Broadcast adresss size
	static const int uart_port = 7455;	// UDP port
	char interface_name[100] = {""};

	int BuildClientAddress() {
		int ret;
		sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sd == -1) {
			return 0;
		}

		#ifdef SO_BSDCOMPAT
		{
		   int one=1;
		   setsockopt(sd, SOL_SOCKET, SO_BSDCOMPAT, &one,sizeof(one));
		   setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &one,sizeof(one));
		}
		#endif

		client_addr.sin_family       = AF_INET;
		client_addr.sin_port         = htons(uart_port);
		client_addr.sin_addr.s_addr  = INADDR_ANY;
		client_addr_len              = sizeof(client_addr);
		ret = bind(sd, (struct sockaddr *)&client_addr, client_addr_len);
		if (ret == -1) {
			return 0;
		}
		return 1;
	}

	int DiscoverBroadcastAddress() {
		struct ifconf   interface_conf;
		struct ifreq   *net_interface;
		int   interface_num, result, i, cmp;
		char  buffer[1024];

		interface_conf.ifc_len = sizeof(buffer);
		interface_conf.ifc_buf = buffer;
		result = ioctl(sd, SIOCGIFCONF, (char *) &interface_conf);
		net_interface = interface_conf.ifc_req;
		interface_num = interface_conf.ifc_len/sizeof(struct ifreq);

		for(i = 0; i < interface_num; net_interface++) {
			//if has defined an interface, get it.
			if(interface_name[0]=='\0') {
				cmp= strcmp("lo0",net_interface->ifr_ifrn.ifrn_name) && strcmp("lo",net_interface->ifr_ifrn.ifrn_name);

			} else { //has the interface the same name of defined interface
				cmp = !strcmp(interface_name,net_interface->ifr_ifrn.ifrn_name);
			}
	
			if (cmp && (net_interface->ifr_addr.sa_family == AF_INET) &&
				    (net_interface->ifr_flags & IFF_BROADCAST)) {
	
				result = ioctl(sd, SIOCGIFBRDADDR, (char *) net_interface);
	
				if (result >= 0) {
					memcpy((char *)&broadcast_addr, (char *)(&(net_interface->ifr_broadaddr)), sizeof(net_interface->ifr_broadaddr));
					broadcast_addr.sin_port = htons(uart_port);
					broadcast_addr_len      = sizeof(broadcast_addr);

					//TeleMidia broadcast:
					//inet_aton("139.82.95.63", &broadcast_addr.sin_addr);

					return 1;
				}
			}
			i++;
		}
	
		return 0;
	}

	Uart::Uart() {
		if (BuildClientAddress() != 0) {
			if (DiscoverBroadcastAddress() != 0) {
				srand(time(NULL));

				dataIndicationFlag = false;
				errorProb = 0.0;
			}
		}
	}

	Uart::~Uart() {
		close(sd);
	}

	void Uart::dataRequest(char data) {
		int result;

		if (rand()%100 < errorProb) {
			data = (data & 0xFE);
		}

		result = sendto(
			    sd,
			    (char*)&data,
			    sizeof(data),
			    0,
			    (struct sockaddr*)&broadcast_addr,
			    broadcast_addr_len);

		if (result == -1)
			fprintf(stderr, "Warning! sendto dataRequest() (ERRNO = %d).\n", errno);
	}

	bool Uart::dataIndication() {
		return dataIndicationFlag;
	}

	char Uart::dataReceive() {
		dataIndicationFlag = false;
		char data;
		memcpy((char*)&data, (char*)&inputData, sizeof(inputData));
		inputData = '\0';
		return data;
	}

	void Uart::setErrorProb(float prob) {
		this->errorProb = prob;
	}

	float Uart::getErrorProb() {
		return errorProb;
	}

	bool Uart::setInterfaceName(char* interface) {
		if (strlen(interface) > 99)
			return false;

		strcpy(interface_name, interface);
		return true;
	}

	void Uart::mainLoop() {
		int numfds, result;
		fd_set fdset;
		struct timeval tv_timeout;

		numfds= sd + 1;
		FD_ZERO(&fdset);
		FD_SET(sd, &fdset);
		tv_timeout.tv_sec  = 0;
		tv_timeout.tv_usec = 0;

		result= select(numfds, &fdset, NULL, NULL, &tv_timeout);

		switch (result) {
			case -1:
				fprintf(stderr, "Warning! select (ERRNO = %d).\n", errno);
				break;

			case 1:
				while(dataIndicationFlag) {
					usleep(200000);
					fprintf(stderr, "Warning! receive buffer overflow.\n");
				}

				// receive data
				result= recv(sd, (char*)&inputData, sizeof(inputData), 0);
				if (result == -1) {
					fprintf(stderr, "Warning! receive data (ERRNO = %d).\n", errno);
            		return;
         		}

				dataIndicationFlag = true;
				break;

			default:
				break;
		}
	}
}
