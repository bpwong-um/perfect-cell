/**
 * @file autosampler.c
 * @brief Implements functions for ISCO autosampler
 * @author Brandon Wong and Matt Bartos
 * @version TODO
 * @date 2017-06-19
 */

#include <device.h>
#include <string.h>
#include "autosampler.h"

// prototype bottle_count interrupt
CY_ISR_PROTO(isr_SampleCounter);

// Declare variables
uint8 SampleCount = 0, SampleCount1 = 0, autosampler_state;;

uint8 autosampler_start(){
    isr_SampleCounter_StartEx(isr_SampleCounter);
    BottleCounter_Start();
    autosampler_state = AUTOSAMPLER_STATE_OFF;
    
    return 1u;
}

uint8 autosampler_stop() {
    return 1u;
}

/*
Start the autosampler.  Then power it on.
*/
uint8 autosampler_power_on() {
    Valve_POS_EN_Write(1u);
    CyDelay(1000u);//give the sampler time to boot
    
    autosampler_state = AUTOSAMPLER_STATE_IDLE;
    
    return 1u;
}

uint8 autosampler_power_off() {
    Valve_POS_EN_Write(0u);
    autosampler_state = AUTOSAMPLER_STATE_OFF;
    
    return 1u;
}

uint8 autosampler_take_sample(uint8 *count){
    uint8 count2 = 0;
    if (*count >= MAX_BOTTLE_COUNT) {        
        return 0;
    }
    
    uint32 i = 0u, delay = 100u, interval;
    
    // Reset the bottle Counter
    BottleCounter_WriteCounter(0u);
    autosampler_state = AUTOSAMPLER_STATE_BUSY;
            
    // Send PULSE_COUNT pulses @ 10Hz to trigger sampler
    for(i=0; i < PULSE_COUNT; i++){
        Pin_Sampler_Trigger_Write(1u);
        CyDelay(100u);
        Pin_Sampler_Trigger_Write(0u);
        CyDelay(100u);
    }
    
    interval =  2u*60*1000/delay;           // Wait Max of 2 Minutes for distributor arm to move
    
    for (i = 0; i < interval ; i++) {  
        CyDelay(delay);
        if (Pin_Sampler_Completed_Sample_Read()!=0) { // Event pin on autosampler is HI
            break;
        }
    }
    
    
    if (Pin_Sampler_Completed_Sample_Read() != 0) {
        
        interval =  4u*60*1000/delay;       // Wait Max of 4 Minutes for pumping to complete
        for (i = 0; i < interval ; i++) { 
            CyDelay(delay);
            if (Pin_Sampler_Completed_Sample_Read()==0) { // Event pin on autosampler is HI
                break;
            }
        }
    }
    
    autosampler_state = AUTOSAMPLER_STATE_IDLE;
    
    //*count = BottleCount_Read();
    count2 = BottleCounter_ReadCounter();
    *count = count2;
    return 1u;
}

uint8 zip_autosampler(char *labels[], float readings[], uint8 *array_ix, int *autosampler_trigger, uint8 *bottle_count, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 2;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    (*autosampler_trigger) = 0u; // Update the value locally
    labels[*array_ix] = "autosampler_trigger"; // Update the database
    readings[*array_ix] = 0;
    (*array_ix)++;
        
    if (*bottle_count < MAX_BOTTLE_COUNT) {
	    labels[*array_ix] = "isco_bottle";
        autosampler_start();
        autosampler_power_on();                                    
        if (autosampler_take_sample(bottle_count) ) {
			readings[*array_ix] = *bottle_count;
		}
		else {
			// Use -1 to flag when a triggered sample failed
            readings[*array_ix] = -1;
		}                        
        autosampler_power_off(); 
        autosampler_stop();
	    (*array_ix)++;
		}
	else {
        //debug_write("bottle_count >= MAX_BOTTLE_COUNT");
		}
    return *array_ix;
}

CY_ISR(isr_SampleCounter){
    SampleCount1++;
}

//*  Steps for Testing the autosampler *//
// - Enter Debug Mode
// - Insert Breakpoint at main.c:154
// - Run 
// - Set next instruction at data.c:185
// - Manually set autosampler_flag to 1
// - Manually set autosampler_trigger to 1
// - Insert Breakpoint at autosampler.c:102
// - Run
// - Set next instuction at autosampler.c:105
// - Insert Breakpoint at autosampler.c:114
// - Run
// - Insert Breakpoint at autosampler.c:52
// - The value *referenced* by [count] should be 0 (or less than MAX_BOTTLE_COUNT)
//   Check by hovering over the variable and cliking the [+] button to expand// 
// - NOTE: Insert Breakpoint at autosampler.c:72 if you want to 
//         skip waiting for the autosampler to respond
//         and then set next instruction at autosampler.c:91 
// - Insert Breakpoint at autosampler.c:96
// - Run
// - Check the value of [count2] at autosampler.c:94 to double check the bottle counter
//   by hovering over the variable
// - Run
// - Relay should click as it opens again, turning off the autosampler and the yellow light if it is enabled
// - Done

/* [] END OF FILE */
