#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AdInputService.h"
#include "PIC32_AD_Lib.h"
#include "dbprintf.h"
#include <xc.h>

static uint8_t MyPriority;
static uint8_t MIN_INTERVAL = 5;
// Full Step: 3ms
// Wave Drive: 5ms
// Half Drive: 5ms
// PWM: 13ms

/**
 * Called by external services, reads latest potentiometer value
 */
uint16_t GetIntervalfromADC(void) {
    // Use library function to use AN4 (RB2) for auto-scan mode
    // BIT4HI corresponds to AN4 
    if (ADC_ConfigAutoScan(BIT4HI)){
        uint32_t ADCResults[1]; // Library requires using array to receive data
        uint32_t interval;

        // Read auto-scan results
        ADC_MultiRead(ADCResults);
        uint32_t RawADC = ADCResults[0];
        DB_printf("ADCResults:%u \n", ADCResults[0]);
        if (RawADC>1023) RawADC=1023;
        DB_printf("RawADC:%u \n", RawADC);

        // Linearly map 10-bit ADC value (0-1023) to 1ms - 250ms range, 920:3ms
        if (RawADC<=920)
        {
            interval = 100 - ((100-MIN_INTERVAL)*RawADC/920) ;
        }
        else
        {
            uint32_t diffADC = RawADC-920;
            interval = MIN_INTERVAL - diffADC/103;
        }

        return interval;
    }
}
