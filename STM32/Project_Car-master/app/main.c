#include <stdint.h>
#include <stm32f10x.h>

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
#include "Timer_1234.h"														/* Timer driver */
#include "it.h"																		/* IT driver */
#include "API_CAN.h"


 /* ---------------------------------------
  * Tests constant
  * ---------------------------------------*/
 data_ultrasound VAL_ULTRA ; 
 data_odometer VAL_ODOMETER ;
 data_potentiometer VAL_POTEN ;
 data_battery VAL_BAT ;

/*-------------------------------------------
---------------------------------------------
--------------------------------------------*/

char text[17];

unsigned int val_Tx = 0, val_Rx = 0;  /* Globals used for display */
char trame[8];
char SpeedRx[1];
char DirRx[1];
unsigned int periodic_modulo = 0;     /* Global used to determine the time to send periodic CAN frame */
unsigned int cnt_failure = 0;         /* Global used to detect a CAN failure */
unsigned int nb_rcv = 0;							/* Global used to count the number of commands received */

volatile uint32_t msTicks;            /* Counts 1ms timeTicks     */

//Current distance detected for each ultrasound sensor
float SR, SL, FSR, FR, FL, FSL; 	
//Previous distance detected for each ultrasound sensor																			
float Prev_SR, Prev_SL, Prev_FSR, Prev_FR, Prev_FL, Prev_FSL; 
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
/*void SysTick_Handler(void) {
  msTicks++;                        //increment counter necessary in Delay() 
}
*/
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

  CAN_setup ();                                   /* setup CAN Controller     */  
	CAN_wrFilter (CAN_ID_CMD_DIR, STANDARD_FORMAT);             /* Enable reception of msgs */
	CAN_wrFilter (CAN_ID_CMD_SPEED, STANDARD_FORMAT);             /* Enable reception of msgs */
	
  CAN_start ();                                   /* start CAN Controller   */
	CAN_TxMsg.id = CAN_ID_ULTRASOUND;
  CAN_waitReady ();                               /* wait til tx mbx is empty */
}

 
void canPeriodic (void) {
	int i;
	uint8_t angle_direction, car_dist ; 
	uint8_t speed = (uint8_t)((SpeedSensor_get(SPEED_CM_S,SENSOR_L)+SpeedSensor_get(SPEED_CM_S,SENSOR_R))/2.0) ;
	int dist;
	/*------------------------------------------
	 * Send an ultrasound frame every 200ms
	 *-----------------------------------------*/
	//Take the obstacle distance for each sensor
	FSL = US_CalcDistance(0);
	FL  = US_CalcDistance(1);
	FSR = US_CalcDistance(2);
	SL  = US_CalcDistance(3);
	FR  = US_CalcDistance(4);
	SR  = US_CalcDistance(5);
	
	car_dist = speed/5; // Distance of the car in 200 ms 
		
	//Detect an obstacle at less than 2 meters and get the obstacle distance
	
	if (SL < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[0] = 1; 
		dist = (((int)(SL)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[0] += dist;
		if (Prev_SL-SL > car_dist + 2){
			VAL_ULTRA.ultrasound.bytes_ultrasound[0] += 2; 
		}
	}
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[0] = 0;}
	
	if (FSL < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[1] = 1;
		dist = (((int)(FSL)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}		
		VAL_ULTRA.ultrasound.bytes_ultrasound[1] += dist;
		if (Prev_FSL-FSL > car_dist){
			VAL_ULTRA.ultrasound.bytes_ultrasound[1] += 2; 
		}
	}	
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[1] = 0;}	
	
	if (FL < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[2] = 1; 
		dist = (((int)(FL)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[2] += dist;
		if (Prev_FL-FL > car_dist){
			VAL_ULTRA.ultrasound.bytes_ultrasound[2] += 2; 
		}
	}		
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[2] = 0;}
	
	if (FR < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[3] = 1; 
		dist = (((int)(FR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[3] += dist;
		if (Prev_FR-FR > car_dist){
			VAL_ULTRA.ultrasound.bytes_ultrasound[3] += 2; 
		}
	}
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[3] = 0;}
	
	if (FSR < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[4] = 1;
		dist = (((int)(FSR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}		
		VAL_ULTRA.ultrasound.bytes_ultrasound[4] += dist;
		if (Prev_FSR-FSR > car_dist){
			VAL_ULTRA.ultrasound.bytes_ultrasound[4] += 2; 
		}
	}		
	else {VAL_ULTRA.ultrasound.bytes_ultrasound[4] = 0;}
	
	if (SR < 200) {
		VAL_ULTRA.ultrasound.bytes_ultrasound[5] = 1; 
		dist = (((int)(SR)) >> 1) << 2;
		if (dist > 252) {
			dist = 252;
		}
		VAL_ULTRA.ultrasound.bytes_ultrasound[5] += dist;
		if (Prev_SR-SR > car_dist){
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

			if(angle_direction<=117) {
				VAL_POTEN.potentiometer.num_potentiometer = 2;
			} else if(angle_direction<=138) {
				VAL_POTEN.potentiometer.num_potentiometer = 0;
			} else if(angle_direction<=170) {
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
		
			VAL_BAT.battery.num_battery = Battery_get() ; 
			create_battery_frame(VAL_BAT,trame) ; 
			CAN_waitReady() ; 
			CAN_TxRdy2 = 0;    													/* CAN HW unready to transmit message mailbox 2*/
			
			memcpy(CAN_TxMsg.data, trame, sizeof(trame));
			CAN_wrMsg (&CAN_TxMsg);
		
			periodic_modulo = 0;
			break;
	}		
	
	
	// stop the car if no command messages received during the last second
	if (cnt_failure == 4) {		
		if (nb_rcv == 0){
			SpeedRx[0] = 0;
			DirRx[0] = 0;
		}	
		cnt_failure = 0;
	}
	else if (cnt_failure < 4) {
		cnt_failure ++;
	}
}


void Turn(uint8_t deg){
	uint8_t angle = Direction_get() ;
	if (deg > Direction_get() + 5){
		FrontMotor_turn(LEFT);}
	else if (deg < Direction_get() - 5){
		FrontMotor_turn(RIGHT);}
	else {
		Motor_Disable(FRONT_MOTOR);}}

		
void Speed_Cmd(char *cmd){
	if (cmd[0] == '0'){ 										//STOP
		Motor_setSpeed(REAR_MOTOR_L, 0); 
		Motor_setSpeed(REAR_MOTOR_R, 0);}	
	else if (cmd[0] == '1') { 							//Default speed		
		Motor_setSpeed(REAR_MOTOR_L, 0.5); 
		Motor_setSpeed(REAR_MOTOR_R, 0.5);}
	else if (cmd[0] == '2') {     					//Turbo speed		
		Motor_setSpeed(REAR_MOTOR_L, 1); 
		Motor_setSpeed(REAR_MOTOR_R, 1);}}	
		
		
void Dir_Cmd(char *cmd, uint8_t angle){
	if (DirRx[0] == '0'){				//Position centrale des roues
		if (angle >= 127 || angle <= 137){
				Motor_Enable(FRONT_MOTOR);
				Turn(132);}} 															
	else if (DirRx[0] == '1')	{	//Position � gauche des roues		
		if (angle >= 145 || angle <= 155){
				Motor_Enable(FRONT_MOTOR);
				Turn(150);}} 
	else if (DirRx[0] == '2') { //Position � droite des roues
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
	
	SpeedRx[0] = '0';
	DirRx[0] = '0';
	
  Timer_1234_Init (TIM2, 200000);								  /* set Timer 2 every 200ms */
	Timer_Active_IT(TIM2, 0, canPeriodic);					/* Active Timer2 IT					*/
	
  while (1) {
		angle = Direction_get() ;	
		
		FSL  = US_CalcDistance(0);
		FL  = US_CalcDistance(1);
		FSR  = US_CalcDistance(2);
		SL = US_CalcDistance(3);
		FR  = US_CalcDistance(4);
		SR = US_CalcDistance(5);	
		
		if (CAN_RxRdy) {                              //rx msg on CAN Ctrl 
			nb_rcv ++;
			CAN_RxRdy = 0;
			if (CAN_RxMsg.id == CAN_ID_CMD_SPEED){
				memcpy(SpeedRx, CAN_RxMsg.data, sizeof(SpeedRx));}				
			if (CAN_RxMsg.id == CAN_ID_CMD_DIR){
				memcpy(DirRx, CAN_RxMsg.data, sizeof(DirRx));}
		}	
		
		//Speed_Cmd(SpeedRx);
		
		if (SpeedRx[0] == '0')
		{
			Motor_setSpeed(REAR_MOTOR_L, 0); //STOP
			Motor_setSpeed(REAR_MOTOR_R, 0);	
			
		}
		else if (SpeedRx[0] == '1')
		{
			
			Motor_setSpeed(REAR_MOTOR_L, 0.5); //Default speed
			Motor_setSpeed(REAR_MOTOR_R, 0.5);				
		}
		else if (SpeedRx[0] == '2')
		{
			
			Motor_setSpeed(REAR_MOTOR_L, 1); //Turbo speed
			Motor_setSpeed(REAR_MOTOR_R, 1);
		} 

		//Dir_Cmd(DirRx, angle);		
		
		if (DirRx[0] == '0') //Position centrale des roues 127
		{
			if (angle >= 127 || angle <= 137){
				Motor_Enable(FRONT_MOTOR);
				Turn(132);
			}
			
		}
		else if (DirRx[0] == '1') //Position � gauche des roues 155
		{
			if (angle >= 145 || angle <= 155){
				Motor_Enable(FRONT_MOTOR);
				Turn(150);
			}
			
		}
		else if (DirRx[0] == '2') //Position � droite des roues 106
		{
			if (angle >= 105 || angle <= 115){
				Motor_Enable(FRONT_MOTOR);
				Turn(110);
			}
		}			
  }
}
