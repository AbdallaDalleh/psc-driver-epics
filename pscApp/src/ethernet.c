
#include "ethernet.h"

psc_t* psc_init(char* ip, u16 port)
{
	int status;
	struct sockaddr_in socket_address;
	psc_t* device = (psc_t*) malloc(sizeof(psc_t));

	device->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(device->fd < 0)
		return NULL;

	strncpy(device->ip, ip, sizeof(device->ip));
	device->ip[strlen(device->ip)] = '\0';
	device->port = port;
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(device->ip);
	socket_address.sin_port = htons(device->port);
	status = connect(device->fd, (struct sockaddr*) &socket_address, sizeof(socket_address));
	if(status < 0)
		return NULL;

	device->rx.status  = (u16) PACKET_STATUS_RAW;
	device->rx.command = (u16) COMMAND_READ;
	device->rx.address = (u16) ADDRESS_ILOAD;
	device->rx.data	= (u32) 0;

	device->tx.status  = (u16) PACKET_STATUS_RAW;
	device->tx.command = (u16) COMMAND_WRITE;
	device->tx.address = (u16) ADDRESS_SET_REF;
	device->tx.data	= (u32) 0;

	device->poller[0].fd = device->fd;
	device->poller[0].events = POLLIN;
	device->poller[0].revents = 0;
	return device;
}

int psc_read(psc_t* device, float* value)
{
	int status;

	status = write(device->fd, &device->rx, sizeof(device->rx));
	if(status < 0)
		return PSC_WRITE;
	
	status = poll(device->poller, 1, POLL_TIMEOUT);
	if(status <= 0)
		return PSC_POLL;

	status = read(device->fd, &device->rx, sizeof(device->rx));
	if(status < 0)
		return PSC_READ;

	*value = *(float*)(&device->rx.data);
	return PSC_OK;
}

int psc_write(psc_t* device)
{
	int status;

	status = write(device->fd, &device->tx, sizeof(device->tx));
	if(status < 0)
		return PSC_WRITE;

	status = poll(device->poller, 1, POLL_TIMEOUT);
	if(status <= 0)
		return PSC_POLL;

	status = read(device->fd, &device->tx, sizeof(device->tx));
	if(status < 0)
		return PSC_READ;

	return PSC_OK;
}

