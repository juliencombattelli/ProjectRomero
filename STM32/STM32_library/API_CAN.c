/*----------------------------------------------------------------------------
 * Name:    API_CAN.c
 * Purpose: API for CAN functions
 * Note(s): 
 *----------------------------------------------------------------------------
*/

#include "API_CAN.h"
#include <string.h>


//create the ultrasound data frame
void create_ultrasound_frame(data_ultrasound data, char * frame) {
	frame[0] = data.bytes_ultrasound[0] ;
	frame[1] = data.bytes_ultrasound[1] ; ;
	frame[2] = data.bytes_ultrasound[2] ; ;
	frame[3] = data.bytes_ultrasound[3] ; ;
	frame[4] = data.bytes_ultrasound[4] ; ;
	frame[5] = data.bytes_ultrasound[5] ; ;
	frame[6] =  '\0' ;
	frame[7] = '\0' ;
}	

//create the potentiometer data frame
void create_potentiometer_frame(data_potentiometer data, char * frame) {
	frame[0] = data.potentiometer.bytes_potentiometer[0] ; 
	frame[1] = data.potentiometer.bytes_potentiometer[1] ; 
	frame[2] = '\0' ;
	frame[3] = '\0' ;
	frame[4] = '\0' ;
	frame[5] = '\0' ;
	frame[6] = '\0' ;
	frame[7] = '\0' ;
}

//create the potentiometer data frame
void create_odometer_frame(data_odometer data, char * frame) {
	frame[0] = data.left_odometer.bytes_left_odometer[0] ; 
	frame[1] = data.left_odometer.bytes_left_odometer[1] ; 
	frame[2] = data.right_odometer.bytes_right_odometer[0] ;
	frame[3] = data.right_odometer.bytes_right_odometer[1] ;
	frame[4] = '\0' ;
	frame[5] = '\0' ;
	frame[6] = '\0' ;
	frame[7] = '\0' ;
}


