/**
 * @file valve.c
 * @brief Implements functions for operating valve
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */

#include "valve.h"
#include "misc.h"

#define N_VALVES      4  // 2 potentiometer; 2 reed switch
#define N_VALVE_ITER  30 // number of iterations for dead reckoning to fully open/close

uint8  potentiometer_flag = 1u;

int test_valve(){
	int p_count = 0;
	
	for (p_count = 0; p_count < 2; p_count++){
		blink_LED(5u);
		Valve_OUT_Write(1u);
		CyDelay(1000u);
		Valve_OUT_Write(0u);
		
		Valve_IN_Write(1u);
		CyDelay(1000u);
		Valve_IN_Write(0u);
        
		Valve_2_OUT_Write(1u);
		CyDelay(1000u);
		Valve_2_OUT_Write(0u);
		
		Valve_2_IN_Write(1u);
		CyDelay(1000u);
		Valve_2_IN_Write(0u);
                
	}
    return 1;
}

int test_valves(){
	int p_count = 0, n_valve = 0;
	
	for (p_count = 0; p_count < 1; p_count++){
        
        blink_LED(5u);
        
		for (n_valve = 1; n_valve <= 2; n_valve++) {
    		
            valve_OUT(n_valve, 1u);
    		CyDelay(1000u);
    		valve_OUT(n_valve, 0u);

    		valve_IN(n_valve, 1u);
    		CyDelay(1000u);
    		valve_IN(n_valve, 0u);
        }
                
	}
    
    //float32 tmp_pos = 0.0;
    //move_valves(0,&tmp_pos,1,0);
    return 1;
}

int valve_IN(int valve_flag, int value) {
    
    // For now, assume we only have 2 valves
    // - if valve_flag = 1, 3, 5, 7, 9... (odd), start/stop moving Valve 1 IN
    if (valve_flag % 2 == 1) {
        Valve_IN_Write(value);
    }
    // - if valve_flag = 0, 2, 4, 6, 8... (even), start/stop moving Valve 2 IN
    if (valve_flag % 2 == 0) {
        Valve_2_IN_Write(value);
    }
    
    return 1;
}

int valve_OUT(int valve_flag, int value) {
    
    // For now, assume we only have 2 valves
    // - if valve_flag = 1, 3, 5, 7, 9... (odd), start/stop moving Valve 1 OUT
    if (valve_flag % 2 == 1) {
        Valve_OUT_Write(value);
    }
    // - if valve_flag = 0, 2, 4, 6, 8... (odd), start/stop moving Valve 2 OUT
    if (valve_flag % 2 == 0) {
        Valve_2_OUT_Write(value);
    }
    return 1;
}

int start_Valves_POS(int valve_flag, uint8 potentiometer_flag) {
    
    if (potentiometer_flag == 1u) {
        /* Start the Analog MUX */
        start_AMUX(); 
        
        /* Set up the multiplexer and select valve_flag input (zero-indexed) 
           Need offset for v_bat, which is attached to the 0th input of the MUX
        */    	
        AMux_Select((uint8) valve_flag+1); //AMux_Select(1u);
        
    }   
    // else assume we're working with a reed switch actuator (digital pulse input)
    // else (potentiometer_flag == 0u)
    else {
        
        PulseCounter_Start();
        
        /* Start the MUX */
        start_Pulse_MUX(); 
        
        // Reset the pulse Counter
        PulseCounter_WriteCounter(0u);
        
        /* Set MUX to read from input for valve_flag 
           Need offset for autosampler, which is attached to the 0th input of the MUX
        */
        Pulse_MUX_Controller_Write((uint8) valve_flag);         
    }
    
    /* flip on the ADC pin */
    if (valve_flag % 2 == 1) {
        Valve_POS_EN_Write(1u);
    }
    else { // assume we have 2 valves only
        Valve_2_POS_EN_Write(1u);
    }
    // Delay to let the pin settle
    CyDelay(100u);
    
    return 1;
}

int stop_Valves_POS(int valve_flag, uint8 potentiometer_flag) {
    
    /* flip off the ADC pin */
	if (valve_flag % 2 == 1) {
        Valve_POS_EN_Write(0u);
    }
    else { // assume we have 2 valves only
        Valve_2_POS_EN_Write(0u);
    } 
    
    if (potentiometer_flag == 1u) {
        stop_AMUX();        
    }
    // else assume we're working with a reed switch actuator (digital pulse input)
    // else (potentiometer_flag == 0u)
    else {
        stop_Pulse_MUX();         
    }
    return 1;
}

