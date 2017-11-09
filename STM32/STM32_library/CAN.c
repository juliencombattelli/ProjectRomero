/*----------------------------------------------------------------------------
 * Name:    CAN.c
 * Purpose: low level CAN functions
 * Note(s): 
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

#include "STM32F10x.h"
#include "CAN.h"

/* CAN identifier type */
#define CAN_ID_STD            ((uint32_t)0x00000000)  /* Standard Id          */
#define CAN_ID_EXT            ((uint32_t)0x00000004)  /* Extended Id          */

/* CAN remote transmission request */
#define CAN_RTR_DATA          ((uint32_t)0x00000000)  /* Data frame           */
#define CAN_RTR_REMOTE        ((uint32_t)0x00000002)  /* Remote frame         */


CAN_msg       CAN_TxMsg;                  /* CAN messge for sending           */
CAN_msg       CAN_RxMsg;                  /* CAN message for receiving        */                        

unsigned int  CAN_TxRdy0 = 0;              /* CAN HW ready to transmit message mailbox 0*/
unsigned int  CAN_TxRdy1 = 0;              /* CAN HW ready to transmit message mailbox 1*/
unsigned int  CAN_TxRdy2 = 0;              /* CAN HW ready to transmit message mailbox 2*/
unsigned int  CAN_RxRdy = 0;              /* CAN HW received a message        */


/*----------------------------------------------------------------------------
  setup CAN interface
 *----------------------------------------------------------------------------*/
void CAN_setup (void)  {
  unsigned int brp;

  RCC->APB1ENR |= ( 1UL << 25);           /* enable clock for CAN             */

  /* Note: MCBSTM32 uses PB8 and PB9 for CAN                                  */
  RCC->APB2ENR |=  ( 1UL <<  0);          /* enable clock for AF              */
  AFIO->MAPR   &=  0xFFFF9FFF;            /* reset CAN remap                  */
  AFIO->MAPR   |=  0x00004000;            /*   set CAN remap, use PB8, PB9    */

  RCC->APB2ENR |=  ( 1UL <<  3);          /* enable clock for GPIO B          */
  GPIOB->CRH   &= ~(0x0F <<  0);
  GPIOB->CRH   |=  (0x08 <<  0);          /* CAN RX PB.8 input push pull      */
  
  GPIOB->CRH   &= ~(0x0F <<  4);
  GPIOB->CRH   |=  (0x0B <<  4);          /* CAN TX PB.9 alt.output push pull */ 

  NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);    /* enable CAN TX interrupt          */
  NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);   /* enable CAN RX interrupt          */

  CAN1->MCR = (CAN_MCR_INRQ   |           /* initialisation request           */
               CAN_MCR_NART    );         /* no automatic retransmission      */
                                          /* only FIFO 0, tx mailbox 0 used!  */
  CAN1->IER = (CAN_IER_FMPIE0 |           /* enable FIFO 0 msg pending IRQ    */
               CAN_IER_TMEIE    );        /* enable Transmit mbx empty IRQ    */

  /* Note: this calculations fit for PCLK1 = 36MHz */
  brp  = (36000000UL / 18) / 500000;      /* baudrate is set to 500k bit/s    */
                                                                          
  /* set BTR register so that sample point is at about 72% bit time from bit start */
  /* TSEG1 = 12, TSEG2 = 5, SJW = 4 => 1 CAN bit = 18 TQ, sample at 72%    */
  CAN1->BTR &= ~(((        0x03) << 24) | ((        0x07) << 20) | ((         0x0F) << 16) | (          0x1FF)); 
  CAN1->BTR |=  ((((4-1) & 0x03) << 24) | (((5-1) & 0x07) << 20) | (((12-1) & 0x0F) << 16) | ((brp-1) & 0x1FF));
}


/*----------------------------------------------------------------------------
  leave initialisation mode
 *----------------------------------------------------------------------------*/
void CAN_start (void)  {

  CAN1->MCR &= ~CAN_MCR_INRQ;             /* normal operating mode, reset INRQ*/
  while (CAN1->MSR & CAN_MCR_INRQ);
}



/*----------------------------------------------------------------------------
  check if transmit mailbox is empty
 *----------------------------------------------------------------------------*/
void CAN_waitReady (void)  {
  switch (CAN_TxMsg.id) {
	case CAN_ID_ULTRASOUND: 
		while ((CAN1->TSR & CAN_TSR_TME0) == 0);  /* Transmit mailbox 0 is empty    */
		CAN_TxRdy0 = 1;
		break;
	case CAN_ID_DIR:
		while ((CAN1->TSR & CAN_TSR_TME1) == 0);  /* Transmit mailbox 1 is empty    */
		CAN_TxRdy1 = 1;
		break;
	case CAN_ID_SPEED:
		while ((CAN1->TSR & CAN_TSR_TME2) == 0);  /* Transmit mailbox 2 is empty    */
		CAN_TxRdy2 = 1;
		break;
	default:
		break;
	}
}


