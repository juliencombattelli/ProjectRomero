#include <stdint.h>
#include <stm32f10x.h>

#include "system_time.h"
#include "manager.h"
#include "motors.h"
#include "front_motor.h"
#include "direction.h"
#include <stdio.h>
#include <string.h>


#include "stm32f10x.h"                            /* STM32F10x Definitions    */
#include "LCD.h"                                  /* LCD function prototypes  */
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

unsigned int val_Tx = 0, val_Rx = 0;              /* Globals used for display */
char trame[8];
char SpeedRx[8];
char DirRx[8];
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
	
	VAL_ULTRA.bytes_ultrasound[0] = '0'; 
	VAL_ULTRA.bytes_ultrasound[1] = '1'; 
	VAL_ULTRA.bytes_ultrasound[2] = '2'; 
	VAL_ULTRA.bytes_ultrasound[3] = '3'; 
	VAL_ULTRA.bytes_ultrasound[4] = '4'; 
	VAL_ULTRA.bytes_ultrasound[5] = '5'; 
	
	VAL_ODOMETER.left_odometer.bytes_left_odometer[0] = 'A';
	VAL_ODOMETER.left_odometer.bytes_left_odometer[1] = 'B';
	VAL_ODOMETER.right_odometer.bytes_right_odometer[0] = 'C';
	VAL_ODOMETER.right_odometer.bytes_right_odometer[1] = 'D';
	VAL_POTEN.potentiometer.bytes_potentiometer[0] = 'X';
  VAL_POTEN.potentiometer.bytes_potentiometer[1] = 'Y';
}

void canPeriodic (void) {
	int i;
	
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
	
	//CAN_TxMsg.data[0] = trame[0];                 /* data[0] = ADC value      */
	//CAN_TxMsg.data[1] = trame[1];                 /* data[0] = ADC value      */
	//CAN_TxMsg.data[2] = trame[2];                 /* data[0] = ADC value      */
	//CAN_TxMsg.data[3] = trame[3];                 /* data[0] = ADC value      */
	//CAN_TxMsg.data[4] = trame[4];                 /* data[0] = ADC value      */
	//CAN_TxMsg.data[5] = trame[5];                 /* data[0] = ADC value      */
	
	memcpy(CAN_TxMsg.data, trame, sizeof(trame));
	CAN_wrMsg (&CAN_TxMsg);                     /* transmit message         */
	
	switch (periodic_modulo) {
		/*------------------------------------------
	 * Send an direction frame every 600ms
	 *-----------------------------------------*/
		case 0:
			CAN_TxMsg.id = CAN_ID_DIR;                /* initialize msg to send   */  
			for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
			CAN_TxMsg.len = 8;
			CAN_TxMsg.format = STANDARD_FORMAT;
			CAN_TxMsg.type = DATA_FRAME;	
		
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
		
			create_odometer_frame(VAL_ODOMETER, trame);  
			CAN_waitReady ();
			CAN_TxRdy2 = 0;    													/* CAN HW unready to transmit message mailbox 2*/
					
			memcpy(CAN_TxMsg.data, trame, sizeof(trame));
			CAN_wrMsg (&CAN_TxMsg);
			periodic_modulo++;
			break;
		
		case 2:
			periodic_modulo = 0;
			break;
	}		
}


int Tourner(uint8_t deg){
	uint8_t angle = Direction_get() ;
	if (deg > Direction_get() + 5){
		FrontMotor_turn(LEFT);
	}
	else if (deg < Direction_get() - 5) {
		FrontMotor_turn(RIGHT);
	}
	else {
		FrontMotor_turn(NONE);
	}
	

}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
uint8_t angle;

int main (void)  {
  
	Timer_1234_Init (TIM2, 200000);								/* set Timer 2 every second */
	Timer_Active_IT(TIM2, 0, canPeriodic);					/* Active Timer2 IT					*/

  SysTick_Config(SystemCoreClock / 1000);         /* SysTick 1 msec IRQ       */

	Manager_Init();
	Motor_QuickInit(REAR_MOTOR_L);
	Motor_QuickInit(REAR_MOTOR_R);
	Motor_Enable(REAR_MOTOR_L);
	Motor_Enable(REAR_MOTOR_R);
	
#ifdef __USE_LCD
  lcd_init  ();                                   /* initialise LCD           */
  lcd_clear ();
  lcd_print ("MCBSTM32 CanDemo");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");
  Delay (4000);                                   /* Wait for initial display */

  lcd_clear ();
  lcd_print ("CAN at 500kbit/s");
#endif

  can_Init ();                                    /* initialize CAN interface */

  
  while (1) {
		angle = Direction_get() ;				
		if (CAN_RxRdy) {                              //rx msg on CAN Ctrl       
      //memcpy(trameRx, CAN_RxMsg.data, sizeof(trameRx));			
			CAN_RxRdy = 0;
			if (CAN_RxMsg.id == CAN_ID_CMD_SPEED)
			{
				memcpy(SpeedRx, CAN_RxMsg.data, sizeof(SpeedRx));	
			}				
			if (CAN_RxMsg.id == CAN_ID_CMD_DIR)
			{
				memcpy(DirRx, CAN_RxMsg.data, sizeof(DirRx));	
			}
    }		
		
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

		if (DirRx[0] == '0') //Position centrale des roues
		{
			Tourner(132);
		}
		else if (DirRx[0] == '1') //Position à gauche des roues
		{
			Tourner(150);
		}
		else if (DirRx[0] == '2') //Position à droite des roues
		{
			Tourner(110);
		}		
  }
}