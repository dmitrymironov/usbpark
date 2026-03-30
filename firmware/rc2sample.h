#ifndef	RC2_SAMPLE_H
#define	RC2_SAMPLE_H

// TODO: Check loganalyz. This firmware misses an end of short
// impulse by some reason

////////////////////////////////////////////////////////////////////
//
// SHARED PART
//

#define	EEPROM_FLAG_ADDR			0x0001
#define	EEPROM_BUZZER_PR2			0x0002
#define	EEPROM_BUZZER_CCPR1L		0x0003
#define	EEPROM_BUZZER_FLAGS			0x0004
#define	EEPROM_SHIFT_8BYTES			0x0005

// Only ON will trigger the bootloader
#define	EEPROM_BOOTLOADER_ON			0xEB
#define	EEPROM_BOOTLOADER_OFF			0xCD

// ************************************* !HID_BOOTLOADER ** BEGIN
// With Bootloader, it is highly critical to remove
// just everything that does not matter
#if !defined(HID_BOOTLOADER)
void make_in_packet(void);

#endif
// ************************************* !HID_BOOTLOADER ** END

// Size of incoming word with command
#define	HOST_COMMAND_SIZE		7
// Size + 1
//
// Dictated by particular case of challenger 26 protocol instruction code
//
// <command>
// <num>
// <proto>
// <line bit>
// <proto>
// <line bit>


// Number of impulses we can keep in memory
// due to minimal packet size of 64
// It can't be more than 12 1+(12*(1+4))=60)
#define	USB_PACKET_SIZE			12

// Number of Quant structure in deckCycles
#define	STORAGE_PACKET_SIZE 		104

enum COMMAND {
	CMD_READ_PARKTRONIC=1,
	CMD_STOP_READING, //2
	CMD_PING,  // 3
	CMD_GET_FIRMWARE_STATE, //4
	CMD_GET_DATA_PACKET, // 5
	CMD_RESET, // 6
	CMD_GET_VERSION, // 7
	CMD_READ_DATA_PACKET, // 8

// After that command, control packet will instruct
// the device to parse special data formats
// Issue CMD_STOP_READING to cancel read
/*
	Control packet format
	CMD_DATA_FORMAT

	<num of specific instructions>

	<protocol control byte>
	<protocol specific data of known length>

	Case of challenger 26 protocol:
	CMD_DATA_FORMAT

	1 or 2  - number of lines we will process

	word:
		26 - challenger 26 protocol
		7,6,5,4 - RB7, RB6, RB5 or RB4, respectively
*/
	CMD_DATA_FORMAT, //9


	CMD_GET_IO_REGISTERS1, //10
	CMD_GET_IO_REGISTERS2,//11
	CMD_SET_REGISTER,//12
	CMD_ENTER_BOOTLOADER,//13
/*
	Timeout happens when line is silent for a period of time,
	so for example reverse gear was un-plugged.

	MUST BE GREATER THEN MAX num of HEADS for the line
*/
	CMD_SET_BUZZER,//14
	CMD_READ_BUZZER, //15
	CMD_SET_SHIFT,//16
	CMD_READ_SHIFT, // 17,
	CMD_GET_THERMAL, // 18
	CMD_DEVICE_ERROR,//19
	CMD_DATA_TIMEOUT//20 -- LAST ONE!!
};

enum DEVICE_ERROR_CODES {
	DEVICE_ERROR_MSDP=1
	};

// 0xC8 - 8-bit, no prescaler
// 0x88 - no prescaler, 1:1; 1000 cycles/6 seconds, 8 MHz
// 0x87 - 1:256
// 0x82 - 1:8
// 0x81 - 1:4
// 0x80 - 1:2
//#define	T0C0N_PRESCALE	0x88

//
// Support for 8-buzzer extension board
//
#define	MULTI_BUZZER

#define	T0C0N_PRESCALE	0x82

#define	version_buf	m_send2host._version_buf
#define	usbStat		m_send2host._usbStat


#if	!defined(HOST_APPLICATION)
#if	!defined(RC2_LINE)
#define	RC2_LINE	PORTCbits.RC2
#endif //!defined(HOST_APPLICATION)
#endif //!defined(RC2_LINE)