/*----------------------------------------------------------------------------
  wite a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
void CAN_wrMsg (CAN_msg *msg)  {
	int numBox;
	
	switch (msg->id) {
		case CAN_ID_ULTRASOUND:
			numBox = 0;
			break;
		case CAN_ID_DIR:
			numBox = 1;
			break;
		default:
			numBox = 2;
			break;
	}
	
  CAN1->sTxMailBox[numBox].TIR  = 0;           /* Reset TIR register               */
                                          /* Setup identifier information     */
  if (msg->format == STANDARD_FORMAT) {   /*    Standard ID                   */
    CAN1->sTxMailBox[numBox].TIR |= (uint32_t)(msg->id << 21) | CAN_ID_STD;
  } else {                                /* Extended ID                      */
    CAN1->sTxMailBox[numBox].TIR |= (uint32_t)(msg->id <<  3) | CAN_ID_EXT;
  }
                                          /* Setup type information           */
  if (msg->type == DATA_FRAME)  {         /* DATA FRAME                       */
    CAN1->sTxMailBox[numBox].TIR |= CAN_RTR_DATA;
  } else {                                /* REMOTE FRAME                     */
    CAN1->sTxMailBox[numBox].TIR |= CAN_RTR_REMOTE;
  }
                                          /* Setup data bytes                 */
  CAN1->sTxMailBox[numBox].TDLR = (((uint32_t)msg->data[3] << 24) | 
                              ((uint32_t)msg->data[2] << 16) |
                              ((uint32_t)msg->data[1] <<  8) | 
                              ((uint32_t)msg->data[0])        );
  CAN1->sTxMailBox[numBox].TDHR = (((uint32_t)msg->data[7] << 24) | 
                              ((uint32_t)msg->data[6] << 16) |
                              ((uint32_t)msg->data[5] <<  8) |
                              ((uint32_t)msg->data[4])        );
                                          /* Setup length                     */
  CAN1->sTxMailBox[numBox].TDTR &= ~CAN_TDT0R_DLC;
  CAN1->sTxMailBox[numBox].TDTR |=  (msg->len & CAN_TDT0R_DLC);

  CAN1->IER |= CAN_IER_TMEIE;                 /* enable  TME interrupt        */
  CAN1->sTxMailBox[numBox].TIR |=  CAN_TI0R_TXRQ;  /* transmit message             */
}


/*----------------------------------------------------------------------------
  read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
void CAN_rdMsg (CAN_msg *msg)  {
  int numFIFO;
	switch (msg->id) {
		case CAN_ID_CMD_SPEED:
			numFIFO = 0;
			break;
		case CAN_ID_CMD_DIR:
			numFIFO = 0;
			break;
		case CAN_ID_RMT_ULTRASOUND:
			numFIFO = 1;
			break;
		case CAN_ID_RMT_DIR:
			numFIFO = 1;
			break;
		case CAN_ID_RMT_SPEED:
			numFIFO = 1;
			break;
	}      

	/* Read identifier information  */
  if ((CAN1->sFIFOMailBox[numFIFO].RIR & CAN_ID_EXT) == 0) {
    msg->format = STANDARD_FORMAT;
    msg->id     = 0x000007FF & (CAN1->sFIFOMailBox[numFIFO].RIR >> 21);
  } else {
    msg->format = EXTENDED_FORMAT;
    msg->id     = 0x1FFFFFFF & (CAN1->sFIFOMailBox[numFIFO].RIR >> 3);
  }
                                              /* Read type information        */
  if ((CAN1->sFIFOMailBox[numFIFO].RIR & CAN_RTR_REMOTE) == 0) {
    msg->type =   DATA_FRAME;
  } else {
    msg->type = REMOTE_FRAME;
  }
                                              /* Read number of rec. bytes    */
  msg->len     = (CAN1->sFIFOMailBox[numFIFO].RDTR      ) & 0x0F;
                                              /* Read data bytes              */
  msg->data[0] = (CAN1->sFIFOMailBox[numFIFO].RDLR      ) & 0xFF;
  msg->data[1] = (CAN1->sFIFOMailBox[numFIFO].RDLR >>  8) & 0xFF;
  msg->data[2] = (CAN1->sFIFOMailBox[numFIFO].RDLR >> 16) & 0xFF;
  msg->data[3] = (CAN1->sFIFOMailBox[numFIFO].RDLR >> 24) & 0xFF;

  msg->data[4] = (CAN1->sFIFOMailBox[numFIFO].RDHR      ) & 0xFF;
  msg->data[5] = (CAN1->sFIFOMailBox[numFIFO].RDHR >>  8) & 0xFF;
  msg->data[6] = (CAN1->sFIFOMailBox[numFIFO].RDHR >> 16) & 0xFF;
  msg->data[7] = (CAN1->sFIFOMailBox[numFIFO].RDHR >> 24) & 0xFF;

  CAN1->RF0R |= CAN_RF0R_RFOM0;             /* Release FIFO 0 output mailbox */
}

