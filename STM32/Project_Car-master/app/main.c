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

/*-------------------------------------------
---------------------------------------------
--------------------------------------------*/

char text[17];
uint8_t battery_level ; 

unsigned int val_Tx = 0, val_Rx = 0;              /* Globals used for display */
char trame[8];
char SpeedRx[1];
char DirRx[1];
unsigned int periodic_modulo = 0;                 /* Global used to determine the time to send periodic CAN frame */

volatile uint32_t msTicks;                        /* counts 1ms timeTicks     */
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
  CAN_wrFilter (CAN_ID_RMT_ULTRASOUND, STANDARD_FORMAT);             /* Enable reception of msgs */
	CAN_wrFilter (CAN_ID_CMD_DIR, STANDARD_FORMAT);             /* Enable reception of msgs */
	CAN_wrFilter (CAN_ID_CMD_SPEED, STANDARD_FORMAT);             /* Enable reception of msgs */
	CAN_wrFilter (CAN_ID_RMT_DIR, STANDARD_FORMAT);             /* Enable reception of msgs */
	CAN_wrFilter (CAN_ID_RMT_SPEED, STANDARD_FORMAT);             /* Enable reception of msgs */
	
  CAN_start ();                                   /* start CAN Controller   */
	CAN_TxMsg.id = CAN_ID_ULTRASOUND;
  CAN_waitReady ();                               /* wait til tx mbx is empty */
	
	//TODO: TO DELETE
	VAL_ULTRA.bytes_ultrasound[0] = '0'; 
	VAL_ULTRA.bytes_ultrasound[1] = '1'; 

}

 
void canPeriodic (void) {
	int i;
	uint8_t angle_direction ; 
	uint8_t speed = (uint8_t)((SpeedSensor_get(SPEED_CM_S,SENSOR_L)+SpeedSensor_get(SPEED_CM_S,SENSOR_R))/2.0) ;
	
	
	
	/*------------------------------------------
	 * Send an ultrasound frame every 200ms
	 *-----------------------------------------*/
	CAN_TxMsg.id = CAN_ID_ULTRASOUND;                /* initialize msg to send   */  
  for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
  CAN_TxMsg.len = 8;
  CAN_TxMsg.format = STANDARD_FORMAT;
  CAN_TxMsg.type = DATA_FRAME;	
	
	create_ultrasound_frame(VAL_ULTRA, trame);
	//val_Tx++; //Value of the ultrasound sensorstrame
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
			battery_level = Battery_get() ; 
			periodic_modulo = 0;
			break;
	}		
}


void Turn(uint8_t deg){
	uint8_t angle = Direction_get() ;
	if (deg > Direction_get() + 5){
		FrontMotor_turn(LEFT);}
	else if (deg < Direction_get() - 5){
		FrontMotor_turn(RIGHT);}
	else {
		FrontMotor_turn(NONE);}}

		
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
		
		
void Dir_Cmd(char *cmd){
	if (DirRx[0] == 0){				//Position centrale des roues
		Turn(132);} 															
	else if (DirRx[0] == 1)	{	//Position à gauche des roues		
		Turn(150);} 
	else if (DirRx[0] == 2) { //Position à droite des roues
		Turn(110);}}
		
/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
uint8_t angle;
float SR, SL, FSR, FR, FL, FSL;
		
int main (void)  {
  
	
	Timer_1234_Init (TIM2, 200000);								/* set Timer 2 every second */
	Timer_Active_IT(TIM2, 0, canPeriodic);					/* Active Timer2 IT					*/

  SysTick_Config(SystemCoreClock / 1000);         /* SysTick 1 msec IRQ       */

	Manager_Init();
	Motor_QuickInit(REAR_MOTOR_L);
	Motor_QuickInit(REAR_MOTOR_R);
	Motor_Enable(REAR_MOTOR_L);
	Motor_Enable(REAR_MOTOR_R);	
  can_Init ();                                    /* initialize CAN interface */
  
  while (1) {
		angle = Direction_get() ;	
		
		FSL  = US_CalcDistance(0);
		FL  = US_CalcDistance(1);
		FSR  = US_CalcDistance(2);
		SL = US_CalcDistance(3);
		FR  = US_CalcDistance(4);
		SR = US_CalcDistance(5);
		
		SpeedRx[0] = 2;
		
		if (CAN_RxRdy) {                              //rx msg on CAN Ctrl         		
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
			Motor_setSpeed(REAR_MOTOR_L, 0.5); //Default speed
			Motor_setSpeed(REAR_MOTOR_R, 0.5);				
		}
		else if (SpeedRx[0] == 2)
		{
			Motor_setSpeed(REAR_MOTOR_L, 1); //Turbo speed
			Motor_setSpeed(REAR_MOTOR_R, 1);
		} 

		//Dir_Cmd(DirRx);		
		
		if (DirRx[0] == 0) //Position centrale des roues 127
		{
			Turn(127);
		}
		else if (DirRx[0] == 1) //Position à gauche des roues 155
		{
			Turn(150);
		}
		else if (DirRx[0] == 2) //Position à droite des roues 106
		{
			Turn(110);
		}			
		
		
		

  }
}