float32 read_Valves_POS(int valve_flag, uint8 potentiometer_flag) {
	
	uint16 pulses;
    uint32 reading;
	float32 valve_pos, v_bat;
    

    if (valve_flag) {
        // Do nothing for now
    }
	
    if (potentiometer_flag == 1u) {
       
    	/* Read battery voltage*/
        v_bat = read_vbat();
        
        /* Read the potentiometer voltage in bits */
        reading = VBAT_ADC_Read32();
    	
    	/* Calculate ratio of the potentiometer voltage to the battery voltage 
    	   This returns the percentage that the valve is open
    	*/
    	valve_pos = VBAT_ADC_CountsTo_Volts(reading) / v_bat;    	   	 
    	
    }
    // else assume we're working with a reed switch actuator
    else { // (potentiometer_flag == 0u)
        
        /* Get the pulses and store in valve_pos */
        pulses = PulseCounter_ReadCounter();
        valve_pos = pulses;
      
    }   
    
	return valve_pos;
}


int move_valves(int valve_ref, float32* valve_pos, int valve_flag, uint8 potentiometer_flag){
	int iter;
	int valve_out = 0;
	float32 d_valve;
	
	start_Valves_POS(valve_flag, potentiometer_flag);
    
    if (valve_ref == 0){
        
		// Push the actuator out and fully close the valve 
        for(iter = 0; iter < N_VALVE_ITER; iter++) {
											
			// valve_pos maxes out at 0.88 but we'll leave
			//   this at 0.90 to ensure the valve fully closes
			//
			//if(valve_pos > 0.90) {									
			//	break;
			//}
								
			valve_OUT(valve_flag, 1u);
			CyDelay(1000u);
			valve_OUT(valve_flag, 0u);
            
            *valve_pos = read_Valves_POS(valve_flag, potentiometer_flag);
		}
        
        stop_Valves_POS(valve_flag, potentiometer_flag);
		
        // Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
	}
	else if (valve_ref == 100){
		
        // Push the actuator out and fully close the valve 
		for(iter = 0; iter < N_VALVE_ITER; iter++) {
											
			// say limit for open valve is 0.05
			//if(valve_pos > 0.05) {									
			//	break;
			//}
								
			valve_IN(valve_flag, 1u);
			CyDelay(1000u);
			valve_IN(valve_flag, 0u);
            
            *valve_pos = read_Valves_POS(valve_flag, potentiometer_flag);
		}        
        
        stop_Valves_POS(valve_flag, potentiometer_flag);
        
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
	}
	else if((valve_ref > 0 && valve_ref < 100) && potentiometer_flag){	
														
	    // Derivative controller
	    // move the actuator within 5 percent of the desired pos
		for(iter = 0; iter < 20; iter++){
			*valve_pos = 100 * read_Valves_POS(valve_flag, potentiometer_flag);
			d_valve = valve_ref - *valve_pos;
								
			// pull the actuator in if desired pos > current pos
			// to open up the valve
			// (e.g. desired is 95% open and valve is currently 50% open)
			if(d_valve >= 2.5){
				valve_IN(valve_flag, 1u);
				CyDelay(1000u);
				valve_IN(valve_flag, 0u);	
			}
		    // push the actuator out if desired pos < current pos
			// to close the valve
			// (e.g. desired is 15% open and valve is currently 40% open)
			if (d_valve <= -2.5){
				valve_OUT(valve_flag, 1u);
				CyDelay(1000u); 
				valve_OUT(valve_flag, 0u);
			}
								
			// reached objective of getting within 1 percent
			if (d_valve < 2.5 && d_valve > -2.5){
				break;
			}
		}			
        
        stop_Valves_POS(valve_flag, potentiometer_flag);
        
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;						
	}
    
    stop_Valves_POS(valve_flag, potentiometer_flag);
    
    return valve_out;
}


