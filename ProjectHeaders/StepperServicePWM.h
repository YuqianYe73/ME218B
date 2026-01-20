#ifndef STEPPER_SERVICE_H
#define STEPPER_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AdInputService.h"
#include <xc.h>

bool InitStepperServicePWM(uint8_t Priority);
ES_Event_t RunStepperServicePWM(ES_Event_t ThisEvent);
bool PostStepperServicePWM(ES_Event_t ThisEvent);

#endif

