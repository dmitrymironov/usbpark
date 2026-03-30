/*
	$Id: i2c.h,v 1.3 2009/02/03 16:57:53 dmi Exp $
*/
#ifndef	_I2C_H
#define	_I2C_H

#define	HIGH_SPEED_SLEW_RATE_CONTROL_DISABLED	SSPSTATbits.SMP
#define	SMBUS_INPUTS_ENABLE			SSPSTATbits.CKE

#define	SSCP_MODULE_ENABLE			SSPCON1bits.SSPEN

#define	SDA_INPUT_ENABLE			TRISBbits.TRISB0
#define	SCL_INPUT_ENABLE			TRISBbits.TRISB1

#define	PERIPHERAL_INTERRUPT			INTCONbits.PEIE
#define	I2C_INTERRUPT				PIE1bits.SSPIE

#define	I2C_INTERRUPT_FLAG			PIR1bits.SSPIF

#define	ADMSK1	SSPCON2bits.RSEN
#define	ADMSK2	SSPCON2bits.PEN
#define	ADMSK3	SSPCON2bits.RCEN
#define	ADMSK4	SSPCON2bits.ACKEN
#define	ADMSK5	SSPCON2bits.ACKDT

void open_i2c();
void close_i2c();
void process_i2c_interrupt();
#endif//_I2C_H
