/**
 * @file valve.h
 * @brief Declares functions for operating valve
 * @author Brandon Wong
 * @version TODO
 * @date 2017-06-19
 */
#ifndef VALVE_H
#define VALVE_H
#include <project.h>
#include "misc.h"

/**
 * @brief Test the valve by opening and closing it.
 *
 * @return 1
 */
int test_valve();

/**
 * @brief Test the valves by opening and closing it using valve_IN() and valve_OUT().
 *
 * @return 1
 */
int test_valves();

/**
 * @brief replace all Valve_[xx]_IN() functions with a single function that moves the [xx] valve
 *
 * @param valve_flag Flag that indicates to move the [xx]th valve
 * @param value      Value (hi or low) to write to Valve_[xx]_IN()
 */
int valve_IN(int valve_flag, int value);

/**
 * @brief replace all Valve_[xx]_OUT() functions with a single function that moves the [xx] valve
 *
 * @param valve_flag Flag that indicates to move the [xx]th valve
 * @param value      Value (hi or low) to write to Valve_[xx]_OUT()
 */
int valve_OUT(int valve_flag, int value);

/**
 * @brief replace all Valve_[xx]_OUT() functions with a single function that moves the [xx] valve
 *
 * @param valve_flag Flag that indicates to move the [xx]th valve
 * @param value      Value (hi or low) to write to Valve_[xx]_OUT()
 */
int start_Valves_POS(int valve_flag, uint8 potentiometer_flag);

/**
 * @brief replace all Valve_[xx]_OUT() functions with a single function that moves the [xx] valve
 *
 * @param valve_flag Flag that indicates to move the [xx]th valve
 * @param value      Value (hi or low) to write to Valve_[xx]_OUT()
 */
int stop_Valves_POS(int valve_flag, uint8 potentiometer_flag);

/**
 * @brief Read the current position of the valve
 *
 * @return Position of the valve as a percent, with 0 meaning completely open
 * and 100 meaning completely closed.
 */
float32 read_Valves_POS(int valve_flag, uint8 potentiometer_flag);

/**
 * @brief Move valve to the position indicated by @p valve
 *
 * @param valve_ref Reference position to move the valve:
 * - 0: Completely open
 * - 100: Completely closed
 * - 1-99: Partially open
 * @param valve_pos Feedback from potentiometer or reed switch
 * @param valve_flag Flag to indicate which valve to move
 * @param potentiometer_flag Flag if potentiometer is equipped
 *
 * @return Number of iterations needed to close valve. TODO: Unclear.
 */
int move_valves(int valve_ref, float32* valve_pos, int valve_flag, uint8 potentiometer_flag);


/**
 * @brief iterate throw valve_flag and update labels and readings arrays for each enabled valve
 *
 * @param labels Array to store labels corresponding to reach trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 * @param valve_flag Int of bits that flag which valves are active
 * 
 @ return (*array_ix) + number of entries filled
 */
uint8 zip_valves(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size, int valve_flag);


/**
 * @brief Read the current position of the valve
 *
 * @return Position of the valve as a percent, with 0 meaning completely open
 * and 100 meaning completely closed.
 */
float32 read_Valve_POS();

/**
 * @brief Move valve to the position indicated by @p valve
 *
 * @param valve Position to move the valve:
 * - 0: Completely open
 * - 100: Completely closed
 * - 1-99: Partially open
 *
 * @return Number of iterations needed to close valve. TODO: Unclear.
 */
int move_valve(int valve);

/**
 * @brief Moves first valve and inserts current value of valve_trigger into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */
uint8 zip_valve(char *labels[], float readings[], uint8 *array_ix, int *valve_trigger, uint8 max_size);

/**
 * @brief Moves second valve and inserts current value of valve_2_trigger into labels and readings arrays.
 *
 * @param labels Array to store labels corresponding to each trigger result
 * @param readings Array to store trigger results as floating point values
 * @param array_ix Array index to label and readings
 * @param max_size Maximum size of label and reading arrays (number of entries)
 *
 * @return (*array_ix) + number of entries filled
 */

uint8 zip_valve_2(char *labels[], float readings[], uint8 *array_ix, int *valve_2_trigger, uint8 max_size);

#endif
/* [] END OF FILE */
