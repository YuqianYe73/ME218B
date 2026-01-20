#include "ES_Configure.h"
#include "ES_Framework.h"
#include "StepperService.h"
#include "AdInputService.h" 
#include "dbprintf.h"
#include "PWM_PIC32.h"
#include <xc.h>

// Define pin connected with motor: RB10-RB13
#define MAX_STEPS 3000 

// Priority variable
static uint8_t MyPriority;

static uint8_t DutyCycle1[16] = {71, 92, 100, 92, 71, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38}; // A+
static uint8_t DutyCycle2[16] = {0, 0, 0, 0, 0, 0, 0, 38, 71, 92, 100, 92, 71, 38, 0, 0}; // A-
static uint8_t DutyCycle3[16] = {71, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 71, 92, 100, 92}; // B+
static uint8_t DutyCycle4[16] = {0, 0, 0, 38, 71, 92, 100, 92, 71, 38, 0, 0, 0, 0, 0, 0}; // B-

// Static variable currentStepIndex = 0
static uint8_t currentStepIndex = 0;

// Direction variable: 1 for forward, -1 for backward
static int8_t currentDirection = 1;

//For step count
static uint16_t stepCount = 0;

/**
 * Set stepper motor pins to the current step order in FullStepTable
 */
static void UpdateMicrosteppingPWM(void) {
    // Duty cycle values for each phase
    PWMOperate_SetDutyOnChannel(DutyCycle1[currentStepIndex], 1); // A+
    PWMOperate_SetDutyOnChannel(DutyCycle2[currentStepIndex], 2); // A-
    PWMOperate_SetDutyOnChannel(DutyCycle3[currentStepIndex], 3); // B+
    PWMOperate_SetDutyOnChannel(DutyCycle4[currentStepIndex], 4); // B-
}

/**
 * Initialize hardware pins and start initial step timer
 */
bool InitStepperServicePWM(uint8_t Priority) {
    ES_Event_t ThisEvent;
    MyPriority = Priority;

    //Set RB9 as digital input: direction change switch
    TRISBSET = (1 << 8);
    ANSELBCLR = (1 << 8);

    // Set up PWM channel for stepper motor control
    PWMSetup_BasicConfig(4);
    // Map PWM channels to output pins 
    PWMSetup_MapChannelToOutputPin(1, PWM_RPA0); //RA0 pin16 A+
    PWMSetup_MapChannelToOutputPin(2, PWM_RPB5); //RB5 pin14 A-
    PWMSetup_MapChannelToOutputPin(3, PWM_RPB9); //RB9 pin18 B+
    PWMSetup_MapChannelToOutputPin(4, PWM_RPA2); //RA2 pin9 B-

    for (uint8_t i = 1; i <= 4; i++) {
        PWMSetup_AssignChannelToTimer(i, _Timer2_); // All using Timer2
    }
    PWMSetup_SetFreqOnTimer(20000, _Timer2_); // Carrier Frequency Set to 20kHz

    // Start initial step timer (1.2: 100ms, 10 steps per second)
    ES_Timer_InitTimer(STEP_TIMER_PWM, 50); 

    // Post the initial transition event
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * Run stepper motor by running stepper service and handling events
 */
ES_Event_t RunStepperServicePWM(ES_Event_t ThisEvent) {

    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    switch (ThisEvent.EventType) {
        // Handle step timer timeout event
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == STEP_TIMER_PWM) {
                if (stepCount < MAX_STEPS) {
                    stepCount++;
                
                // read RB8, 1 << 8 corresponds to Pin 16 (RB7)
                if ((PORTB & (1 << 8)) != 0) {
                    // If RB8 is high, set to forward, currentStepIndex = currentStepIndex + 1
                    currentDirection = 1;
                    currentStepIndex++;
                } else {
                    // If RB8 is low (connected to GND), set to backward, currentStepIndex = currentStepIndex - 1
                    currentDirection = -1;
                    currentStepIndex--;
                }

                // Wrap index around 0-15 using AND 0x0F
                currentStepIndex &= 0x0F;
                
                // Set stepper motor pins to new step pattern
                UpdateMicrosteppingPWM();

                uint32_t AdInterval = GetIntervalfromADC();

                // If interval is non-zero, restart timer
                if (AdInterval > 0) {
                    ES_Timer_InitTimer(STEP_TIMER_PWM, AdInterval);
                }
                }
                else{
                    // If stepCount reaches MAX_STEPS, stop the motor
                    ES_Timer_InitTimer(STEP_TIMER_PWM, 0);
                }
            }
            break;

        case ES_INIT:
            stepCount = 0;
            break;
        default:
            break;
    }

    return ReturnEvent;
}

bool PostStepperServicePWM(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

