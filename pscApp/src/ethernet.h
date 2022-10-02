#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <poll.h>

#include <epicsExport.h>
#include <drvSup.h>
#include <iocsh.h>

#pragma pack(2)

#define PSC_OK		0
#define PSC_SOCKET	1
#define PSC_READ	2
#define PSC_WRITE	3
#define PSC_CONNECT	4
#define PSC_POLL	5

#define UDP_PACKET_LENGTH	10
#define PACKET_STATUS_RAW  	0x0000
#define COMMAND_READ		0x0001
#define COMMAND_WRITE		0x0002
#define PS_ADDRESS_SHIFT	14
#define ADDRESS_PRIORITY	0x2 | (1 << PS_ADDRESS_SHIFT)
#define ADDRESS_ILOAD		153 | (1 << PS_ADDRESS_SHIFT)
#define ADDRESS_SET_REF		175 | (1 << PS_ADDRESS_SHIFT)
#define ETHERNET_ENABLE		0x1000000
#define POLL_TIMEOUT		1000
#define PORT				9322
#define NUMBER_OF_DEVICES	69
#define NAME_LENGTH			20
#define IP_LENGTH			20

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int32_t  i32;
typedef struct pollfd pollfd;

typedef struct
{
	u16 status;
	u16 command;
	u16 address;
	u32 data;
} packet_t;

typedef struct 
{
	char name[NAME_LENGTH];
	char ip[IP_LENGTH];
	int  port;
	int  fd;

	pollfd   poller[1];
	packet_t rx;
	packet_t tx;
	packet_t eth;
} psc_t;

int psc_init(psc_t* device, char* ip, u16 port);
int psc_read(psc_t* device, float* value);
int psc_write(psc_t* device);