// Aliases

// 7
#define	HIGH_PRIO_IRPT_ENABLE	INTCONbits.GIEH
#define	IRPT_PRIORITIES_ENABLE	RCONbits.IPEN
// 6
//
// Do not use priorities on the interrupts, USB trouble
//
#define	LOW_PRIO_IRPT_ENABLE	INTCONbits.GIEL
// 6
#define	PERIPHERAL_IRPT_ENABLE	INTCONbits.PEIE

// 7
#define	LOW_SIG_PULLUP_DISABLE	INTCON2bits.RBPU
// 5
#define	TIMER0_IRPT_ENABLE	INTCONbits.TMR0IE
// 2
#define	TIMER0_PRIO		INTCON2bits.TMR0IP
// 2
#define	TIMER0_FLAG		INTCONbits.TMR0IF
// 7
#define	TIMER0_ON		T0CONbits.TMR0ON

// 1
#define	TIMER3_IRPT_ENABLE	PIE2bits.TMR3IE
// 1
#define	TIMER3_PRIO		IPR2bits.TMR3IP
// 1
#define	TIMER3_FLAG		PIR2bits.TMR3IF
// 0
#define	TIMER3_ON		T3CONbits.TMR3ON

// THERMAL BUS DATA PIN
// Use interrupt locking
//#define	DS1820_INTERRUPT_LOCK
#define	DS1820_DATAPIN PORTCbits.RC6

#define	PORTB_IRPT_ON_CHANGE_ENABLE	INTCONbits.RBIE
#define	PORTB_IRPT_ON_CHANGE_PRIO	INTCON2bits.RBIP
#define	PORTB_IRPT_ON_CHANGE_FLAG	INTCONbits.RBIF

// PORTBbits.RB4
#define	DATA_PORT		PORTB
//#define	DATA_PIN		(PORTB&0x10)
//#define	CONTROL_PIN		TRISBbits.TRISB4

enum REGS {
	REG_INDF2=1,
	REG_POSTINC2=2,
	REG_POSTDEC2=3,
	REG_PREINC2=4,
	REG_PLUSW2=5,
	REG_FSR2H=6,
	REG_FSR2L=7,
	REG_STATUS=8,
	REG_TMR0H=9,
	REG_TMR0L=10,
	REG_T0CON=11,
	REG_OSCCON=12,
	REG_HLVDCON=13,
	REG_WDTCON=14,
	REG_RCON=15,
	REG_TMR1H=16,
	REG_TMR1L=17,
	REG_T1CON=18,
	REG_TMR2=19,
	REG_PR2=20,
	REG_T2CON=21,
	REG_SSPBUF=22,
	REG_SSPADD=23,
	REG_SSPSTAT=24,
	REG_SSPCON1=25,
	REG_SSPCON2=26,
	REG_ADRESH=27,
	REG_ADRESL=28,
	REG_ADCON0=29,
	REG_ADCON1=30,
	REG_ADCON2=31,
	REG_CCPR1H=32,
	REG_CCPR1L=33,
	REG_CCP1CON=34,
	REG_CCPR2H=35,
	REG_CCPR2L=36,
	REG_CCP2CON=37,
	REG_BAUDCON=38,
	REG_ECCP1DEL=39,
	REG_ECCP1AS=40,
	REG_CVRCON=41,
	REG_CMCON=42,
	REG_TMR3H=43,
	REG_TMR3L=44,
	REG_T3CON=45,
	REG_SPBRGH=46,
	REG_SPBRG=47,
	REG_RCREG=48,
	REG_TXREG=49,
	REG_TXSTA=50,
	REG_RCSTA=51,
	REG_EEADR=52,
	REG_EEDATA=53,
	REG_EECON2=54,
	REG_EECON1=55,
	REG_IPR2=56,
	REG_PIR2=57,
	REG_PIE2=58,
	REG_IPR1=59,
	REG_PIR1=60,
	REG_PIE1=61,
	REG_OSCTUNE=62,
	REG_TRISC=63,
	REG_TRISB=64,
	REG_TRISA=65,
	REG_LATC=66,
	REG_LATB=67,
	REG_LATA=68,
	REG_PORTE=69,
	REG_PORTD=70,
	REG_PORTC=71,
	REG_PORTB=72,
	REG_PORTA=73,
	REG_UEP15=74,
	REG_UEP14=75,
	REG_UEP13=76,
	REG_UEP12=77,
	REG_UEP11=78,
	REG_UEP10=79,
	REG_UEP9=80,
	REG_UEP8=81,
	REG_UEP7=82,
	REG_UEP6=83,
	REG_UEP5=84,
	REG_UEP4=85,
	REG_UEP3=86,
	REG_UEP2=87,
	REG_UEP1=88,
	REG_UEP0=89,
	REG_UCFG=90,
	REG_UADDR=91,
	REG_UCON=92,
	REG_USTAT=93,
	REG_UEIE=94,
	REG_UEIR=95,
	REG_UIE=96,
	REG_UIR=97,
	REG_UFRMH=98,
	REG_UFRML=99,
	REG_INTCON=100,
	REG_INTCON2=101,
	REG_INTCON3=102,
	LAST_REGISTER_CODE=103
};//enum REGS

