/*----------------------------------------------------------------------------
 * Name:    CanDemo.c
 * Purpose: CAN example for MCBSTM32
 * Note(s): possible defines set in "options for target - C/C++ - Define"
 *            __USE_LCD   - enable Output on LCD
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009-2013 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

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
char trameRx[8];
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
  display transmit and receieve values
 *----------------------------------------------------------------------------*/
void val_display (void) {

#ifdef __USE_LCD
  //sprintf(text, "Tx:0x%02X, Rx:0x%02X", val_Tx, val_Rx);
	lcd_clear ();
	sprintf(text, "RX %d", CAN_RxMsg.id);
	
	lcd_print  (text);
	sprintf(text, "TX %s", trame);
  set_cursor (0, 1);
  lcd_print  (text);                              /* print string to LCD      */
#endif

  Delay (10);                                     /* dlay for 10ms            */
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

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
//int main (void)  {
//  
//	Timer_1234_Init (TIM1, 200000);								/* set Timer 2 every second */
//	Timer_Active_IT(TIM1, 0, canPeriodic);					/* Active Timer2 IT					*/

//  SysTick_Config(SystemCoreClock / 1000);         /* SysTick 1 msec IRQ       */

//#ifdef __USE_LCD
//  lcd_init  ();                                   /* initialise LCD           */
//  lcd_clear ();
//  lcd_print ("MCBSTM32 CanDemo");
//  set_cursor (0, 1);
//  lcd_print ("  www.keil.com  ");
//  Delay (4000);                                   /* Wait for initial display */

//  lcd_clear ();
//  lcd_print ("CAN at 500kbit/s");
//#endif

//  can_Init ();                                    /* initialize CAN interface */

//  
//  while (1) {
//		
//    if (CAN_RxRdy) {                              /* rx msg on CAN Ctrl       */
//      //memcpy(trameRx, CAN_RxMsg.data, sizeof(trameRx));
//			val_display (); 
//			CAN_RxRdy = 0;
//      
//			//val_Rx = CAN_RxMsg.data[0];
//			
//    }
//                                  /* display TX and RX values */
//  }
//}
