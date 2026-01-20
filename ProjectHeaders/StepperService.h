#ifndef STEPPER_SERVICE_PWM_H
#define STEPPER_SERVICE_PWM_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AdInputService.h"
#include <xc.h>

bool InitStepperService(uint8_t Priority);
ES_Event_t RunStepperService(ES_Event_t ThisEvent);
bool PostStepperService(ES_Event_t ThisEvent);

#endif