#ifdef	REG_STRARR
const char *reg_strings[] = {
	"INDF2",//1
	"POSTINC2",//2
	"POSTDEC2",//3
	"PREINC2",//4
	"PLUSW2",//5
	"FSR2H",//6
	"FSR2L",//7
	"STATUS",//8
	"TMR0H",//9
	"TMR0L",//10
	"T0CON",//11
	"OSCCON",//12
	"HLVDCON",//13
	"WDTCON",//14
	"RCON",//15
	"TMR1H",//16
	"TMR1L",//17
	"T1CON",//18
	"TMR2",//19
	"PR2",//20
	"T2CON",//21
	"SSPBUF",//22
	"SSPADD",//23
	"SSPSTAT",//24
	"SSPCON1",//25
	"SSPCON2",//26
	"ADRESH",//27
	"ADRESL",//28
	"ADCON0",//29
	"ADCON1",//30
	"ADCON2",//31
	"CCPR1H",//32
	"CCPR1L",//33
	"CCP1CON",//34
	"CCPR2H",//35
	"CCPR2L",//36
	"CCP2CON",//37
	"BAUDCON",//38
	"ECCP1DEL",//39
	"ECCP1AS",//40
	"CVRCON",//41
	"CMCON",//42
	"TMR3H",//43
	"TMR3L",//44
	"T3CON",//45
	"SPBRGH",//46
	"SPBRG",//47
	"RCREG",//48
	"TXREG",//49
	"TXSTA",//50
	"RCSTA",//51
	"EEADR",//52
	"EEDATA",//53
	"EECON2",//54
	"EECON1",//55
	"IPR2",//56
	"PIR2",//57
	"PIE2",//58
	"IPR1",//59
	"PIR1",//60
	"PIE1",//61
	"OSCTUNE",//62
	"TRISC",//63
	"TRISB",//64
	"TRISA",//65
	"LATC",//66
	"LATB",//67
	"LATA",//68
	"PORTE",//69
	"PORTD",//70
	"PORTC",//71
	"PORTB",//72
	"PORTA",//73
	"UEP15",//74
	"UEP14",//75
	"UEP13",//76
	"UEP12",//77
	"UEP11",//78
	"UEP10",//79
	"UEP9",//80
	"UEP8",//81
	"UEP7",//82
	"UEP6",//83
	"UEP5",//84
	"UEP4",//85
	"UEP3",//86
	"UEP2",//87
	"UEP1",//88
	"UEP0",//89
	"UCFG",//90
	"UADDR",//91
	"UCON",//92
	"USTAT",//93
	"UEIE",//94
	"UEIR",//95
	"UIE",//96
	"UIR",//97
	"UFRMH",//98
	"UFRML",//99
	"INTCON",//100
	"INTCON2",//101
	"INTCON3"//102
};//reg_strings
#endif

#endif//RC2_SAMPLE_H
