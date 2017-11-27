#ifndef API_CAN_H
#define API_CAN_H

#include "CAN.h"
#include "stm32f10x.h"       /* STM32F10x Definitions    */


/*----------------------------------------------------------------------------------------------------------------------------------------------
// TODO: 

	- Comment the header 
	- Define the ultrasound data type 
	
------------------------------------------------------------------------------------------------------------------------------------------------*/

//STM32 -> Raspy
//data structure for ultrasound sensors
//type must be define
typedef struct data_ultrasound
{		
	
	unsigned char bytes_ultrasound[6] ; 
	
} data_ultrasound  ;

//data structure for potentiometer sensor
typedef struct data_potentiometer
{
	union {
		short num_potentiometer;
		unsigned char  bytes_potentiometer[2];
	}potentiometer;
	
} data_potentiometer ; 

//data structure for odometer sensor
typedef struct data_odometer 
{
		union {
			short num_left_odometer;
			unsigned char  bytes_left_odometer[2];
		}left_odometer;
		
		union {
			short num_right_odometer;
			unsigned char  bytes_right_odometer[2];
		}right_odometer;
		
} data_odometer ; 


//Raspy -> STM32
//data structure for speed command
typedef struct data_speed_command 
{
	union {
		short num_speed_command;
		unsigned char  bytes_speed_command[2];
	} speed_command;	
	
} data_speed_command ; 
//data structure for direction command
typedef struct data_direction_command 
{
	union {
		short num_direction_command;
		unsigned char  bytes_direction_command[2];
	} direction_command;
	
} data_direction_command ; 


//ultrasound data type must be define 
void create_ultrasound_frame(data_ultrasound data, char * trame) ; 
void create_potentiometer_frame(data_potentiometer data, char * trame) ; 
void create_odometer_frame(data_odometer data, char * trame) ;  

#endif


