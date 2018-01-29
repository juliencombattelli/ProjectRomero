/*----------------------------------------------------------------------------
 * Name:    main.c 
 * Authors: Guillaume De Brito & Corentin Egreteau
 * Date: 01/2018
 *----------------------------------------------------------------------------
*/

#include <stdint.h>
#include <stm32f10x.h>
#include <stdlib.h>

#include "system_time.h"
#include "manager.h"
#include "motors.h"
#include "front_motor.h"
#include "direction.h"
#include "speed_sensors.h"
#include "battery.h"
#include <stdio.h>
#include <string.h>

#include "us_sensors.h"

#include "stm32f10x.h"                            /* STM32F10x Definitions    */
#include "CAN.h"                                  /* STM32 CAN adaption layer */
#include "Timer_1234.h"							  /* Timer driver */
#include "it.h"									  /* IT driver */
#include "API_CAN.h"


 /* ---------------------------------------
  * Variables containing the data to send in a CAN frame
  * ---------------------------------------*/
 data_ultrasound VAL_ULTRA ; 
 data_odometer VAL_ODOMETER ;
 data_potentiometer VAL_POTEN ;
 data_battery VAL_BAT ;

/*-------------------------------------------
------------ GLOBAL VARIABLES -------------------
--------------------------------------------*/


uint8_t vit;  						// Global car speed variable useful for the debug mode on Keil
char trame[8]; 						// frame of 8 bytes send on the CAN bus
char SpeedRx[1];    				// Take the value of the speed received on the CAN
char DirRx[1];      				// Take the value of the direction received on the CAN
unsigned int periodic_modulo = 0;   // Global used to determine the time to send periodic CAN frame 
unsigned int nb_rcv = 0;			// Global used to count the number of commands received 
unsigned int detect_dist = 200		// Global used to define the obstacle detection distance in cm 

volatile uint32_t msTicks;          // Counts 1ms timeTicks    

/* SR => SideRight, SL => SideLeft, FSR => FrontSideRight
   FSL => FrontSideLeft, FR => FrontRight, FL => FrontLeft */ 
float SR, SL, FSR, FR, FL, FSL; 	// Current distance detected for each ultrasound sensor
																		
float Prev_SR, Prev_SL, Prev_FSR, Prev_FR, Prev_FL, Prev_FSL; // Previous distance detected for each ultrasound sensor	

 
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;                        //increment counter necessary in Delay() 
}

/*----------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *----------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}


/*----------------------------------------------------------------------------
  initialize CAN interface
 *----------------------------------------------------------------------------*/
void can_Init (void) {
	CAN_setup ();                                   	/* setup CAN Controller     */  
	CAN_wrFilter (CAN_ID_CMD_DIR, STANDARD_FORMAT);     /* Enable reception of msgs for the direction*/
	CAN_wrFilter (CAN_ID_CMD_SPEED, STANDARD_FORMAT);   /* Enable reception of msgs for the speed */	
	CAN_start ();                                  	    /* start CAN Controller   */
	CAN_TxMsg.id = CAN_ID_ULTRASOUND;
	CAN_waitReady ();                                   /* wait til tx mbx is empty */
}

