
#include "ethernet.h"

static psc_t devices[NUMBER_OF_DEVICES];
static u32   deviceCount;

static long init()
{
	int status = 0;
	int i = 0;
	for(i = 0; i < deviceCount; i++) {
		status = psc_init(&devices[i], devices[i].ip, PORT);
		if(status != PSC_OK) {
			printf("Could not initialize device: Name: %s, IP: %s\n", devices[i].name, devices[i].ip);
			continue;
		}
	}

	return 0;
}

int psc_init(psc_t* device, char* ip, u16 port)
{
	int status;
	struct sockaddr_in socket_address;
	// psc_t* device = (psc_t*) malloc(sizeof(psc_t));

	device->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(device->fd < 0)
		return PSC_SOCKET;

	strncpy(device->ip, ip, sizeof(device->ip));
	device->ip[strlen(device->ip)] = '\0';
	device->port = port;
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(device->ip);
	socket_address.sin_port = htons(device->port);
	status = connect(device->fd, (struct sockaddr*) &socket_address, sizeof(socket_address));
	if(status < 0)
		return PSC_CONNECT;

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
	return PSC_OK;
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

static const iocshArg  configureArg0 = { "name", iocshArgString };
static const iocshArg  configureArg1 = { "ip", iocshArgString };
static const iocshArg* configureArgs[] = 
{
    &configureArg0,
    &configureArg1
};
static const iocshFuncDef configureDef = { "pscConfigure", 2, configureArgs };
static long  configure(char *name, char *ip)
{
	struct hostent *hostentry;
	struct in_addr **addr_list;

	if(deviceCount >= NUMBER_OF_DEVICES)
	{
		printf("\x1B[31m[evr][] Unable to configure device: Too many devices\r\n\x1B[0m");
		return -1;
	}
	if(!name || !strlen(name) || strlen(name) >= NAME_LENGTH)
	{
		printf("\x1B[31m[evr][] Unable to configure device: Missing or incorrect name\r\n\x1B[0m");
		return -1;
	}
	if(!ip)
	{
		printf("\x1B[31m[evr][] Unable to configure device: Missing or incorrect ip\r\n\x1B[0m");
		return -1;
	}

	if((hostentry = gethostbyname(ip)) == NULL) 
	{
		printf("\x1B[31mUnable to configure device: Could not resolve hostname\r\n\x1B[0m");
		return -1;
	}
	addr_list = (struct in_addr **) hostentry->h_addr_list;

	if(addr_list[0] != NULL)
		strncpy(devices[deviceCount].ip, inet_ntoa(*addr_list[0]), IP_LENGTH);
	else
		strncpy(devices[deviceCount].ip, ip, IP_LENGTH);

	strcpy(devices[deviceCount].name, name);

	deviceCount++;
	return 0;
}

static void configureFunc (const iocshArgBuf *args)
{
    configure(args[0].sval, args[1].sval);
}

static void pscRegister(void)
{
	iocshRegister(&configureDef, configureFunc);
}

static drvet drv_psc = {
    2,
    NULL,
    init
};
epicsExportAddress(drvet, drv_psc);
epicsExportRegistrar(pscRegister);

