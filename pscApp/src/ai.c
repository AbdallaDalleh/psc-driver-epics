#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <callback.h>
#include <aiRecord.h>
#include <recSup.h>
#include <devSup.h>
#include <dbAccess.h>
#include <epicsExport.h>

#include "ethernet.h"

static long init_record(struct aiRecord* record)
{
	if(record->inp.type != INST_IO) {
		printf("Invalid record type\n");
		return -1;
	}

	char* token = strtok(record->inp.value.instio.string, " ");
	if(token == NULL) {
		printf("Could not parse INP field.\n");
		return -2;
	}

	psc_t* device = psc_get_device(token);
	if(device == NULL) {
		printf("Could not find device with name: %s\n", token);
		return -3;
	}
	strncpy(device->name, token, sizeof(device->name));

	token = strtok(NULL, " ");
	if(token == NULL) {
		printf("Could not parse INP field: Address\n");
		return -2;
	}
	device->rx.address = atoi(token);

	token = strtok(NULL, " ");
	if(token == NULL) {
		printf("Could not parse INP field: PS\n");
		return -2;
	}
	device->rx.address = device->rx.address | ( atoi(token) << PS_ADDRESS_SHIFT );

	record->dpvt = (void*) device;
	return 0;
}

static long read_ai(struct aiRecord* record)
{
	int status;
	psc_t* device = (psc_t*) record->dpvt;
	if(record->pact) {
		status = psc_read(device, (float*) &record->val);
		printf("Completed processing. Value %f\n\n", record->val);
		return 2;
	}

	record->pact = true;
	dbScanLock((struct dbCommon*) record);
	(record->rset->process)(record);
	dbScanUnlock((struct dbCommon*) record);
	return 2;
}

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioinit_info;
	DEVSUPFUN io;
	DEVSUPFUN misc;
} dev_ai = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	NULL
};

epicsExportAddress(dset, dev_ai);