/*----------------------------------------------------------------------------
  send periodically CAN frames
*----------------------------------------------------------------------------*/
void canPeriodic (void) {
	int i; 						// loop indicator
	uint8_t angle_direction; 	// angle of the front wheels
	uint8_t car_dist;			// distance traveled by the car during a period 
	uint8_t bat;				// value in percentage of the battery	
	int dist;					// distance of an obstacle detected by the ultrasounds	
	
	uint8_t speed = (uint8_t)((SpeedSensor_get(SPEED_CM_S,SENSOR_L)+SpeedSensor_get(SPEED_CM_S,SENSOR_R))/2.0); // current speed of the car in CM/s
	
	/*------------------------------------------
	 * Send an ultrasound frame every 100ms
	 *-----------------------------------------*/
	//Take the obstacle distance for each sensor
	FSL = US_CalcDistance(0);
	FL  = US_CalcDistance(1);
	FSR = US_CalcDistance(2);
	SL  = US_CalcDistance(3);
	FR  = US_CalcDistance(4);
	SR  = US_CalcDistance(5);
	
	car_dist = speed/10 + 2; // Distance in cm traveled by the car in 100 ms (+2 : margin used to not detect a mobile obstacle when the car is immobile du to the ultrasound fluctuation)
		
	// 1 ultrasound <=> 1 byte => ultrasound[7:0] = dist[7:2] mobile[1] detected[0]
	// dist is the distance in cm divided by 2 of an obstacle detected on 6 bits 
	// mobile is equal to 1 if the obstacle is moving otherwise equals to 0
	// detected equals 1 if an obstacle is detected at less than 2 meters
		
	if (SL < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[0] = 1; // detected[0] = 1
		dist = (((int)(SL)) >> 1) << 2; // ">> 1" is the division of the distance by 2 and "<< 2" is a left shift to put this data in dist[7:2]
		if (dist > 252) { // maximum value on 6 bits is 64 => 64 << 2 = 252
			dist = 252;	  // the maximal distance send in the frame is 1,28 m (64*2)
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[0] += dist; // dist[7:2] = dist
		
		// Check if the diff between two successive measures of the same ultrasound is superior to the distance traveled by the car during this period
		// or if during the prior period one of the neighbouring ultrasound detected an obstacle and during the current period it do not detect it anymore
		if (abs((int)(Prev_SL-SL)) > car_dist || (Prev_FSL < detect_dist && FSL > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[0] += 2; // mobile[1] = 1
		}
	}
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[0] = 0;}
	
	if (FSL < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[1] = 1;
		dist = (((int)(FSL)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}		
		VAL_ULTRA.ultrasound.bytes_ultrasound[1] += dist;
		if (abs((int)(Prev_FSL-FSL)) > car_dist || (Prev_SL < detect_dist && SL > detect_dist) 
			|| (Prev_FL < detect_dist && FL > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[1] += 2; 
		}
	}	
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[1] = 0;}	
	
	if (FL < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[2] = 1; 
		dist = (((int)(FL)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[2] += dist;
		if (abs((int)(Prev_FL-FL)) > car_dist || (Prev_FSL < detect_dist && FSL > detect_dist) 
			|| (Prev_FR < detect_dist && FR > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[2] += 2; 
		}
	}		
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[2] = 0;}
	
	if (FR < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[3] = 1; 
		dist = (((int)(FR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[3] += dist;
		if (abs((int)(Prev_FR-FR)) > car_dist || (Prev_FL < detect_dist && FL > detect_dist)
			|| (Prev_FSR < detect_dist && FSR > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[3] += 2; 
		}
	}
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[3] = 0;}
	
	if (FSR < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[4] = 1;
		dist = (((int)(FSR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}		
		VAL_ULTRA.ultrasound.bytes_ultrasound[4] += dist;
		if (abs((int)(Prev_FSR-FSR)) > car_dist || (Prev_FR < detect_dist && FR > detect_dist)
			|| (Prev_SR < detect_dist && SR > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[4] += 2; 
		}
	}		
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[4] = 0;}
	
	if (SR < detect_dist) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[5] = 1; 
		dist = (((int)(SR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[5] += dist;
		if (abs((int)(Prev_SR-SR)) > car_dist || (Prev_FSR < detect_dist && FSR > detect_dist)){
			VAL_ULTRA.ultrasound.bytes_ultrasound[5] += 2; 
		}		
	}	
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[5] = 0;}
	
	Prev_FSL = FSL;
	Prev_FL  = FL;
	Prev_FSR = FSR;
	Prev_SL  = SL;
	Prev_FR  = FR;
	Prev_SR  = SR;	
	
	CAN_TxMsg.id = CAN_ID_ULTRASOUND;                /* initialize msg to send   */  
    for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
    CAN_TxMsg.len = 8;
    CAN_TxMsg.format = STANDARD_FORMAT;
    CAN_TxMsg.type = DATA_FRAME;	
	
	create_ultrasound_frame(VAL_ULTRA, trame);
	CAN_waitReady ();
 	CAN_TxRdy0 = 0;    													/* CAN HW unready to transmit message mailbox 0*/	
	memcpy(CAN_TxMsg.data, trame, sizeof(trame));
	CAN_wrMsg (&CAN_TxMsg);                     /* transmit message         */
	
	switch (periodic_modulo) {
		/*------------------------------------------
	 * Send a direction frame every 600ms
	 *-----------------------------------------*/
		case 0:
			CAN_TxMsg.id = CAN_ID_DIR;                /* initialize msg to send   */  
			for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
			CAN_TxMsg.len = 8;
			CAN_TxMsg.format = STANDARD_FORMAT;
			CAN_TxMsg.type = DATA_FRAME;	
			//direction detection
			angle_direction = Direction_get() ;				

			if(angle_direction<=110) {
				VAL_POTEN.potentiometer.num_potentiometer = 2;
			} 
			else if (angle_direction<= 122) {
				VAL_POTEN.potentiometer.num_potentiometer = 4;
			}
			else if(angle_direction<=132) {
				VAL_POTEN.potentiometer.num_potentiometer = 0;
			} 
			else if(angle_direction<=146) {
				VAL_POTEN.potentiometer.num_potentiometer = 3;
			} 
			else {
				VAL_POTEN.potentiometer.num_potentiometer = 1;
			}

			create_potentiometer_frame(VAL_POTEN, trame) ; 
			CAN_waitReady ();
			CAN_TxRdy1 = 0;    													/* CAN HW unready to transmit message mailbox 1*/
					
			memcpy(CAN_TxMsg.data, trame, sizeof(trame));
			CAN_wrMsg (&CAN_TxMsg);                     /* transmit message         */
			periodic_modulo++;
			break;
		
		/*------------------------------------------
	 * Send an speed frame every 600ms
	 *-----------------------------------------*/
		case 1:
			CAN_TxMsg.id = CAN_ID_SPEED;                /* initialize msg to send   */  
			for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
			CAN_TxMsg.len = 8;
			CAN_TxMsg.format = STANDARD_FORMAT;
			CAN_TxMsg.type = DATA_FRAME;	
		
			VAL_ODOMETER.odometer.num_odometer = speed;		
			create_odometer_frame(VAL_ODOMETER, trame);  
			CAN_waitReady ();
			CAN_TxRdy2 = 0;    													/* CAN HW unready to transmit message mailbox 2*/
					
			memcpy(CAN_TxMsg.data, trame, sizeof(trame));
			CAN_wrMsg (&CAN_TxMsg);
			periodic_modulo++;
			break;
		/*------------------------------------------
	 * Send a battery frame every 600ms
	 *-----------------------------------------*/
		case 2:
			//TODO: CAN part 
			CAN_TxMsg.id = CAN_ID_BATTERY ; 						/* initialize message to send   */
			for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
			CAN_TxMsg.len = 8;
			CAN_TxMsg.format = STANDARD_FORMAT;
			CAN_TxMsg.type = DATA_FRAME;		
		
			bat = Battery_get();
			if(bat <= 15) {
				VAL_BAT.battery.num_battery = 3;				
			} else if(bat <= 30) {
				VAL_BAT.battery.num_battery = 2;	
			} else if(bat <= 60) {
				VAL_BAT.battery.num_battery = 1;	
			} else if(bat <= 100) {
				VAL_BAT.battery.num_battery = 0;	
			}
			
			create_battery_frame(VAL_BAT,trame) ; 
			CAN_waitReady() ; 
			CAN_TxRdy2 = 0;    													/* CAN HW unready to transmit message mailbox 2*/
			
			memcpy(CAN_TxMsg.data, trame, sizeof(trame));
			CAN_wrMsg (&CAN_TxMsg);
		
			periodic_modulo = 0;
			break;
	}		
	
	// stop the car if no command messages received during the last 200 millisecond
	if (nb_rcv == 0){
			SpeedRx[0] = 0;
			DirRx[0] = 0;			
		}	
	nb_rcv = 0;		
}

/*----------------------------------------------------------------------------
  Turns the front wheels at the angle desired
 *----------------------------------------------------------------------------*/
void Turn(uint8_t deg){
	uint8_t angle = Direction_get(); //take current angle of the wheels
	if (deg > Direction_get() + 3){	 // +3 is used to have a tolerance interval and to avoid wheel oscillations
		FrontMotor_turn(LEFT);}
	else if (deg < Direction_get() - 3){
		FrontMotor_turn(RIGHT);}
	else {
		FrontMotor_turn(NONE);}}

		 
void Speed_Cmd(char *cmd){
	if (cmd[0] == 0){ 										//STOP
		Motor_setSpeed(REAR_MOTOR_L, 0); 
		Motor_setSpeed(REAR_MOTOR_R, 0);}	
	else if (cmd[0] == 1) { 							//Default speed		
		Motor_setSpeed(REAR_MOTOR_L, 0.5); 
		Motor_setSpeed(REAR_MOTOR_R, 0.5);}
	else if (cmd[0] == 2) {     					//Turbo speed		
		Motor_setSpeed(REAR_MOTOR_L, 1); 
		Motor_setSpeed(REAR_MOTOR_R, 1);}}	
		
		
void Dir_Cmd(char *cmd, uint8_t angle){
	if (DirRx[0] == 0){				//Position centrale des roues
		if (angle >= 127 || angle <= 137){
				Motor_Enable(FRONT_MOTOR);
				Turn(132);}} 															
	else if (DirRx[0] == 1)	{	//Position à gauche des roues		
		if (angle >= 145 || angle <= 155){
				Motor_Enable(FRONT_MOTOR);
				Turn(150);}} 
	else if (DirRx[0] == 2) { //Position à droite des roues
		if (angle >= 105 || angle <= 115){
				Motor_Enable(FRONT_MOTOR);
				Turn(110);}}}
		
/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
uint8_t angle;

		
int main (void)  {
    SysTick_Config(SystemCoreClock / 1000);         /* SysTick 1 msec IRQ       */

	Manager_Init();
	Motor_QuickInit(REAR_MOTOR_L);
	Motor_QuickInit(REAR_MOTOR_R);
	Motor_Enable(REAR_MOTOR_L);
	Motor_Enable(REAR_MOTOR_R);	
    can_Init ();                                    /* initialize CAN interface */
	
	FSL = US_CalcDistance(0);
	FL  = US_CalcDistance(1);
	FSR = US_CalcDistance(2);
	SL  = US_CalcDistance(3);
	FR  = US_CalcDistance(4);
	SR  = US_CalcDistance(5);
	
	SpeedRx[0] = 0;
	DirRx[0] = 0;
	
    Timer_1234_Init (TIM2, 100000);								  /* set Timer 2 every 200ms */
	Timer_Active_IT(TIM2, 0, canPeriodic);					/* Active Timer2 IT					*/
	
  while (1) {
		//angle = Direction_get() ;					
		vit = (uint8_t)((SpeedSensor_get(SPEED_CM_S,SENSOR_L)+SpeedSensor_get(SPEED_CM_S,SENSOR_R))/2.0) ;
	
		if (CAN_RxRdy) {                              //rx msg on CAN Ctrl 
			nb_rcv ++;
			CAN_RxRdy = 0;
			if (CAN_RxMsg.id == CAN_ID_CMD_SPEED){
				memcpy(SpeedRx, CAN_RxMsg.data, sizeof(SpeedRx));}				
			if (CAN_RxMsg.id == CAN_ID_CMD_DIR){
				memcpy(DirRx, CAN_RxMsg.data, sizeof(DirRx));}
		}	
		
		//Speed_Cmd(SpeedRx);
		
		if (SpeedRx[0] == 0)
		{
			Motor_setSpeed(REAR_MOTOR_L, 0); //STOP
			Motor_setSpeed(REAR_MOTOR_R, 0);	
			
		}
		else if (SpeedRx[0] == 1)
		{
			
			Motor_setSpeed(REAR_MOTOR_L, 0.35); //Default speed
			Motor_setSpeed(REAR_MOTOR_R, 0.35);				
		}
		else if (SpeedRx[0] == 2)
		{
			
			Motor_setSpeed(REAR_MOTOR_L, 0.5); //Turbo speed
			Motor_setSpeed(REAR_MOTOR_R, 0.5);
		} 

		//Dir_Cmd(DirRx, angle);		
		
		if (DirRx[0] == 0) //Position centrale des roues 127
		{
			if (angle <= 125 || angle >= 135){
				//Motor_Enable(FRONT_MOTOR);
				Turn(130);
			}			
		}  
		else if (DirRx[0] == 1) //Position à extrême gauche des roues 155
		{
			if (angle <= 152 || angle >= 158){
				//Motor_Enable(FRONT_MOTOR);
				Turn(155);
			}			
		}
		else if (DirRx[0] == 2) //Position à extrême droite des roues 106
		{
			if (angle <= 102 || angle >= 108){
				//Motor_Enable(FRONT_MOTOR);
				Turn(105);
			}
		}	
		else if (DirRx[0] == 3) //Position à gauche des roues 141
		{
			if (angle <= 136 || angle >= 146){
				//Motor_Enable(FRONT_MOTOR);
				Turn(141);
			}			
		}
		else if (DirRx[0] == 4) //Position à droite des roues 117
		{
			if (angle <= 112 || angle >= 122){
				//Motor_Enable(FRONT_MOTOR);
				Turn(117);
			}
		}	
		

		
  }
}