uint8 zip_valves(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size, int valve_flag){

    uint8 nvars = 2, k = 0, which_valve = 0;
    float32 *valve_pos;
    
    // Ensure we don't access nonexistent array index
    // Count the number of actuators flagged
    for (k = 0; k < N_VALVES; k++) {
        if (0x00000001 & valve_flag >> k) {
            
            /* Redundant
            if (*valve_trigger >= 0) { 
                // add one var for the acknowledgement; 
                // another for the potentiometer/reed switch
                nvars = 2; 
            }
            */

            // Return if max_size is exceeded
            if (*array_ix + nvars >= max_size) {
                return *array_ix;
            }            
            
            // For potentiometer valves
            if (k < 2) {
                potentiometer_flag = 1u;
            }
            // For reed switch actuators
            else {
                potentiometer_flag = 0u;
            }
            
            // Move one valve at a time, all to the same desired position (valve_trigger)
            // and then update the labels and readings
            // (To move two valves to different positions, two commands must be sent to the server,
            // where the flag for only one valve is set at a time.)
            // - Move and Update valve 1
            if (k % 2 == 0) {
                if (*valve_trigger >= 0 && *valve_trigger <= 100) {
                    which_valve = 1u;
            	    move_valves(*valve_trigger, valve_pos, which_valve, potentiometer_flag);
                    // in the future, instead of "which_valve", consider parsing valve_flag in move_valves()
                    
                    // in the future, consider char *valve_triggers[] = {"valve_trigger", "valve_2_trigger"};
                    //                         char *valve_poses[]    = {"valve_pos", "valve_2_pos"};  
                    labels[*array_ix] = "valve_trigger";
                    readings[*array_ix] = -1 * (k+1); // valve_trigger must be less than 0 to avoid triggering the valves
                    (*array_ix) += 1;
                }
            
                labels[*array_ix] = "valve_pos";
                readings[*array_ix] = *valve_pos;
                (*array_ix) += 1;
            }
            // - Move and Update valve 2
            else { // (k % 2 == 0) 
                if (*valve_trigger >= 0 && *valve_trigger <= 100) {
                    which_valve = 2u;
            	    move_valves(*valve_trigger, valve_pos, which_valve, potentiometer_flag);
                    // in the future, instead of "which_valve", consider parsing valve_flag in move_valves()
                    
                    labels[*array_ix] = "valve_2_trigger";
                    readings[*array_ix] = -1 * (k+1); // valve_trigger must be less than 0 to avoid triggering the valves
                    (*array_ix) += 1;
                } 
                
                labels[*array_ix] = "valve_2_pos";
                readings[*array_ix] = *valve_pos;
                (*array_ix) += 1;                
            }
        }
    }
    
    
    return 0u;
}

float32 read_Valve_POS() {
	
	uint32 reading;
	float32 valve_pos, v_bat;
	
    /* Start up the Analog MUX */
    start_AMUX();
	
    // Get the battery voltage 
	v_bat = read_vbat();
    
    /*
    // Start the Analog MUX and select second input 
    AMux_Start();
    AMux_Select(1u);
	
	// Restore User configuration & start the ADC 
	VBAT_ADC_Wakeup();
	VBAT_ADC_Start(); 
    */
    
	/* flip on the ADC pin */
	Valve_POS_EN_Write(1u);
	CyDelay(100u);
    
	/* Set up the multiplexer */
	AMux_Select(1u);
	
	/* Read the potentiometer voltage in bits */
    reading = VBAT_ADC_Read32();
	
	/* Calculate ratio of the potentiometer voltage to the battery voltage 
	   This returns the percentage that the valve is open
	*/
	valve_pos = VBAT_ADC_CountsTo_Volts(reading) / v_bat;
	
	/* Stop the conversion & save user configuration */
	VBAT_ADC_Sleep();
	
	/*
	// flip off the ADC pin 
	Valve_POS_EN_Write(0u);	
	
    // Turn of the Analog MUX 
    AMux_Stop();
    */
    
    // Stop the Analog MUX
    stop_AMUX();
    
	return valve_pos;
}

