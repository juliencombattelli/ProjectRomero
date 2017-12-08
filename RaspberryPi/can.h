/*
 * can.h
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#ifndef CAN_H_
#define CAN_H_


//STM32 -> Raspy
//data structure for ultrasound sensors
//type must be define
typedef struct data_ultrasound
{
    union {
        uint8_t num_ultrasound[6] ;
        unsigned char bytes_ultrasound[6] ;
    } ultrasound ;
    
} data_ultrasound  ;

//data structure for potentiometer sensor
typedef struct data_potentiometer
{
    union {
        uint8_t num_potentiometer;
        unsigned char  bytes_potentiometer;
    }potentiometer;
    
} data_potentiometer ;

//data structure for odometer sensor
typedef struct data_odometer
{
    union {
        uint8_t num_odometer;
        unsigned char  bytes_odometer;
    }odometer;
    
} data_odometer ;

//data structure for battery
typedef struct data_battery
{
    
    union {
        uint8_t num_battery ;
        unsigned char bytes_battery ;
    } battery ;
} data_battery ;


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


typedef struct obstacle
{
    int detected ;  //1 if obstacle detected else 0
    int mobile ;    //1 if mobile ; 0 if static
    int dist ;      //distance from the detected object
} obstacle ;


//speed and direction command must be define
void create_speed_command_frame(data_speed_command data, char * trame);
void create_direction_command_frame(data_direction_command data, char * trame);

//init the send
int initSend();

/*
 * TODO: Add value from bluetooth in parameters
 * Call the function create_speed_command_frame (learn more about structures  : canRaspi.h)
 */
//send data speed command
void SendData_Speed(uint16_t speed);

/*
 * TODO: Add value from bluetooth in parameters
 * Call the function create_direction_command_frame (learn more about structures  : canRaspi.h)
 */
//send data direction command
void SendData_Direction(uint16_t direction);

//receive and print data depending on the id of the frame
void * ReceiveData();

//analyse the obstacle detection
void Obstacle_Detection(data_ultrasound data, obstacle ** obst) ;

#endif /* CAN_H_ */