/*----------------------------------------------------------------------------
  CAN write message filter
 *----------------------------------------------------------------------------*/
void CAN_wrFilter (unsigned int id, unsigned char format)  {
  static unsigned short CAN_filterIdx = 0;
         unsigned int   CAN_msgId     = 0;
  
  if (CAN_filterIdx > 13) {                 /* check if Filter Memory is full*/
    return;
  }
                                            /* Setup identifier information  */
  if (format == STANDARD_FORMAT)  {         /*   Standard ID                 */
      CAN_msgId |= (uint32_t)(id << 21) | CAN_ID_STD;
  }  else  {                                /*   Extended ID                 */
      CAN_msgId |= (uint32_t)(id <<  3) | CAN_ID_EXT;
  }

  CAN1->FMR  |=   CAN_FMR_FINIT;            /* set initMode for filter banks */
  CAN1->FA1R &=  ~(1UL << CAN_filterIdx);   /* deactivate filter             */

                                            /* initialize filter             */
  CAN1->FS1R |= (unsigned int)(1 << CAN_filterIdx);     /* set 32-bit scale configuration    */
  CAN1->FM1R |= (unsigned int)(1 << CAN_filterIdx);     /* set 2 32-bit identifier list mode */

  CAN1->sFilterRegister[CAN_filterIdx].FR1 = CAN_msgId; /*  32-bit identifier                */
  CAN1->sFilterRegister[CAN_filterIdx].FR2 = CAN_msgId; /*  32-bit identifier                */
  if (id == CAN_ID_CMD_SPEED || id == CAN_ID_CMD_DIR) {
		CAN1->FFA1R &= ~(unsigned int)(1 << CAN_filterIdx); /* assign filter to FIFO 0           */
	}
	else {
		CAN1->FFA1R &= (unsigned int)(1 << CAN_filterIdx);  /* assign filter to FIFO 1           */
	}
  CAN1->FA1R  |=  (unsigned int)(1 << CAN_filterIdx);   /* activate filter                   */

  CAN1->FMR &= ~CAN_FMR_FINIT;              /* reset initMode for filterBanks*/

  CAN_filterIdx += 1;                       /* increase filter index         */
}


/*----------------------------------------------------------------------------
  CAN transmit interrupt handler
 *----------------------------------------------------------------------------*/
void USB_HP_CAN1_TX_IRQHandler (void) {
	switch (CAN_TxMsg.id) {
		case CAN_ID_ULTRASOUND: 
			if (CAN1->TSR & CAN_TSR_RQCP0) {          /* request completed mbx 0        */
				CAN1->TSR |= CAN_TSR_RQCP0;             /* reset request complete mbx 0   */
			}
			CAN_TxRdy0 = 1;
			break;
		case CAN_ID_DIR:
			if (CAN1->TSR & CAN_TSR_RQCP1) {          /* request completed mbx 1        */
				CAN1->TSR |= CAN_TSR_RQCP1;             /* reset request complete mbx 1   */
			}
			CAN_TxRdy1 = 1;
			break;
		case CAN_ID_SPEED:
			if (CAN1->TSR & CAN_TSR_RQCP2) {          /* request completed mbx 2        */
				CAN1->TSR |= CAN_TSR_RQCP2;             /* reset request complete mbx 2   */
			}
			CAN_TxRdy2 = 1;
			break;
		default:
			break;
	}	
  CAN1->IER &= ~CAN_IER_TMEIE;            /* disable  TME interrupt         */
}


/*----------------------------------------------------------------------------
  CAN receive interrupt handler
 *----------------------------------------------------------------------------*/
void USB_LP_CAN1_RX0_IRQHandler (void) {

  if (CAN1->RF0R & CAN_RF0R_FMP0) {			/* message pending ?              */
	CAN_rdMsg (&CAN_RxMsg);                 /* read the message               */

    CAN_RxRdy = 1;                          /* set receive flag               */
  }
}