int move_valve(int valve){
	int iter;
	int valve_out = 0;
	float32 valve_pos, d_valve;
	
	if (valve == 0){
		// Push the actuator out and fully close the valve 
		for(iter = 0; iter < 24; iter++) {
			valve_pos = read_Valve_POS();
								
			// valve_pos maxes out at 0.88 but we'll leave
			//   this at 0.90 to ensure the valve fully closes
			//
			//if(valve_pos > 0.90) {									
			//	break;
			//}
								
			Valve_OUT_Write(1u);
			CyDelay(1000u);
			Valve_OUT_Write(0u);
			}
														
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
		}
	else if (valve == 100){
		// Push the actuator out and fully close the valve 
		for(iter = 0; iter < 23; iter++) {
			valve_pos = read_Valve_POS();
								
			// say limit for open valve is 0.05
			//if(valve_pos > 0.05) {									
			//	break;
			//}
								
			Valve_IN_Write(1u);
			CyDelay(1000u);
			Valve_IN_Write(0u);
		}
														
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;
	}
	else if((valve > 0 && valve < 100) && potentiometer_flag){	
														
	    // Derivative controller
	    // move the actuator within 5 percent of the desired pos
		for(iter = 0; iter < 20; iter++){
			valve_pos = 100 * read_Valve_POS();
			d_valve = valve - valve_pos;
								
			// pull the actuator in if desired pos > current pos
			// to open up the valve
			// (e.g. desired is 95% open and valve is currently 50% open)
			if(d_valve >= 2.5){
				Valve_IN_Write(1u);
				CyDelay(1000u);
				Valve_IN_Write(0u);	
			}
		    // push the actuator out if desired pos < current pos
			// to close the valve
			// (e.g. desired is 15% open and valve is currently 40% open)
			if (d_valve <= -2.5){
				Valve_OUT_Write(1u);
				CyDelay(1000u); 
				Valve_OUT_Write(0u);
			}
								
			// reached objective of getting within 1 percent
			if (d_valve < 2.5 && d_valve > -2.5){
				break;
			}
		}							
		// Acknowledge the command
		valve_out = -1*iter;
		return valve_out;						
		}
    return valve_out;
}

uint8 zip_valve(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size){
    uint8 nvars = 0;
    float32 valve_pos;
    
    // Ensure we don't access nonexistent array index    
    if (*valve_trigger >= 0) { 
        nvars += 1; 
    }
    if (potentiometer_flag == 1u) {
        nvars += 1;
    }
    
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
    
    
    // Ellsworth does not have a potentiometer installed
    // Simply flip the pins for now and come back to implement
    // a pulse counter
            
    if (*valve_trigger >= 0 && *valve_trigger <= 100) {
	    *valve_trigger = move_valve(*valve_trigger);
        
        labels[*array_ix] = "valve_trigger";
        readings[*array_ix] = -1;
        (*array_ix) += 1;
    }
    
    if (potentiometer_flag == 1u) {
	    valve_pos = 100. * read_Valve_POS();
        
        labels[*array_ix] = "valve_pos";
        readings[*array_ix] = valve_pos;
        (*array_ix) += 1;
    }
    
    //(*array_ix) += nvars;
    
    /*
    // If zero, open the valve completely
    // IMPORTANT: If there is a "null" entry,
    //            intparse_influxdb returns 0
    //            Make sure default is to open the valve
    if (*valve_trigger == 0) { 
        Valve_OUT_Write(1u);
        CyDelay(20000u);
        Valve_OUT_Write(0u);
    // Else, if 100, close the valve completely
    } else if(*valve_trigger == 100) {
        Valve_IN_Write(1u);
        CyDelay(20000u);
        Valve_IN_Write(0u);
    } else {
        // For now, do nothing for the other cases
    }
    
    // Acknowledge the trigger by updating it to -1
    // -1, and negative values are reserved for actuator response
    labels[*array_ix] = "valve_trigger";
    readings[*array_ix] = -1;
    (*array_ix) += 1;*/
    
    return *array_ix;
}

uint8 zip_valve_2(char *labels[], float readings[], uint8 *array_ix, int *valve_2_trigger, uint8 max_size){
    // Ensure we don't access nonexistent array index
    uint8 nvars = 1;
    if(*array_ix + nvars >= max_size){
        return *array_ix;
    }
        // If zero, open the valve completely
        // IMPORTANT: If there is a "null" entry,
        //            intparse_influxdb returns 0
        //            Make sure default is to open the valve
        if (*valve_2_trigger == 0) { 
            Valve_2_OUT_Write(1u);
            CyDelay(20000u);
            Valve_2_OUT_Write(0u);
        // Else, if 100, close the valve completely
        } else if(*valve_2_trigger == 100) {
            Valve_2_IN_Write(1u);
            CyDelay(20000u);
            Valve_2_IN_Write(0u);
        } else {
            // For now, do nothing for the other cases
        }
        
        // Acknowledge the trigger by updating it to -1
        // -1, and negative values are reserved for actuator response
        labels[*array_ix] = "valve_2_trigger";
        readings[*array_ix] = -1;
        (*array_ix) += 1;
        return *array_ix;
}

/* [] END OF FILE */
