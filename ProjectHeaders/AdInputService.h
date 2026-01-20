#ifndef AdInputService_H
#define AdInputService_H

#include "ES_Types.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AdInputService.h"
#include "PIC32_AD_Lib.h"
#include <xc.h>

bool InitAdInputService(uint8_t Priority);

bool PostAdInputService(ES_Event_t ThisEvent);

ES_Event_t RunAdInputService(ES_Event_t ThisEvent);

uint16_t GetIntervalfromADC(void);

#endif

