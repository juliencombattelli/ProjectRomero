/*----------------------------------------------------------------------------
 * Name:    CAN.h
 * Purpose: low level CAN definitions
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

#ifndef __CAN_H
#define __CAN_H

#define STANDARD_FORMAT  0
#define EXTENDED_FORMAT  1

#define DATA_FRAME       0
#define REMOTE_FRAME     1

#define CAN_ID_ULTRASOUND			((unsigned int) 1) 				/* Identificator priority for the ultrasound frame */
#define CAN_ID_DIR						((unsigned int) 6) 				/* Identificator priority for the direction frame */
#define CAN_ID_SPEED					((unsigned int) 7) 				/* Identificator priority for the speed frame */
#define CAN_ID_CMD_DIR				((unsigned int) 2) 				/* Identificator priority for the direction control frame */
#define CAN_ID_CMD_SPEED			((unsigned int) 3) 				/* Identificator priority for the speed control frame */
#define CAN_ID_RMT_ULTRASOUND	((unsigned int) 9) 				/* Identificator priority for the ultrasound remote frame */
#define CAN_ID_RMT_DIR				((unsigned int) 4) 				/* Identificator priority for the direction remote frame */
#define CAN_ID_RMT_SPEED			((unsigned int) 5) 				/* Identificator priority for the speed remote frame */

typedef struct  {
  unsigned int   id;                 // 29 bit identifier
  unsigned char  data[8];            // Data field
  unsigned char  len;                // Length of data field in bytes
  unsigned char  format;             // 0 - STANDARD, 1- EXTENDED IDENTIFIER
  unsigned char  type;               // 0 - DATA FRAME, 1 - REMOTE FRAME
} CAN_msg;

/* Functions defined in module CAN.c */
void CAN_setup         (void);
void CAN_init          (void);
void CAN_start         (void);
void CAN_waitReady     (void);
void CAN_wrMsg         (CAN_msg *msg);
void CAN_rdMsg         (CAN_msg *msg);
void CAN_wrFilter      (unsigned int id, unsigned char filter_type);

void CAN_testmode      (unsigned int testmode);

extern CAN_msg       CAN_TxMsg;      // CAN messge for sending
extern CAN_msg       CAN_RxMsg;      // CAN message for receiving                                
extern unsigned int  CAN_TxRdy0;      // CAN HW ready to transmit a message mailbox 0
extern unsigned int  CAN_TxRdy1;      // CAN HW ready to transmit a message mailbox 1
extern unsigned int  CAN_TxRdy2;      // CAN HW ready to transmit a message mailbox 2
extern unsigned int  CAN_RxRdy;      // CAN HW received a message

#endif // __CAN_H


