#include "ES_Configure.h"
#include "ES_Framework.h"
#include "StepperService.h"
#include "AdInputService.h" 
#include "dbprintf.h"
#include <xc.h>

// Define pin connected with motor: RB10-RB13
#define MOTOR_MASK 0x3C00
#define MAX_STEPS 3000 

/*
 * FullStepTable
 * When Half Step currentStepIndex &= 0x03 3->6
 * AdInterval
 * Change MIN_INTERVAL to different values
 */

// Priority variable
static uint8_t MyPriority;

// Full step table[0b1010, 0b1001, 0b0101, 0b0110]
//static uint8_t FullStepTable[] = {0x0A, 0x09, 0x05, 0x06}; // Full Step
// Wave Drive table[0b1000, 0b0001, 0b0100, 0b0010]
//static uint8_t FullStepTable[] = {0x08, 0x01, 0x04, 0x02}; // Wave Drive
// Half step table[0b1010, 0b1000, 0b1001, 0b0001, 0b0101, 0b0100, 0b0110, 0b0010]
static uint8_t FullStepTable[] = {0x0A, 0x08, 0x09, 0x01, 0x05, 0x04, 0x06, 0x02}; // Half Step

// Static variable currentStepIndex = 0
static uint8_t currentStepIndex = 0;

// Direction variable: 1 for forward, -1 for backward
static int8_t currentDirection = 1;

//For step count
static uint16_t stepCount = 0;

/**
 * Set stepper motor pins to the current step order in FullStepTable
 */
static void SetStepperPins(void) {
    // Align 4-bit state to R pins B10-13 by shifting left 10 bits
    uint32_t stepData = (uint32_t)FullStepTable[currentStepIndex]; 
    uint32_t shiftedData = stepData << 10; // Example: 0b1010 -> 0b10100000000000

    // Clear RB10-13
    uint32_t currentPortState = LATB; 
    uint32_t clearedPort = currentPortState & ~MOTOR_MASK; // ~MOTOR_MASK: all bits except RB10-13 are 1, RB10-13 are 0

    // Fill the cleared bits with the aligned step data
    uint32_t finalOutput = clearedPort | shiftedData; 
    LATB = finalOutput;
}

/**
 * Initialize hardware pins and start initial step timer
 */
bool InitStepperService(uint8_t Priority) {
    ES_Event_t ThisEvent;
    MyPriority = Priority;

    // Set RB10-13 as digital outputs
    TRISBCLR = MOTOR_MASK; 
    ANSELBCLR = MOTOR_MASK;
    //Set RB9 as digital input: direction change switch
    TRISBSET = (1 << 8);
    ANSELBCLR = (1 << 8);

    // Set initial stepper motor pin state
    SetStepperPins();

    // Start initial step timer (1.2: 100ms, 10 steps per second)
    ES_Timer_InitTimer(STEP_TIMER, 50); 

    // Post the initial transition event
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * Run stepper motor by running stepper service and handling events
 */
ES_Event_t RunStepperService(ES_Event_t ThisEvent) {

    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    switch (ThisEvent.EventType) {
        // Handle step timer timeout event
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == STEP_TIMER) {
                if (stepCount < MAX_STEPS) {
                    stepCount++;
                
                // read RB8, 1 << 8 corresponds to Pin 17 (RB8)
                if ((PORTB & (1 << 8)) != 0) {
                    // If RB8 is high, set to forward, currentStepIndex = currentStepIndex + 1
                    currentDirection = 1;
                    currentStepIndex++;
                } else {
                    // If RB8 is low (connected to GND), set to backward, currentStepIndex = currentStepIndex - 1
                    currentDirection = -1;
                    currentStepIndex--;
                }
                    
                // Wrap index around 0-3 using AND 0x03 0100(4)&0011 = 0000. Half step: 0x07
                currentStepIndex &= 0x07;
                
                // Set stepper motor pins to new step pattern
                SetStepperPins();
                
                uint32_t AdInterval = GetIntervalfromADC();

                // If interval is non-zero, restart timer
                if (AdInterval > 0) {
                    ES_Timer_InitTimer(STEP_TIMER, AdInterval);
                }
                }
                else{
                    // If stepCount reaches MAX_STEPS, stop the motor
                    ES_Timer_InitTimer(STEP_TIMER, 0);
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

bool PostStepperService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}
