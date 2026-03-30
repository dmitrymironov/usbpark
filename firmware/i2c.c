//
// $Id: i2c.c,v 1.5 2009/02/10 23:18:47 dmi Exp $
//

#include	"p18f2550.h"

#if	defined(USE_I2C)
#include	"i2c.h"
#endif

#include	"rc2sample.h"

extern unsigned int state;

#if	defined(USE_I2C)
void open_i2c(){
	SSCP_MODULE_ENABLE=0;
	///////////////////

	HIGH_SPEED_SLEW_RATE_CONTROL_DISABLED=1;
	SMBUS_INPUTS_ENABLE=0;

	SDA_INPUT_ENABLE=1;
	SCL_INPUT_ENABLE=1;

	// 7-bit slave mode
	ADMSK1	=	0x1;		
	ADMSK2	=	0x1;
	ADMSK3	=	0x1;
	ADMSK4	=	0x1;
	ADMSK5	=	0x1;

	SSPADD	=	0x1;

	I2C_INTERRUPT		=1;
	PERIPHERAL_INTERRUPT	=1;
		
	///////////////////
	SSCP_MODULE_ENABLE=1;
	}

void close_i2c(){

	PERIPHERAL_INTERRUPT	=0;
	I2C_INTERRUPT		=0;
	SSCP_MODULE_ENABLE	=0;
	}

void process_i2c_interrupt(){
	state=ST_I2C_INTERRUPT1;
	}
#if	defined(USE_I2C)

