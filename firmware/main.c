// $Id: main.c,v 1.85 2012/03/26 16:36:26 dmi Exp $
/*
Causes compiler syntax error on newer USB stack 2.7
#include "Compiler.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "USB/usb_device.h"
#include "USB/usb.h"
#include "USB/usb_function_generic.h"
#include "usb_config.h"
*/

/*
 Timer assignment
 
 Timer0 TMR0H and TMR0L - real time clock source via "compute_clock_counter"
 Timer2 generates buzzer tone with CCP1
 Timer3 used to blink leds with "short_timeout" and "long_timeout"
 
 Timer1 is planned to be used to drive 8 additional buzzers in quadro-parkass
*/

#include "GenericTypeDefs.h"
#include "usb_config.h"
#include "USB/usb_common.h"
#include "Compiler.h"
#include "USB/usb_ch9.h"
#include "USB/usb_device.h"
#include "USB/usb_hal_pic18.h"
#include "USB/usb_function_generic.h"
#include "HardwareProfile.h"

/*
#define	set_bit(ref,b) 		((ref|=(1<<b))
#define clear_bit(ref,b)	(ref&=~(1<<b))
#define	get_bit(ref,b)		((ref&(1<<b))?1:0)
*/

#define	set_bit(ref,b) 		(ref|=(1<<b))
#define clear_bit(ref,b)	(ref&=~(1<<b))

unsigned char get_bit(unsigned char ref, unsigned char b)
{
	if(b==0) return ref & 0x1;
	return ((ref&(1<<b))?1:0);
}

#if	defined(USE_I2C)
#include "i2c.h"
#endif

#include	"../Bootloader/src/settings.h"

/* --- OLD SETTINGS

// OFF -CCP2 input/output is multiplexed with RB3
// ON - CCP2 input/output is multiplexed with RC1
#pragma config CCP2MX	= OFF
// PICEOSHIT IS BROKEN IN HARDWARE.
//

#pragma config PLLDIV   = 2         // (20 MHz crystal on PICDEM FS USB board)
#pragma config CPUDIV   = OSC1_PLL2
#pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
#pragma config FOSC     = HSPLL_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOR      = ON
#pragma config BORV     = 3

// Disable for Self-Power 3.3v
#pragma config VREGEN   = ON      //USB Voltage Regulator

#pragma config WDT      = OFF
#pragma config WDTPS    = 32768
#pragma config LPT1OSC  = OFF

// Disable analog functions on PORTB on reset
#pragma config PBADEN   = OFF

// reset on stack overflo
#pragma config STVREN   = ON

#pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
#pragma config XINST    = OFF       // Extended Instruction Set

//
// Modifying code with bootloader can cause system erase
// Code protection
//
#pragma config MCLRE    = ON

// Code protection
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CP2      = OFF
#pragma config CP3      = OFF

// Boot block code protection
#pragma config CPB      = OFF

#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF

#pragma config WRTB     = ON       // Boot Block Write Protection
#pragma config WRTC     = OFF	   // Configuration register

//      #pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF
----------------------- === OLD SETTINGS === */

/** PRIVATE PROTOTYPES *********************************************/
static void InitializeSystem(void);
void USBDeviceTasks(void);
void HighPriorityInterruptHandler(void);
void LowPriorityInterruptHandler(void);
void UserInit(void);
void ProcessIO(void);
void ProcessCommands(void);

void recordDataPortSamplingInterrupt(void);
void recordPollUpdateIntoChallenger(void);
unsigned long getTimeOfLastChange(void);

void stop_interrupts(void);
void start_interrupts(void);
void timer0_stop(void);
void timer0_start(void);
void fatal_error(void);

void standalone_stop(void);
void standalone_start(void);

void poll_data_pin(void);

void pio_get_version(void);
void pio_ping(void);
#ifndef	MULTI_BUZZER
void pio_read_buzzer(void);
#endif
void pio_read_shift(void);
void pio_get_thermal(void);

unsigned char read_EEPROM(unsigned int);

void pio_reset(void);
void pio_wait_for_inhandle(void);

void ioc_start(void);
void ioc_stop(void);

#if	defined(USE_I2C)
void process_i2c_interrupt(void);
void open_i2c(void);
void close_i2c(void);
#endif

void makeStatusDataPacket(void);
void add_ch26data_bit(unsigned char);

#ifndef	MULTI_BUZZER
void setup_buzzer(void);
void buzzer_on(void);
void buzzer_off(void);
#endif

void grabMinDistance(void);

#ifndef	MULTI_BUZZER
void buzzer_distance(void);
void buzzer_wait_for_5beeps(void);
void save_buzzer(unsigned char arr[]);
#endif

void save_shift(unsigned char arr[]);

void stop_timer3(void);
void start_timer3(void);
void timer3_interrupt(void);

void short_timeout(void);
void long_timeout(void);

//void set_state(unsigned char);

#ifndef	MULTI_BUZZER
#define buzzer_status() T2CONbits.TMR2ON
#define	buzzer_3beeps() { ledS.buzzerCounter=3; }
#endif

// Counts number of cycles passed from the beginning of measurement
unsigned long sampling_clock_counter = 0;	//var

#include	"rc2sample.h"


////////////////////////////////////// CHALLANGER26 // START
#include "challenger26.h"

// Used functions prototypes
void init_BuzzDevs(void);
int set_EEPROM(unsigned int addr,unsigned char val);
void Delay100TCYx ( unsigned char unit );

unsigned char widthSilence(unsigned long l);
unsigned char widthDataTimeout(unsigned long l);
void initDistData(struct DistData*);
void initChallengerLine(struct ChallengerLine*);

unsigned char widthLow(unsigned long l);
unsigned char isNoise(unsigned long l);

int detectImpulse(unsigned long l, unsigned long h);
unsigned char writeToChallengerData(
	struct ChallengerLine* c,
	unsigned long l,
	unsigned long h);
int isValidChallengerPacket(const struct ChallengerLine* c);
void processChallengerPacket(const struct ChallengerLine* c);

// LEDS
// FSM
#define ANIMATION_ALL_3SHORT 		0x1
#define	ANIMATION_ALL_2SHORT_LONG 	0x2

void led_animate(unsigned char code);
void init_leds(void);
void led_set(char led,char status);

////////////////////////////////////// CHALLENGER26 // END
/*
	Signalling status with LEDs

	Bootloader - RED

	Power on - GREEN

	Fatal error - All LEDs long blink

	PING - YELLOW/GREEN/RED short-short-short
	VERSION - YELLOW/GREEN/RED short-short-long

	RB4 activity - RED short blink
	RB5 activity - YELLOW short blink

	FEEDING SAMPLE data - GREEN short blink
	FEEDING FORMATTED data - GREEN long blink
*/

#define	T3_PERIOD_SHORT 2
#define	T3_PERIOD_LONG  12

#define DATACOUNT_LIMIT 10
#define	DATACOUNT_STEP	3

#define	lOFF	0b00
#define	lSHORT	0b01
#define	lLONG	0b10
#define	lON	0b11

#define	LED_WHITE	0b001
#define	LED_GREEN	0b010
#define	LED_RED		0b100

#define	colorOn(b) 	_clrOn(LED_##b)
#define	colorOff(b)	_clrOff(LED_##b)

void _clrOn(unsigned char c)
{
	PORTA|=c;
}

void _clrOff(unsigned char c)
{
	PORTA&=~c;
}

struct SignalLed {
	unsigned char s: 1;
	unsigned char l: 1;
	unsigned char cnt : 4; // 0-15

	unsigned char red: 2;
	unsigned char green: 2;
	unsigned char white: 2;

	unsigned char bkRed: 2;
	unsigned char bkGreen: 2;
	unsigned char bkWhite: 2;

	#ifndef	MULTI_BUZZER
	unsigned char buzzerAlwaysOn: 1;
	unsigned char buzzerPeriod:5;
	// countdown of beeps if no buzzerPeriod set
	// countdown of delay if buzzerPeriod set
	unsigned char buzzerCounter:5;
	#endif
	
	// Finate state machine mechanism
	unsigned char animation_code: 3; // 0-7
	unsigned char animation_count: 4; // 0-15

	// That will be changed by short interrupt
	unsigned char countRB4:2;
	unsigned char countRB5:2;

	unsigned char dataCountRB4;
	unsigned char dataCountRB5;

	unsigned char minDistance;

	unsigned char usbOUTtimeout;
	} ledS;

// LEDS - end
struct Quant {
	unsigned char q_data;
	unsigned long q_time;
	};

#define	BITFLAG(a)	unsigned char a: 1;

struct _flags {
	BITFLAG(get_version)
	BITFLAG(get_thermal)
	BITFLAG(reset)
	BITFLAG(standalone)
	// stop_reading will also clear scanner state
	BITFLAG(stop_reading)
	// read_parktronic is one time command, that will initiate reading process
	BITFLAG(read_parktronic)
	// Scanner flag indicates that we are processing the
	// data without sending samples out. It's only the flag
	BITFLAG(scanner)
#if	defined(USE_I2C)
	BITFLAG(open_i2c)
	BITFLAG(close_i2c)
#endif
	BITFLAG(get_io_registers1)
	BITFLAG(get_io_registers2)
	BITFLAG(ping)
#ifndef	MULTI_BUZZER	
	BITFLAG(buzzer)
#endif	
	BITFLAG(shift)
	BITFLAG(data_packet)
	// Flag indicates no data currently arrives
	BITFLAG(no_feed)
	};

#define	BITFLAG1(a)	flags.a=1;
#define	BITFLAG0(a)	flags.a=0;

//This return will be a "retfie fast", since this is in a #pragma interrupt section
struct _flags flags;	//var
	
#ifdef	MULTI_BUZZER
void generate_carry_pwm();
void generate_carry_pwm()
{
	T2CONbits.TMR2ON=0;
	
	T2CONbits.T2CKPS1=1;
	T2CONbits.T2CKPS0=0;//TMR2 prescale: 16

	// Target for 2.5Khz carrying pwm 49:16
	PR2=255;
	
	// T2OUTPS Poscatscale 1111=1:16 0001=1:2 0000=1:1
	T2CONbits.T2OUTPS3=0;
	T2CONbits.T2OUTPS2=0;
	T2CONbits.T2OUTPS1=0;
	T2CONbits.T2OUTPS0=0;

	// 100 gives 50% duty
	
	// duty cycle control
	CCPR1L=110; 
	CCP1CONbits.DC1B0=0;
	CCP1CONbits.DC1B1=0;

	// Start CCP1, module configuration
	TRISCbits.TRISC2=0; // CCP1

	// CCPxM3:CCPxM0 11xx for PWM
	CCP1CONbits.CCP1M3=1;
	CCP1CONbits.CCP1M2=1;
	CCP1CONbits.CCP1M1=0;
	CCP1CONbits.CCP1M0=1;
	
	// Generating timer 2 on
	T2CONbits.TMR2ON=1;
}

#ifdef	MULTI_BUZZER

// 
// Datastructures for multi-buzzer support
//

// Configure pins for driving extrenal output
// RC1, RC0, RA5, RB2, RB1, RB0, RC7, RC6

// HCM1206A max loud: 2520Hz, 395us period

// 5.3KHz TMR1H=0xFA;TMR1L=0x00;
// 4.3KHz TMR1H=0xF8;TMR1L=0x00;
// 3.1KHz TMR1H=0xF4;TMR1L=0x00;
// 2.5Khz TMR1H=0xF0;TMR1L=0x00;
// 2.1Khz TMR1H=0xEE;TMR1L=0x00;
// TMR1H=0xFA;TMR1L=0x00; -- max loud for HCM1206A on 1:2
// #define	TIMER1_SET_COUNTERS TMR1H=0x80;TMR1L=0x00;

#define	TRIS0	TRISCbits.TRISC1
#define	TRIS1	TRISCbits.TRISC0
#define	TRIS2	TRISAbits.TRISA5
#define	TRIS3	TRISBbits.TRISB2
#define	TRIS4	TRISBbits.TRISB1
#define	TRIS5	TRISBbits.TRISB0
#define	TRIS6	TRISCbits.TRISC7
#define	TRIS7	TRISCbits.TRISC6

/* Debug times only
#define	PIN0	PORTCbits.RC1
#define	PIN1	PORTCbits.RC0
#define	PIN2	PORTAbits.RA5
#define	PIN3	PORTBbits.RB2
#define	PIN4	PORTBbits.RB1
#define	PIN5	PORTBbits.RB0
#define	PIN6	PORTCbits.RC7
#define	PIN7	PORTCbits.RC6
*/
#define	ADDR_PORTC	((unsigned char*)(0xF82))
#define	ADDR_PORTB	((unsigned char*)(0xF81))
#define	ADDR_PORTA	((unsigned char*)(0xF80))
struct BuzzDev {

	//
	// SOUND GENERATION PART
	//

	unsigned char* portAddr;
	unsigned char bitA;
	unsigned char rBitA;
	
	//
	// DISTANCE TIMER
	//
			
	unsigned int pause; // How many clocks pass between beeps	
	unsigned int cycles;	
	//
	unsigned char id;
	unsigned char prevDist;
	} buzzdevs[] = {
		/* Second parameter must be 2x to first: widht = load*2 for 50% PWM */
		//////////////////////////////////////////////////////////////////////
		{ADDR_PORTC,0b10,0},  // RC1
		{ADDR_PORTC,0b1,0},	// RC0
		{ADDR_PORTA,0b100000,0},	// RA5
		{ADDR_PORTB,0b100,0},	// RB2
		{ADDR_PORTB,0b10,0},	// RB1
		{ADDR_PORTB,0b1,0},	// RB0
		{ADDR_PORTC,0b10000000,0},	// RC7
		{ADDR_PORTC,0b1000000,0},	// RC6
		{NULL,0,0}
		};

#define	signalBD(d)	(*d->portAddr&d->bitA)
#define	highBD(d)	*d->portAddr|=d->bitA
#define	lowBD(d)	*d->portAddr&=d->rBitA

void start_timer1(void);
void stop_timer1(void);
#endif

// Number of cycles in the beep
#define	BEEP_DURATION	2
struct BuzzDev* shortListBD[9]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

void setBuzzDevDistance(unsigned char id, unsigned char dist)
{
	int idx=0;
	struct BuzzDev* d = &buzzdevs[id];
	// If distance has not changed, go on
	if(d->prevDist==dist) return;
	d->prevDist=dist;
	
	if(dist<30) {	
		// Turn buzzer on
		highBD(d);
		// Disengage all timer-based beeping
		d->cycles=0;
		d->pause=0;		
	}
	else if(dist<26) { d->pause=1; }
	else if(dist<35) { d->pause=2; }
	else if(dist<45) { d->pause=3; }
	else if(dist<55) { d->pause=5; }
	else if(dist<65) { d->pause=7; }
	else if(dist<75) { d->pause=10; }
	else if(dist<85) { d->pause=12; }
	else if(dist<100) { d->pause=14; }
	else if(dist<120) { d->pause=18; }
	else if(dist<140) { d->pause=24; }
	else if(dist<160) { d->pause=26; }
	else if(dist<180) { d->pause=30; }
	else if(dist<200) { d->pause=40; }
	else {
		lowBD(d);
		d->pause=0;
		}
	
	// Re-compute short list of buzzers
	for(d=buzzdevs,idx=0; d->portAddr!=NULL; d++) {
		shortListBD[idx]=NULL;
		if(d->pause) {
			shortListBD[idx++]=d;
			shortListBD[idx]=NULL;
			}
		}
		
	if(idx) start_timer1(); else stop_timer1();
}

void init_BuzzDevs()
{
	unsigned char i=0;
	struct BuzzDev* d=buzzdevs;

	// Make outputs from pins
	TRIS0=0;
	TRIS1=0;
	TRIS2=0;
	TRIS3=0;
	TRIS4=0;
	TRIS5=0;
	TRIS6=0;
	TRIS7=0;
	
	for(i=0;d->portAddr!=NULL;d++,i++) {
		d->id=i;
		d->rBitA=~d->bitA;
		// initialize pin to logic zero to avoid leakage
		lowBD(d);
		d->pause=0;
		d->cycles=0;
		d->prevDist=0;
		}
}
#endif // MULTI_BUZZER

//
// Size of that structure must be 64 bytes,
// or less.
//
struct RC2MeasurementUSB {

	// Number of impulses it holds (instead of 0-terminated array)
	unsigned char code;//COMMAND_READ_DATA_PACKET
	unsigned char size;
	struct Quant deckCycles[USB_PACKET_SIZE];

	unsigned char uData[2];
};

union _m_send2host {
	// Raw data
	char _version_buf[64];
	// Raw sampling data
	struct RC2MeasurementUSB _usbStat;
	};

struct RC2Measurement {
	// Counters
	struct Quant deckCycles[STORAGE_PACKET_SIZE];
};

#pragma code

union clkdata {
	struct s2byte {
		unsigned char b0, b1;
		} s2byte;
	unsigned short clk_count;
	};

#define	compute_clock_counter()	\
	timer_clkdata.s2byte.b1=TMR0H;	\
	timer_clkdata.s2byte.b0=TMR0L;	\
	sampling_clock_counter += (timer_clkdata.clk_count-timer_prev_clk_count);	\
	timer_prev_clk_count=timer_clkdata.clk_count;

// How many parktronic interfaces can stick up on the controller
// RB7, RB6, RB5, RB4
#define	NUMBER_OF_CHALLENGER_DEVICES	2

struct ChallengerData {
	// Both of those are set by user by issuing COMMAND_DATA_FORMAT
	unsigned char usedBitMask;
	unsigned char usedBitCount;
	unsigned char bit2line[8];
	struct ChallengerLine line[NUMBER_OF_CHALLENGER_DEVICES];
	};

void initChallengerData(struct ChallengerData* c);

void initChallengerData(struct ChallengerData* c)
{
	unsigned char i =0;

	c->usedBitCount=0;
	c->usedBitMask=0;
	for(i=0;i<8;i++) c->bit2line[i]=0xFF; // mark as unused
	for(i=0;i<NUMBER_OF_CHALLENGER_DEVICES;i++) {
		initChallengerLine(&c->line[i]);
		}
}

/** VARIABLES ******************************************************/

// flags.scanner dictates what approach to choose
union dataCollection {
	struct RC2Measurement	__usbPackage;	//var
	struct ChallengerData __ch26data;
	} _dc;

#define	usbPackage _dc.__usbPackage
#define ch26data _dc.__ch26data

union clkdata timer_clkdata;
unsigned short timer_prev_clk_count=0;

struct Quant* headp1=NULL;

unsigned char poll=0x0, prev_poll=0xFF;

struct Quant	*package_start=NULL, *package_end=NULL;
struct Quant 	*headp=NULL, *tailp=NULL;

#ifdef	__DEBUG
unsigned long lost=0, unknown_calls=0, calls=0;
#endif

#pragma udata USB_VARIABLES=0x500

// User application buffer for receiving and holding
// OUT packets sent from the host
unsigned char OUTPacket[HOST_COMMAND_SIZE];
USB_HANDLE USBGenericOutHandle;
USB_HANDLE USBGenericInHandle;

#pragma code

union	_m_send2host m_send2host;


/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	//On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
	//the reset, high priority interrupt, and low priority interrupt
	//vectors.  However, the current Microchip USB bootloader
	//examples are intended to occupy addresses 0x00-0x7FF or
	//0x00-0xFFF depending on which bootloader is used.  Therefore,
	//the bootloader code remaps these vectors to new locations
	//as indicated below.  This remapping is only necessary if you
	//wish to program the hex file generated from this project with
	//the USB bootloader.  If no bootloader is used, edit the
	//usb_config.h file and comment out the following defines:
	//#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
	//#define PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER

	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
	#else
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
	#endif

	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);        // See c018i.c in your C18 compiler dir
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
	    _asm goto _startup _endasm
	}
	#endif
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
	     _asm goto HighPriorityInterruptHandler _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto LowPriorityInterruptHandler _endasm
	}

	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	//Note: If this project is built while one of the bootloaders has
	//been defined, but then the output hex file is not programmed with
	//the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
	//As a result, if an actual interrupt was enabled and occured, the PC would jump
	//to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
	//executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
	//(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
	//would effective reset the application.

	//To fix this situation, we should always deliberately place a
	//"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
	//"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
	//hex file of this project is programmed with the bootloader, these sections do not
	//get bootloaded (as they overlap the bootloader space).  If the output hex file is not
	//programmed using the bootloader, then the below goto instructions do get programmed,
	//and the hex file still works like normal.  The below section is only required to fix this
	//scenario.
	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
	     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
	     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"
	
	#pragma interruptlow LowPriorityInterruptHandler
	void LowPriorityInterruptHandler()
	{
	
	unsigned long deltaTime = 0;
	
	compute_clock_counter();

	if(PORTB_IRPT_ON_CHANGE_FLAG) {
		// ////////////////////////////////////////////////////////////////////
		// WARNING
		// Without reading the port, IO queue will lock up
		//
		poll=DATA_PORT; // End the mismatch condition, to be able to clear
		//
		// ////////////////////////////////////////////////////////////////////

		//PORTCbits.RC1=((poll & 0x10)!=0);
		//PORTCbits.RC0=((poll & 0x80)!=0);

		if(poll!=prev_poll) {

			BITFLAG0(no_feed);

			// Record change into the polling line
			if(!flags.scanner) recordDataPortSamplingInterrupt();

			else recordPollUpdateIntoChallenger();

			prev_poll=poll;

			} else {
				// All lines been shut. Fire a full timeout packet
				deltaTime = sampling_clock_counter-getTimeOfLastChange();

				if(widthDataTimeout(deltaTime)) {

					BITFLAG1(no_feed)
					}
				}
		PORTB_IRPT_ON_CHANGE_FLAG=0;
		// Minimize time spent in the interrupt handler
		return;
		} else {
				// All lines been shut. Fire a full timeout packet
				// Second check (on timer pretty much)
				deltaTime = sampling_clock_counter-getTimeOfLastChange();

				if(widthDataTimeout(deltaTime)) {

					BITFLAG1(no_feed)
					}
				}

	if(TIMER3_FLAG) {
		timer3_interrupt();
		TIMER3_FLAG=0;
		return;
		}
		
#if	defined(__DEBUG)
		unknown_calls ++;
#endif
	}	//This return will be a "retfie", since this is in a #pragma interruptlow section

#elif defined(__C30__)
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
        /*
         *	ISR JUMP TABLE
         *
         *	It is necessary to define jump table as a function because C30 will
         *	not store 24-bit wide values in program memory as variables.
         *
         *	This function should be stored at an address where the goto instructions
         *	line up with the remapped vectors from the bootloader's linker script.
         *
         *  For more information about how to remap the interrupt vectors,
         *  please refer to AN1157.  An example is provided below for the T2
         *  interrupt with a bootloader ending at address 0x1400
         */
//        void __attribute__ ((address(0x1404))) ISRTable(){
//
//        	asm("reset"); //reset instruction to prevent runaway code
//        	asm("goto %0"::"i"(&_T2Interrupt));  //T2Interrupt's address
//        }
    #endif
#endif //of "#if defined(__18CXX)"




/** DECLARATIONS ***************************************************/
#pragma code

/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void)
{
        ADCON1 |= 0x0F;                 // Default all pins to digital
        // ALL PORTS TO OUTPUT
        TRISA=0x0;
        TRISB=0x0;
        TRISC=0x0;
        PORTA=0xFF;
        PORTB=0xFF;
        PORTC=0xFF;

	// Initialize thermal bus
	DS1820_DATAPIN = 1;

//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.  Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
// 	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
// 	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.  This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	HardwareProfile.h file.
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif

//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.  If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2"
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in HardwareProfile.h, and that an appropriate I/O pin has been mapped
//	to it in HardwareProfile.h.
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
    #endif

	USBGenericOutHandle = 0;
	USBGenericInHandle = 0;

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
    UserInit();			//Application related initialization.  See user.c

}//end InitializeSystem

void trigger_bootloader_after_reset(void)
{
	//write Byte "1" back to a 1
   	EEADR =0x0001;
   	EEDATA =0x0; // -- to bypass bootloader, overwrite with 0x01;
   	Nop();
   	Nop();
	EECON1bits.EEPGD = 0;
   	EECON1bits.CFGS = 0;
   	EECON1bits.WREN = 1;
   	INTCONbits.GIE = 0;
   	EECON2 = 0x55;
   	EECON2 = 0xAA;
   	EECON1bits.WR = 1;
	while(EECON1bits.WR);
   	INTCONbits.GIE = 1;
   	EECON1bits.WREN = 0;
}

//
// To be called only once inside InitializeSystem
//

void UserInit(void)
{
	unsigned char i=0;

	// ================= Novorado custom init ======
	TRISA=0x0; Nop();Nop();Nop(); PORTA=0x0;

	//set_state(ST_UNKNOWN);

	// Prevent from "bad powering"
	LOW_SIG_PULLUP_DISABLE=1;
	// =============================================

	poll=0x1;
	prev_poll=0x2;

	tailp=NULL;

	/*
	RCON=0;
	INTCON=0;
	INTCON2=0;
	INTCON3=0;
	PIR1=0; PIR2=0;
	PIE1=0; PIE2=0;
	IPR1=0; IPR2=0;
	*/

	//enable priority interrupts
	RCONbits.IPEN = 1;

	BITFLAG0(get_version)
	BITFLAG0(get_thermal)
	BITFLAG0(standalone)
	BITFLAG0(reset)
	BITFLAG0(stop_reading)
	BITFLAG0(read_parktronic)
	BITFLAG0(scanner)
	// No feed initially
	BITFLAG1(no_feed)
#if	defined(USE_I2C)
	BITFLAG0(open_i2c)
	BITFLAG0(close_i2c)
#endif
	BITFLAG0(get_io_registers1)
	BITFLAG0(get_io_registers2)
	BITFLAG0(ping);
#ifndef	MULTI_BUZZER	
	BITFLAG0(buzzer);
#endif
	BITFLAG0(shift);
	BITFLAG0(data_packet);

	usbStat.size=0;

	//flags.startedCollecting=0;
	usbStat.size=0;

	/*
	TRISB=0x0; // all outputs
	*/

	TRISBbits.TRISB4=1;
	TRISBbits.TRISB5=1;

	// ---------------- Finishing Novorado initialization
	init_leds();
	start_timer3();

#ifndef	MULTI_BUZZER
	setup_buzzer();
#endif
	standalone_start();	
	//set_state(ST_JUST_INITIALIZED);
#ifdef	MULTI_BUZZER	
	generate_carry_pwm();
	
	// !!! WARNING !!!
	// Initialize buzzers only after standalone
	// mode has been started, it will initialize 
	// challenger data structures
	init_BuzzDevs();
	start_timer1();
	
	// Timer1 should engage in standalone mode only
	// -- start timer1 on incoming feed and standlone mode only
	// !!!!!!!!!!!!!!!
#endif	
}//end UserInit


void stop_interrupts()
{
	ioc_stop();
	timer0_stop();
}

#define	CHECK_MSDP_OVERFLOW if(posW>=sizeof(version_buf)) { flagError=1; break;} version_buf[posW++] 

void makeStatusDataPacket()
{
	unsigned char temp = 0;
	unsigned char posW = 0; // can't be greater than 64 in this extent
	unsigned char headCnt = 0;
	unsigned char headId = 0;
	unsigned char flagError = 0;

	struct ChallengerLine* thing = NULL;

	// Initiate packet
	// STRUCTURE 1: command code
	version_buf[posW++]=CMD_DATA_FORMAT; // specific code mark, so user app knows its getting formatted data

	// STRUCTURE 2: count of data filled
	version_buf[posW++]=ch26data.usedBitCount; // number of protocol records, size

	for(temp=0;temp<ch26data.usedBitCount;temp++){
		thing = &ch26data.line[temp];

		// Here we need to pack ChallengerLine info into the raw array
		// STRUCTURE 2 + 1: line id
		CHECK_MSDP_OVERFLOW=thing->lineId;

		////////////////////////////////////////////////////////
		// Sending an idle signal
		if(thing->signalTimeout){
			// ---------------------------------------------------------------------
			CHECK_MSDP_OVERFLOW=CMD_DATA_TIMEOUT; // Writing data timeout command
			// instead of number of heads
			// ---------------------------------------------------------------------

			continue;
			}
		// SELECTED LINE SIGNAL TIMEOUT
		/////////////////////////////////////////////////////////

		// STRUCTURE 2 + 2: number of heads used
		// Write number of heads used
		CHECK_MSDP_OVERFLOW = thing->usedHeadsCount;

		// Write distances in the form head id -> distance
		for(headCnt=0;headCnt<thing->usedHeadsCount;headCnt++){

			headId = thing->dist[headCnt].headId;

			// Structure 2 + 2 + 1: head id
			CHECK_MSDP_OVERFLOW = headId;// head id

			// Structure 2 + 2 + 2: distance
			CHECK_MSDP_OVERFLOW = thing->dist[headCnt].dist;// head distance
			}
		}

	if(flagError){
		version_buf[0]=CMD_DEVICE_ERROR;
		version_buf[1]=DEVICE_ERROR_MSDP;
		}

	// Size for case of 3x 8-head parktronic
	// [1] cmd + [1] size + ( [1] lineId + [1] usedHeads + ([1]headId[1]dist)*8 ) * 3
	// 2 + ( 2 + 16 ) * 3 =  2 + 18 * 3 = 56 bit. Still livable, for the 3 parktronic devs on line
	// When get bigger pack, will need to send more than 64 bit packet

	// if pos gets bigger than 64, it is a fatal error. it will overflow USB space variables
	// ineffective -- if(pos>=64) break_on_me();

	// Send data to the R-questor.
	// usbState must be untouched until next USBHandleBusy returned;
	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&usbStat),64);
}

// Prototype
unsigned long reverse_byte_order(unsigned long l);

unsigned long reverse_byte_order(unsigned long l){

	unsigned char tmp;
	unsigned long rv=l;
	unsigned char* arr=(unsigned char*)&rv;

	// Exchange 1st and last
	tmp=arr[0];
	arr[0]=arr[3];
	arr[3]=tmp;

	// Exchange second and third
	tmp=arr[1];
	arr[1]=arr[2];
	arr[2]=tmp;

	return rv;
	}

void make_in_packet()
{
	struct Quant* walkp, *endp, *startp;
	int num_impulses=0;

	if(tailp==NULL) return; // no data yet
	else if(tailp==headp) {
		tailp=NULL;
		return;
		}

	/*
		Number of impulses
		head>tail: head-tail
		head<tail: (NUM_IMPULSES-1-tail)+(head+1)=NUM_IMPULSES+head-tail
	*/
	if(headp!=tailp) {
		num_impulses=(headp-tailp);
		if(num_impulses<0) num_impulses += STORAGE_PACKET_SIZE;

		//if(num_impulses<USB_PACKET_SIZE) return;
	}

	usbStat.code=CMD_READ_DATA_PACKET;
	usbStat.size=0;

	walkp=startp=&usbStat.deckCycles[0];
	endp=&usbStat.deckCycles[USB_PACKET_SIZE-1];

	while(num_impulses){

		// q_data is unsigned char
		walkp->q_data=tailp->q_data;
		// q_time is unsigned long; change byte order
		walkp->q_time=tailp->q_time;//reverse_byte_order(tailp->q_time);

		usbStat.size++;

		if(++tailp>package_end) tailp=package_start;

		if(tailp==headp) {
			tailp=NULL;
			break;
			}

		if(++walkp>endp) break;
		}

	// usbState must be untouched until next USBHandleBusy returned;
	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&usbStat),64);
}

#define bit_is_set(b, i) \
     (b & (1 << (i)))

#define bit_is_clear(b, i) \
     (!bit_is_set(b, i))

/*
#define	WSZ 16
unsigned long wsbuf[WSZ];
unsigned char wscnt=0;
*/

unsigned long getTimeOfLastChange()
{
	unsigned long rv=0;
	unsigned char bitNum = 0;
	unsigned char bitByte = 0;

	struct ChallengerLine* thing = NULL;

//	enum T_IMPULSE impulseKind = TI_INVALID;

	// Cycle trough RB7-RB4 (interrupt pins that connect to 18F2550 Interrupt-on-change IOC feature)
	for(bitNum=7;bitNum>=4;bitNum--){

		bitByte = (1 << bitNum);

		if(bitByte & ch26data.usedBitMask) {

			// Signal change detected on  with bitNum (on RB7, RB6, RB5 or RB4)

			thing = &ch26data.line[ch26data.bit2line[bitNum]];

			if(rv<thing->timeOfLastChange) rv=thing->timeOfLastChange;
			}
		}

	return rv;
}

void recordPollUpdateIntoChallenger() {

	unsigned char bitNum = 0;
	unsigned char bitByte = 0;

	unsigned char currentBitState = 0;
	unsigned char prevBitState = 0;

	struct ChallengerLine* thing = NULL;

	unsigned long deltaTime = 0;

//	enum T_IMPULSE impulseKind = TI_INVALID;

	// Cycle trough RB7-RB4 (interrupt pins that connect to 18F2550 Interrupt-on-change IOC feature)
	for(bitNum=7;bitNum>=4;bitNum--){

		// Mark bit with given num
		bitByte = (1 << bitNum);

		// Find if it has a line connected
		if(bitByte & ch26data.usedBitMask) {

			// Check if this given bit has changed
			currentBitState = (poll & bitByte);
			prevBitState = (prev_poll & bitByte);

			// Compare poll to previous poll state, to find out if bit has changed
			if(  currentBitState != prevBitState ) {

				// Signal change detected on  with bitNum (on RB7, RB6, RB5 or RB4)
				BITFLAG0(no_feed);

				thing = &ch26data.line[ch26data.bit2line[bitNum]];

				// Lined Id: bitNum (7,6,5,4)
				// Bit mask for bit LineId: bitByte.
				// Event time: sampling_clock_counter
				// Previous event time: ch26data.v[linearSearchCnt].timeOfLastChange
				deltaTime = sampling_clock_counter-thing->timeOfLastChange;

				// Challenger protocol is sensitive to noise errors
				if(isNoise(deltaTime)) continue;

				// Store time of the current event
				thing->timeOfLastChange=sampling_clock_counter;

				//
				// Silence between impulses, start over
				//
				if(currentBitState /*low->high switch*/ && widthSilence(deltaTime)) {
					// We started the package
					thing->bitCount=0;
					thing->prevInterval=0;
					// bitNum break
					continue;
					}

				//
				//
				//
				// ____________|---|___|------|_____
				//             A   B   A'      B

				if(currentBitState) {

					// Very first A. Do nothing
					if(thing->bitCount==0 && thing->prevInterval==0) {
						// bitNum break
`						continue;
						}

					/*
					if(wscnt>=WSZ) {
						wscnt=0;
						}
					wsbuf[wscnt++]=thing->prevInterval;
					if(wscnt>=WSZ) {
						wscnt=0;
						}
					wsbuf[wscnt++]=deltaTime;
					*/

					// An A'. Recognize the bit
					if(writeToChallengerData(thing,thing->prevInterval,deltaTime)>=24){
						// It looks we got a complete challenger packet.
						// Let's find out if it is a valid one
						if(isValidChallengerPacket(thing)) {
							// Update the distances table
							processChallengerPacket(thing);

							if(bitNum==4){
								if(ledS.dataCountRB4<DATACOUNT_LIMIT) ledS.dataCountRB4+=DATACOUNT_STEP;
								} else if(bitNum==5){
									if(ledS.dataCountRB5<DATACOUNT_LIMIT) ledS.dataCountRB5+=DATACOUNT_STEP;
									}
							}

						// Impulse recorded. It is not relevant to store that
						// information
						thing->prevInterval=0;

						// On the error event, writeToChallengerData will clear the structs
						continue;
						}
					} else {

						thing->prevInterval=deltaTime;
`						continue;
						}
				}
			}
		}
	}

void recordDataPortSamplingInterrupt(void)
{
	if(headp!=tailp) {

		if(tailp==NULL) {
			headp=&usbPackage.deckCycles[0];
			tailp=headp;
			}

		headp->q_data=poll;
		headp->q_time=sampling_clock_counter;

		headp1=headp;

		headp++;

		if(headp>package_end) headp=package_start;

		if(headp==tailp) {
			headp=headp1;

// Indicate overflow
//			PORTCbits.RC2=1;

#if	defined(__DEBUG)
			lost ++;
#endif
			}

//		PORTCbits.RC2=0;

		if((prev_poll&0b10000)!=(poll&0b10000)){
			if(ledS.countRB4<2) ledS.countRB4++;
			}

		if((prev_poll&0b100000)!=(poll&0b100000)){
			if(ledS.countRB5<2) ledS.countRB5++;
			}
		}
	}

//These are your actual interrupt handling routines.
#pragma interrupt HighPriorityInterruptHandler

//#define	INTERRUPT_ON_RC0
#if	defined	INTERRUPT_ON_RC0
unsigned char fThing=0;
#endif

void HighPriorityInterruptHandler()
{
	
	#ifdef	MULTI_BUZZER
	struct BuzzDev *buzzWalk=NULL;
	unsigned char buzzIdx = 0;
	#endif

	// RC 2 - Pin 4
	// RC 1 - Pin 5
	// RC 0 - Pin 6
#if	defined	INTERRUPT_ON_RC0
	if(fThing) {
		PORTCbits.RC0=0;
		fThing=0;
		} else {
			PORTCbits.RC0=1;
			fThing=0xff;
			}
#endif
	
	compute_clock_counter();

	#ifdef	MULTI_BUZZER
	if(PIR1bits.TMR1IF){

		for(buzzWalk=shortListBD[buzzIdx]; buzzWalk!=NULL; buzzWalk=shortListBD[buzzIdx++]) {
			if(buzzWalk->cycles) buzzWalk->cycles--;
			else {
				if(signalBD(buzzWalk)){
					// Drive signal low and pause 
					lowBD(buzzWalk);
					buzzWalk->cycles=buzzWalk->pause;
					} else {
						// Beep it
						highBD(buzzWalk);
						buzzWalk->cycles=BEEP_DURATION;
						}
				}
			}
		
		PIR1bits.TMR1IF=0;
		return;
		}
	#endif

	if (TIMER0_FLAG)
	    {
		// We had interrupt fired. So let's compute from the overflow state
		timer_clkdata.s2byte.b0=0xFF;
		timer_clkdata.s2byte.b1=0xFF;
		sampling_clock_counter += (timer_clkdata.clk_count-timer_prev_clk_count);
		TIMER0_FLAG = 0;
		return;
	    }

	//Check which interrupt flag caused the interrupt.
	//Service the interrupt
	//Clear the interrupt flag
	//Etc.

#if	defined(USE_I2C)
	if(I2C_INTERRUPT_FLAG){
		process_i2c_interrupt();
		I2C_INTERRUPT_FLAG=0;
		return;
		}
#endif
	if(INTCONbits.INT0IF){
		// Clear the flag to prevent looping
		INTCONbits.INT0IF=1;
		return;
		}

#if	defined(__DEBUG)
	unknown_calls ++;
#endif
}

#pragma code

void setup_portb_for_reading()
{
	if(!flags.scanner){
		package_start=&usbPackage.deckCycles[0];
		package_end=&usbPackage.deckCycles[STORAGE_PACKET_SIZE-1];
		tailp=NULL;
		headp=package_start;
		} else {
			package_start=package_end=tailp=headp=NULL;
			}

	LOW_SIG_PULLUP_DISABLE=0;

	// RB4 & RB5 are inputs
	TRISBbits.TRISB4=1;
	TRISBbits.TRISB5=1;

	LATB=0x0;
	PORTB=0x0;

	poll=0x0;
	prev_poll=0xFF;

	PORTB_IRPT_ON_CHANGE_PRIO=0; // Low priority on data protocol readings
	HIGH_PRIO_IRPT_ENABLE=1;
	PERIPHERAL_IRPT_ENABLE=1;

	PORTB_IRPT_ON_CHANGE_ENABLE=1;
}

void setup_portb_for_reading();

void ioc_start()
{
	setup_portb_for_reading();
}

void ioc_stop()
{
	tailp=headp=NULL;
	PORTB_IRPT_ON_CHANGE_ENABLE=0;
}

void timer0_start()
{
	T0CON = T0C0N_PRESCALE;
	//
	// Timer0 - need for RTC function
	//
	HIGH_PRIO_IRPT_ENABLE=1;
	LOW_PRIO_IRPT_ENABLE=1;
	IRPT_PRIORITIES_ENABLE=1;

	TMR0H = 0;                    //clear timer
	TMR0L = 0;                    //clear timer
	/*
//	T0CON = T0C0N_PRESCALE;                 //set up timer0 - prescaler 1:8

	// 0xC8 - 8-bit, no prescaler
	// 0x88 - no prescaler, 1:1; 1000 cycles/6 seconds, 8 MHz
	// 0x87 - 1:256
	// 0x82 - 1:8
	// 0x81 - 1:4
	// 0x80 - 1:20
	T0CONbits.T08BIT=0; // T08BIT as 0 for 16-bit operation
	T0CONbits.PSA=0; // PSA for Prescaler not assigned; Same as 0x88
	T0CONbits.T0PS0=0;
	T0CONbits.T0PS1=0;
	T0CONbits.T0PS2=0;

	T0CONbits.T0CS=0; // T0CS - 1 for T0CKL pin, 0 for internal CLK0
	T0CONbits.T0SE=0; // 1 for falling edge of T0CKI, 0 for rising edge
	*/

	timer_clkdata.s2byte.b1=0;
	timer_clkdata.s2byte.b0=0;
	timer_prev_clk_count=0;
	TMR0H=0;
	TMR0L=0;
	sampling_clock_counter=0;

	TIMER0_PRIO=1; // High priority
	TIMER0_IRPT_ENABLE=1;
	TIMER0_ON=1;
}

void timer0_stop()
{
	TIMER0_IRPT_ENABLE=0;
	TIMER0_ON=0;
}

void start_interrupts()
{
	//set_state(ST_DIGITAL_IO_STARTED);

	ioc_start();
	timer0_start();
}

#define	COMMAND_BLOCK(a) 	if(flags.a){ if(handleBusy) { return;} flags.a=0;
#define	START_BLOCK(a) 		if(flags.a){ if(handleBusy)  { return;}
#define	NO_OUT_BLOCK(a) 	if(flags.a){ flags.a=0; noout=1;
#define	START_NO_OUT_BLOCK(a) 	if(flags.a){

#define	END_BLOCK if(!noout)  { return;} }

#ifdef	__DEBUG
unsigned long handlecnt=0;
#endif

void ProcessCommands()
{
unsigned char handleBusy=1, noout=0, cnt=0, cmd_shift=0;

	compute_clock_counter();

NO_OUT_BLOCK(reset)
	stop_interrupts();
	pio_reset();
END_BLOCK

NO_OUT_BLOCK(stop_reading)
	stop_interrupts();
	// UserInit(); it was clearing ping flag, looping the open() on host
	//set_state(ST_DIGITAL_IO_STOPPED);
END_BLOCK

#if	defined(USE_I2C)
NO_OUT_BLOCK(open_i2c)
	open_i2c();
END_BLOCK

NO_OUT_BLOCK(close_i2c)
	close_i2c();
END_BLOCK
#endif

NO_OUT_BLOCK(read_parktronic)
	//set_state(ST_DIGITAL_IO_STARTED);
	UserInit();
	start_interrupts();
END_BLOCK

/* NO WAY TO DO THIS. WILL CAUSE TIMER TO RESET ALL THE TIME
// it might happen we will not enter into a detection stage at all
NO_OUT_BLOCK(scanner)
	// DO NOT REMOVE THE FOLLOWING LINE. NO_OUT_BLOCK(scanner) clears it
	BITFLAG1(scanner);
	set_state(ST_DIGITAL_IO_STARTED);
	start_interrupts();
END_BLOCK
*/

handleBusy=USBHandleBusy(USBGenericInHandle);

if(handleBusy) return;

COMMAND_BLOCK(ping)
	pio_ping();
END_BLOCK

COMMAND_BLOCK(shift)
	pio_read_shift();
END_BLOCK

#ifndef	MULTI_BUZZER
COMMAND_BLOCK(buzzer)
	pio_read_buzzer();
END_BLOCK
#endif

COMMAND_BLOCK(get_thermal)
	pio_get_thermal();
END_BLOCK

COMMAND_BLOCK(get_version)
	pio_get_version();
END_BLOCK

COMMAND_BLOCK(get_io_registers2)

	cmd_shift=-63;

	version_buf[cnt=0]=CMD_GET_IO_REGISTERS2;

	version_buf[REG_TRISB+cmd_shift]=TRISB;//64
	version_buf[REG_TRISA+cmd_shift]=TRISA;//65
	version_buf[REG_LATC+cmd_shift]=LATC;//66
	version_buf[REG_LATB+cmd_shift]=LATB;//67
	version_buf[REG_LATA+cmd_shift]=LATA;//68
	version_buf[REG_PORTC+cmd_shift]=PORTC;//71
	version_buf[REG_PORTB+cmd_shift]=PORTB;//72
	version_buf[REG_PORTA+cmd_shift]=PORTA;//73
	version_buf[REG_UEP15+cmd_shift]=UEP15;//74
	version_buf[REG_UEP14+cmd_shift]=UEP14;//75
	version_buf[REG_UEP13+cmd_shift]=UEP13;//76
	version_buf[REG_UEP12+cmd_shift]=UEP12;//77
	version_buf[REG_UEP11+cmd_shift]=UEP11;//78
	version_buf[REG_UEP10+cmd_shift]=UEP10;//79
	version_buf[REG_UEP9+cmd_shift]=UEP9;//80
	version_buf[REG_UEP8+cmd_shift]=UEP8;//81
	version_buf[REG_UEP7+cmd_shift]=UEP7;//82
	version_buf[REG_UEP6+cmd_shift]=UEP6;//83
	version_buf[REG_UEP5+cmd_shift]=UEP5;//84
	version_buf[REG_UEP4+cmd_shift]=UEP4;//85
	version_buf[REG_UEP3+cmd_shift]=UEP3;//86
	version_buf[REG_UEP2+cmd_shift]=UEP2;//87
	version_buf[REG_UEP1+cmd_shift]=UEP1;//88
	version_buf[REG_UEP0+cmd_shift]=UEP0;//89
	version_buf[REG_UCFG+cmd_shift]=UCFG;//90
	version_buf[REG_UADDR+cmd_shift]=UADDR;//91
	version_buf[REG_UCON+cmd_shift]=UCON;//92
	version_buf[REG_USTAT+cmd_shift]=USTAT;//93
	version_buf[REG_UEIE+cmd_shift]=UEIE;//94
	version_buf[REG_UEIR+cmd_shift]=UEIR;//95
	version_buf[REG_UIE+cmd_shift]=UIE;//96
	version_buf[REG_UIR+cmd_shift]=UIR;//97
	version_buf[REG_UFRMH+cmd_shift]=UFRMH;//98
	version_buf[REG_UFRML+cmd_shift]=UFRML;//99

	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
END_BLOCK

COMMAND_BLOCK(get_io_registers1)

	cmd_shift=0;

	version_buf[cnt=0]=CMD_GET_IO_REGISTERS1;

	version_buf[REG_INDF2+cmd_shift]=INDF2;//1
	version_buf[REG_POSTINC2+cmd_shift]=POSTINC2;//2
	version_buf[REG_POSTDEC2+cmd_shift]=POSTDEC2;//3
	version_buf[REG_PREINC2+cmd_shift]=PREINC2;//4
	version_buf[REG_PLUSW2+cmd_shift]=PLUSW2;//5
	version_buf[REG_FSR2H+cmd_shift]=FSR2H;//6
	version_buf[REG_FSR2L+cmd_shift]=FSR2L;//7
	version_buf[REG_STATUS+cmd_shift]=STATUS;//8
	version_buf[REG_TMR0H+cmd_shift]=TMR0H;//9
	version_buf[REG_TMR0L+cmd_shift]=TMR0L;//10
	version_buf[REG_T0CON+cmd_shift]=T0CON;//11
	version_buf[REG_OSCCON+cmd_shift]=OSCCON;//12
	version_buf[REG_HLVDCON+cmd_shift]=HLVDCON;//13
	version_buf[REG_WDTCON+cmd_shift]=WDTCON;//14
	version_buf[REG_RCON+cmd_shift]=RCON;//15
	version_buf[REG_TMR1H+cmd_shift]=TMR1H;//16
	version_buf[REG_TMR1L+cmd_shift]=TMR1L;//17
	version_buf[REG_T1CON+cmd_shift]=T1CON;//18
	version_buf[REG_TMR2+cmd_shift]=TMR2;//19
	version_buf[REG_PR2+cmd_shift]=PR2;//20
	version_buf[REG_T2CON+cmd_shift]=T2CON;//21
	version_buf[REG_SSPBUF+cmd_shift]=SSPBUF;//22
	version_buf[REG_SSPADD+cmd_shift]=SSPADD;//23
	version_buf[REG_SSPSTAT+cmd_shift]=SSPSTAT;//24
	version_buf[REG_SSPCON1+cmd_shift]=SSPCON1;//25
	version_buf[REG_SSPCON2+cmd_shift]=SSPCON2;//26
	version_buf[REG_ADRESH+cmd_shift]=ADRESH;//27
	version_buf[REG_ADRESL+cmd_shift]=ADRESL;//28
	version_buf[REG_ADCON0+cmd_shift]=ADCON0;//29
	version_buf[REG_ADCON1+cmd_shift]=ADCON1;//30
	version_buf[REG_ADCON2+cmd_shift]=ADCON2;//31
	version_buf[REG_CCPR1H+cmd_shift]=CCPR1H;//32
	version_buf[REG_CCPR1L+cmd_shift]=CCPR1L;//33
	version_buf[REG_CCP1CON+cmd_shift]=CCP1CON;//34
	version_buf[REG_CCPR2H+cmd_shift]=CCPR2H;//35
	version_buf[REG_CCPR2L+cmd_shift]=CCPR2L;//36
	version_buf[REG_CCP2CON+cmd_shift]=CCP2CON;//37
	version_buf[REG_BAUDCON+cmd_shift]=BAUDCON;//38
	version_buf[REG_ECCP1DEL+cmd_shift]=ECCP1DEL;//39
	version_buf[REG_ECCP1AS+cmd_shift]=ECCP1AS;//40
	version_buf[REG_CVRCON+cmd_shift]=CVRCON;//41
	version_buf[REG_CMCON+cmd_shift]=CMCON;//42
	version_buf[REG_TMR3H+cmd_shift]=TMR3H;//43
	version_buf[REG_TMR3L+cmd_shift]=TMR3L;//44
	version_buf[REG_T3CON+cmd_shift]=T3CON;//45
	version_buf[REG_SPBRGH+cmd_shift]=SPBRGH;//46
	version_buf[REG_SPBRG+cmd_shift]=SPBRG;//47
	version_buf[REG_RCREG+cmd_shift]=RCREG;//48
	version_buf[REG_TXREG+cmd_shift]=TXREG;//49
	version_buf[REG_TXSTA+cmd_shift]=TXSTA;//50
	version_buf[REG_RCSTA+cmd_shift]=RCSTA;//51
	version_buf[REG_EEADR+cmd_shift]=EEADR;//52
	version_buf[REG_EEDATA+cmd_shift]=EEDATA;//53
	version_buf[REG_EECON2+cmd_shift]=EECON2;//54
	version_buf[REG_EECON1+cmd_shift]=EECON1;//55
	version_buf[REG_IPR2+cmd_shift]=IPR2;//56
	version_buf[REG_PIR2+cmd_shift]=PIR2;//57
	version_buf[REG_PIE2+cmd_shift]=PIE2;//58
	version_buf[REG_IPR1+cmd_shift]=IPR1;//59
	version_buf[REG_PIR1+cmd_shift]=PIR1;//60
	version_buf[REG_PIE1+cmd_shift]=PIE1;//61
	version_buf[REG_OSCTUNE+cmd_shift]=OSCTUNE;//62
	version_buf[REG_TRISC+cmd_shift]=TRISC;//63

	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
END_BLOCK

//
// Must be the last one. Upper commands used to control the device
//
COMMAND_BLOCK(data_packet)

	if(flags.no_feed){

		usbStat.code=CMD_DATA_TIMEOUT;
		usbStat.size=0;

		// usbState must be untouched until next USBHandleBusy returned;
		USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&usbStat),64);
	} else {

		if(flags.scanner) {

			makeStatusDataPacket();

			// Sampling mode, we just send raw data we have grabbed from the logic bits
			// into the packet
			}  else {
				make_in_packet();
				}
		}
END_BLOCK
}

/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/
extern void get_firmware_version(char* d,unsigned char lg);

void pio_wait_for_inhandle()
{
	//set_state(ST_WAITING_OUTPUT_PIPE);

	while(USBHandleBusy(USBGenericInHandle)) Nop();
}

void pio_get_version()
{
	//set_state(ST_GETTING_VERSION);

	// 0 1 2  3 4 5 6 7 8 9 10
	// -CMD-  1 . 1 . 1 x x x
	version_buf[0]=CMD_GET_VERSION;
	version_buf[1]='G';
	version_buf[2]='V';
	get_firmware_version(&version_buf[3],61);

	// usbState must not be touched until next USBHandleBusy returned;
	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
}

void pio_reset()
{
	unsigned char i;

	stop_interrupts();

	for(	sampling_clock_counter=0;
		sampling_clock_counter<500000;
		sampling_clock_counter++) {

			Nop();Nop();Nop();Nop();

			Nop();
			}

	// Perform a full device reset
	UCONbits.SUSPND = 0;		//Disable USB module
	UCON = 0x00;				//Disable USB module
	//And wait awhile for the USB cable capacitance to discharge down to disconnected (SE0) state.
	//Otherwise host might not realize we disconnected/reconnected when we do the reset.
	//A basic for() loop decrementing a 16 bit number would be simpler, but seems to take more code space for
	//a given delay.  So do this instead:

	for(i = 0; i < 0xFF; i++)
	{
		WREG = 0xFF;
		while(WREG)
		{
			WREG--;
			_asm
			bra	0	//Equivalent to bra $+2, which takes half as much code as 2 nop instructions
			bra	0	//Equivalent to bra $+2, which takes half as much code as 2 nop instructions
			_endasm
		}
	}
	Reset();
}

void pio_ping()
{
	unsigned char sz=sizeof(version_buf), i =0;
#ifndef	MULTI_BUZZER	
	buzzer_3beeps();
#endif
	for(i=sz-1;i>0;i--) version_buf[i]='X';
	version_buf[0]=CMD_PING;
	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
	// Led animation
	led_animate(ANIMATION_ALL_3SHORT);
}

/**
 *
 * @param
 */

void set_register(unsigned char r, unsigned char val)
{
switch(r){
	case REG_INDF2: INDF2=val;break;
	case REG_POSTINC2: POSTINC2=val;break;
	case REG_POSTDEC2: POSTDEC2=val;break;
	case REG_PREINC2: PREINC2=val;break;
	case REG_PLUSW2: PLUSW2=val;break;
	case REG_FSR2H: FSR2H=val;break;
	case REG_FSR2L: FSR2L=val;break;
	case REG_STATUS: STATUS=val;break;
	case REG_TMR0H: TMR0H=val;break;
	case REG_TMR0L: TMR0L=val;break;
	case REG_T0CON: T0CON=val;break;
	case REG_OSCCON: OSCCON=val;break;
	case REG_HLVDCON: HLVDCON=val;break;
	case REG_WDTCON: WDTCON=val;break;
	case REG_RCON: RCON=val;break;
	case REG_TMR1H: TMR1H=val;break;
	case REG_TMR1L: TMR1L=val;break;
	case REG_T1CON: T1CON=val;break;
	case REG_TMR2: TMR2=val;break;
	case REG_PR2: PR2=val;break;
	case REG_T2CON: T2CON=val;break;
	case REG_SSPBUF: SSPBUF=val;break;
	case REG_SSPADD: SSPADD=val;break;
	case REG_SSPSTAT: SSPSTAT=val;break;
	case REG_SSPCON1: SSPCON1=val;break;
	case REG_SSPCON2: SSPCON2=val;break;
	case REG_ADRESH: ADRESH=val;break;
	case REG_ADRESL: ADRESL=val;break;
	case REG_ADCON0: ADCON0=val;break;
	case REG_ADCON1: ADCON1=val;break;
	case REG_ADCON2: ADCON2=val;break;
	case REG_CCPR1H: CCPR1H=val;break;
	case REG_CCPR1L: CCPR1L=val;break;
	case REG_CCP1CON: CCP1CON=val;break;
	case REG_CCPR2H: CCPR2H=val;break;
	case REG_CCPR2L: CCPR2L=val;break;
	case REG_CCP2CON: CCP2CON=val;break;
	case REG_BAUDCON: BAUDCON=val;break;
	case REG_ECCP1DEL: ECCP1DEL=val;break;
	case REG_ECCP1AS: ECCP1AS=val;break;
	case REG_CVRCON: CVRCON=val;break;
	case REG_CMCON: CMCON=val;break;
	case REG_TMR3H: TMR3H=val;break;
	case REG_TMR3L: TMR3L=val;break;
	case REG_T3CON: T3CON=val;break;
	case REG_SPBRGH: SPBRGH=val;break;
	case REG_SPBRG: SPBRG=val;break;
	case REG_RCREG: RCREG=val;break;
	case REG_TXREG: TXREG=val;break;
	case REG_TXSTA: TXSTA=val;break;
	case REG_RCSTA: RCSTA=val;break;
	case REG_EEADR: EEADR=val;break;
	case REG_EEDATA: EEDATA=val;break;
	case REG_EECON2: EECON2=val;break;
	case REG_EECON1: EECON1=val;break;
	case REG_IPR2: IPR2=val;break;
	case REG_PIR2: PIR2=val;break;
	case REG_PIE2: PIE2=val;break;
	case REG_IPR1: IPR1=val;break;
	case REG_PIR1: PIR1=val;break;
	case REG_PIE1: PIE1=val;break;
	case REG_OSCTUNE: OSCTUNE=val;break;
//	case REG_TRISE: TRISE=val;break;
//	case REG_TRISD: TRISD=val;break;
	case REG_TRISC: TRISC=val;break;
	case REG_TRISB: TRISB=val;break;
	case REG_TRISA: TRISA=val;break;
//	case REG_LATE: LATE=val;break;
//	case REG_LATD: LATD=val;break;
	case REG_LATC: LATC=val;break;
	case REG_LATB: LATB=val;break;
	case REG_LATA: LATA=val;break;
//	case REG_PORTE: PORTE=val;break;
//	case REG_PORTD: PORTD=val;break;
	case REG_PORTC: PORTC=val;break;
	case REG_PORTB: PORTB=val;break;
	case REG_PORTA: PORTA=val;break;
	case REG_UEP15: UEP15=val;break;
	case REG_UEP14: UEP14=val;break;
	case REG_UEP13: UEP13=val;break;
	case REG_UEP12: UEP12=val;break;
	case REG_UEP11: UEP11=val;break;
	case REG_UEP10: UEP10=val;break;
	case REG_UEP9: UEP9=val;break;
	case REG_UEP8: UEP8=val;break;
	case REG_UEP7: UEP7=val;break;
	case REG_UEP6: UEP6=val;break;
	case REG_UEP5: UEP5=val;break;
	case REG_UEP4: UEP4=val;break;
	case REG_UEP3: UEP3=val;break;
	case REG_UEP2: UEP2=val;break;
	case REG_UEP1: UEP1=val;break;
	case REG_UEP0: UEP0=val;break;
	case REG_UCFG: UCFG=val;break;
	case REG_UADDR: UADDR=val;break;
	case REG_UCON: UCON=val;break;
	case REG_USTAT: USTAT=val;break;
	case REG_UEIE: UEIE=val;break;
	case REG_UEIR: UEIR=val;break;
	case REG_UIE: UIE=val;break;
	case REG_UIR: UIR=val;break;
	case REG_UFRMH: UFRMH=val;break;
	case REG_UFRML: UFRML=val;break;
	case REG_INTCON:INTCON=val;break;
	case REG_INTCON2:INTCON2=val;break;
	case REG_INTCON3:INTCON3=val;break;

//	case REG_SPPCON: SPPCON=val;break;
//	case REG_SPPEPS: SPPEPS=val;break;
//	case REG_SPPCFG: SPPCFG=val;break;
//	case REG_SPPDATA: SPPDATA=val;break;
	};
}

void ProcessIO(void)
{
	unsigned char temp = 0, pos=0;

	compute_clock_counter();


    //User Application USB tasks below.
    //Note: The user application should not begin attempting to read/write over the USB
    //until after the device has been fully enumerated.  After the device is fully
    //enumerated, the USBDeviceState will be set to "CONFIGURED_STATE".
    if(USBDeviceState < CONFIGURED_STATE) return;

    if(USBSuspendControl==1) return;

    //As the device completes the enumeration process, the USBCBInitEP() function will
    //get called.  In this function, we initialize the user application endpoints (in this
    //example code, the user application makes use of endpoint 1 IN and endpoint 1 OUT).
    //The USBGenRead() function call in the USBCBInitEP() function initializes endpoint 1 OUT
    //and "arms" it so that it can receive a packet of data from the host.  Once the endpoint
    //has been armed, the host can then send data to it (assuming some kind of application software
    //is running on the host, and the application software tries to send data to the USB device).

    //If the host sends a packet of data to the endpoint 1 OUT buffer, the hardware of the SIE will
    //automatically receive it and store the data at the memory location pointed to when we called
    //USBGenRead().  Additionally, the endpoint handle (in this case USBGenericOutHandle) will indicate
    //that the endpoint is no longer busy.  At this point, it is safe for this firmware to begin reading
    //from the endpoint buffer, and processing the data.

    if(!USBHandleBusy(USBGenericOutHandle)){		//Check if the endpoint has received any data from the host.

//		PORTCbits.RC2=1;
		ledS.usbOUTtimeout=10;

		// -- OUTPacket has data we can use.
		switch(OUTPacket[0]){

			case CMD_GET_THERMAL:
				BITFLAG1(get_thermal);
				break;

			case CMD_GET_VERSION:
				BITFLAG1(get_version);
				//pio_get_version();
				break;

			case CMD_ENTER_BOOTLOADER:
				// Modify EEPROM flag to trigger up the bootloader
				if(!set_EEPROM(EEPROM_FLAG_ADDR,EEPROM_BOOTLOADER_ON)) break;
				
				#ifndef	MULTI_BUZZER
				// Beep 5 times here to indicate command for bootloader was entered
				buzzer_wait_for_5beeps();
				// do not break; here, we need to Reset()
				#endif
				// CONTROL ---->>>

			case CMD_RESET:
				stop_interrupts();
				BITFLAG1(reset);
				//pio_reset();
				break;

			// Behave ..
			case CMD_STOP_READING:
				stop_interrupts();
				BITFLAG1(stop_reading);
				break;

			case CMD_READ_PARKTRONIC:
				BITFLAG1(read_parktronic);
				BITFLAG0(scanner);
				break;

			case CMD_READ_DATA_PACKET:
				BITFLAG1(data_packet);
				break;
				
			case CMD_SET_REGISTER:
				set_register(OUTPacket[1],OUTPacket[2]);
				break;
				
			case CMD_DATA_FORMAT:
				BITFLAG1(scanner);
				initChallengerData(&ch26data);
				// Process protocol instructions.
				pos=2;
				for(temp=0;temp<OUTPacket[1];temp++){
					switch(OUTPacket[pos++]){
						// Only challenger 26 is implemented, at this point
						case 26:
							add_ch26data_bit(OUTPacket[pos++]);
							break;
						}
					}

				start_interrupts();
				break;

			case CMD_SET_SHIFT:
				save_shift(OUTPacket);
				break;

			case CMD_READ_SHIFT:
				BITFLAG1(shift);
				break;

			#ifndef	MULTI_BUZZER
			case CMD_SET_BUZZER:
				// Reload buzzer
				save_buzzer(OUTPacket);
				setup_buzzer();
				// Blink & beep
				BITFLAG1(ping); // PING!!
				break;

			case CMD_READ_BUZZER:
				BITFLAG1(buzzer);
				break;
			#endif
			
			case	CMD_PING:
				// It will not only stop, it will also abort data reading.
				standalone_stop();
				BITFLAG1(ping);
				//pio_ping();
				break;

			case	CMD_GET_IO_REGISTERS1:
				stop_interrupts();
				BITFLAG1(get_io_registers1);
				break;

			case	CMD_GET_IO_REGISTERS2:
				stop_interrupts();
				BITFLAG1(get_io_registers2);
				break;

			default:
				stop_interrupts();
				//set_state(ST_UNKNOWN);
				break;
			}

		// That is re-arming call as Microchip calls it
		// We should read exactly the amount sent from the host, otherwise USB stack will keep
		// hangin
		USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,HOST_COMMAND_SIZE);

//		PORTCbits.RC2=0;
		}
}//end ProcessI

#ifdef	MULTI_BUZZER
//
// TIMER 1
//
void start_timer1(void)
{
	//Init Timer1 
	T1CONbits.RD16 = 0;      //read/write 16 bit values 
	T1CONbits.T1CKPS1 = 1;    //1:1 prescaler 
	T1CONbits.T1CKPS0 = 1; 
	T1CONbits.T1OSCEN = 0;  //oscillator shut off 
	T1CONbits.TMR1CS = 0;   //user internal clock (Fosc/4) 
	PIE1bits.TMR1IE = 1;    //enable timer1 interrupt on overflow 
	IPR1bits.TMR1IP = 1;    //priority of interrupt, 0- low, 1 - high
	PIR1bits.TMR1IF = 0;    //clear interrupt flag 
	//TIMER1_SET_COUNTERS
	T1CONbits.TMR1ON = 1;      //enable timer1 
}

void stop_timer1(void)
{
	T1CONbits.TMR1ON = 0;      
}
#endif

//
// TIMER 3
//

void start_timer3()
{
	HIGH_PRIO_IRPT_ENABLE=1;

	T3CONbits.TMR3CS=0; // Internal clock

	// Delays with scalers
	// 11 - 1:8 prescale
	T3CONbits.T3CKPS1=1;
	T3CONbits.T3CKPS0=1;

/*
	T3CONbits.T3CKPS1=0; // fast poll
	T3CONbits.T3CKPS0=0;
*/

	TMR3H=0x0;
	TMR3L=0x0;

	T3CONbits.RD16=1; // Timer1 in 16 or 8 bit operation

	TIMER3_FLAG=0;
	TIMER3_PRIO=0; // Low
	TIMER3_IRPT_ENABLE=1; // Enable interrupt
	TIMER3_ON=1; //  Timer on
}

// LEDs interrupt
void stop_timer3()
{
	TIMER3_IRPT_ENABLE=0; // Disable interrupt
	TIMER3_ON=0;
}


void led_set(char led,char status)
{
	switch(led){
		case LED_GREEN:
			ledS.green=status;
			break;
		case LED_WHITE:
			ledS.white=status;
			break;
		case LED_RED:
			ledS.red=status;
			break;
		default: ;
			// FATAL ERROR
		}
}

void animation_all3short(int isShort,int isOn)
{
	if(isShort) switch(ledS.animation_count){
		case 0:
			ledS.bkRed=ledS.red; ledS.bkGreen=ledS.green; ledS.bkWhite=ledS.white;
		case 2: case 4: case 6:
			colorOff(RED); colorOff(GREEN); colorOff(WHITE); ledS.animation_count++;
			break;
		case 1: case 3: case 5:
			colorOn(RED); colorOn(GREEN); colorOn(WHITE); ledS.animation_count++;
			break;
		default:
			ledS.red=ledS.bkRed;ledS.green=ledS.bkGreen;ledS.white=ledS.bkWhite;
			ledS.animation_code=0; ledS.animation_count=0;
			break;
		}
}

void animation_all_2short_long(int isShort,int isOn)
{
	if(isShort) switch(ledS.animation_count){
		case 0:
			ledS.bkRed=ledS.red; ledS.bkGreen=ledS.green; ledS.bkWhite=ledS.white;
		case 2:
			colorOff(RED); colorOff(GREEN); colorOff(WHITE); ledS.animation_count++;
			break;

		case 1:
		case 3:
			colorOn(RED); colorOn(GREEN); colorOn(WHITE); ledS.animation_count++;
			break;
		}

	if(!isShort && ledS.animation_count>=4) switch(ledS.animation_count){
		case 5: case 7: colorOff(RED); colorOff(GREEN); colorOff(WHITE); ledS.animation_count++; break;
		case 6: colorOn(RED); colorOn(GREEN); colorOn(WHITE); ledS.animation_count++; break;
		default:
			ledS.red=ledS.bkRed;ledS.green=ledS.bkGreen;ledS.white=ledS.bkWhite;
			ledS.animation_code=0; ledS.animation_count=0;
			break;
		}
}

void led_animate(unsigned char code){
	ledS.animation_count=0;
	ledS.animation_code=code;
	}

void process_animation(int isShort,int isOn)
{
	switch(ledS.animation_code){
		case ANIMATION_ALL_3SHORT:
			animation_all3short(isShort,isOn);
			break;
		case ANIMATION_ALL_2SHORT_LONG:
			animation_all_2short_long(isShort,isOn);
			break;
		}
}

void timeout_line(unsigned char l,unsigned char val){

	unsigned char h=0;

	struct ChallengerLine* line=&ch26data.line[ch26data.bit2line[l]];

	line->signalTimeout=val;

	if(val) for(h=0;h<line->usedHeadsCount;h++) {
		line->dist[h].dist=0xFF;
		#ifdef	MULTI_BUZZER
		// Shut the buzzer on timed out line
		setBuzzDevDistance(line->dist[h].buzzerId,255);
		#endif
		}
		
	}

void short_leds(int f)
{
	unsigned char meActive=0;
	unsigned char meDataActive=0;

	if(ledS.animation_code) { process_animation(1,f); return; }

	if(f) {
		if(ledS.red==lSHORT||ledS.red==lON)	colorOn(RED);
		if(ledS.green==lSHORT||ledS.green==lON)	colorOn(GREEN);
		if(ledS.white==lSHORT||ledS.white==lON)	colorOn(WHITE);
	} else {
		if(ledS.red==lSHORT||ledS.red==lOFF)		colorOff(RED);
		if(ledS.green==lSHORT||ledS.green==lOFF)	colorOff(GREEN);
		if(ledS.white==lSHORT||ledS.white==lOFF)	colorOff(WHITE);
	}

	// Check line timeouts
	if(!ledS.countRB4 && !ledS.dataCountRB4){
		if(ledS.red!=lOFF){// trick to send timeout packet only once on change
			ledS.red=lOFF;
			timeout_line(4,1);
			}
		} else {
			if(ledS.countRB4){
				ledS.countRB4--;
				ledS.red=lSHORT;
				meActive=1;
				}

			if(ledS.dataCountRB4){
				timeout_line(4,0);
				ledS.dataCountRB4--;
				ledS.red=lLONG;
				meDataActive=1;
				}
			}

	if(!ledS.countRB5 && !ledS.dataCountRB5){
		if(ledS.white!=lOFF){ // trick to send timeout packet only once on change
			ledS.white=lOFF;
			timeout_line(5,1);
			}
		} else {
			timeout_line(5,0);
			if(ledS.countRB5){
				ledS.countRB5--;
				ledS.white=lSHORT;
				meActive=1;
				}

			if(ledS.dataCountRB5){
				ledS.dataCountRB5--;
				ledS.white=lLONG;
				meDataActive=1;
				}
			}

	if(meActive) ledS.green=lLONG;
	else if(meDataActive) ledS.green=lSHORT;
		else ledS.green=lON;
}

void long_leds(int f)
{
	ledS.usbOUTtimeout--;
	if(!ledS.usbOUTtimeout) {
		// Fall back to standalone mode
		standalone_start();
		}

	if(ledS.animation_code) { process_animation(0,f); return; }

	if(f) {
		if(ledS.red==lLONG)	colorOn(RED);
		if(ledS.green==lLONG)	colorOn(GREEN);
		if(ledS.white==lLONG)	colorOn(WHITE);
	} else {
		if(ledS.red==lLONG)	colorOff(RED);
		if(ledS.green==lLONG)	colorOff(GREEN);
		if(ledS.white==lLONG)	colorOff(WHITE);
	}
}

void init_leds()
{
	TRISA=0x0;

	ledS.s=0;
	ledS.l=0;

	ledS.red=ledS.green=ledS.white=lOFF;

	ledS.animation_code=0;
	ledS.animation_count=0;

	ledS.countRB4=ledS.countRB5=0;

	ledS.usbOUTtimeout=0;
}

void short_timeout()
{
	if(flags.standalone) { 
		grabMinDistance(); 
		#ifndef	MULTI_BUZZER
		buzzer_distance(); 
		#endif
		}

	// LED CONTROL
	short_leds(ledS.s);
	ledS.s=!ledS.s;

	#ifndef	MULTI_BUZZER
	// Always on buzzer
	if(ledS.buzzerAlwaysOn) buzzer_on(); else
		// Periodic buzzer
		if(ledS.buzzerPeriod) {
			if(!ledS.buzzerCounter){
				ledS.buzzerCounter=ledS.buzzerPeriod;
				// switch status
				if(buzzer_status()) buzzer_off(); else buzzer_on();
				}
			if(ledS.s) ledS.buzzerCounter--;
			} else {
					// Beep counter number of times
					if(ledS.buzzerCounter){
						if(ledS.s) buzzer_on(); else { ledS.buzzerCounter--; buzzer_off(); }
						}
					}
	#endif
}

void long_timeout()
{
	if(ledS.l){
		ledS.l=0;
		long_leds(1);
		} else {
			ledS.l=1;
			long_leds(0);
			}
}

void timer3_interrupt()
{
	ledS.cnt++;

	if((ledS.cnt%T3_PERIOD_SHORT)==0){
		short_timeout();
		}

	if(ledS.cnt>=T3_PERIOD_LONG){
			long_timeout();
			ledS.cnt=0;
			}
}

// ****************************************************************************
// *************************** TEMPERATURE CONTROL ****************************

#include	"types.h"

#define output_low(pin)     TRISCbits.TRISC6 = 0;(PORTCbits.RC6 = 0)
#define output_high(pin)    TRISCbits.TRISC6 = 0;(PORTCbits.RC6 = 1)
#define input(pin)          input_func()

bool input_func(void)
{
    TRISCbits.TRISC6 = 1;
    return (PORTCbits.RC6);
}

void delay_us(uint16 usec)
{
	// internal clock is 48 Mhz (produced by pre-scaler PLL)
	int steps;
	if(usec<50){
		steps=usec-5; 
		do{ steps--; } while(steps>0);
		return;
		}
	// Bigger waiting times
	Delay100TCYx(usec*30); // usec * 12 / 10 (delay10tcyx)
}

void delay_ms(uint16 msec)
{
	delay_us(1000*msec);
	/*
    while(msec!=0)
    {
        msec--;
        Delay100TCYx(80);
    }
	*/
}

/*

// that's an unusable horse shit

// asm_timed_delay is found in an assembly file 
extern void delay_asm_c18_32mhz(sint16 usec);

#define delay_us    delay_asm_c18_32mhz
*/

#include	"ds1820.h"

void pio_get_thermal()
{
	uint8 c,walker;
	sint16* traw;

	// 0 1 2  3 4 5 6 7 8 9 10
	// -CMD-  1 . 1 . 1 x x x
	version_buf[0]=CMD_GET_THERMAL;
	version_buf[1]='T';
	version_buf[2]='R';
	version_buf[3] = 0; // Sensor counter

	walker=4;
	
	if ( DS1820_FindFirstDevice() )
	{
	   do
	   {
		// 10 bytes : 8 addr + 2 byte temperature
		for(c=0;c<DS1820_ADDR_LEN;c++) {
			version_buf[walker++]=nRomAddr_au8[c];
			}

	       /* get temperature raw value (resolution 1/256°C) */

		traw=(sint16*)&version_buf[walker];
		walker+=2;

	        *traw= DS1820_GetTempRaw();

		version_buf[3] ++;
	   }
	   while ( DS1820_FindNextDevice() && walker<60);
	}

	// usbState must not be touched until next USBHandleBusy returned;
	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
}

// **************************** TEMP CONTROL END ******************************
// ****************************************************************************

/******************************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None

 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/

void add_ch26data_bit(unsigned char rbBit)
{
	unsigned char idx=0;
	struct ChallengerLine* cl = NULL;

	set_bit(ch26data.usedBitMask,rbBit);
	ch26data.bit2line[rbBit]=ch26data.usedBitCount;
	cl = &ch26data.line[ch26data.usedBitCount++];
	cl->lineId = rbBit;

	// Hard code for a special case
	if(rbBit==4 || rbBit==5){

		// Read distances from EEPROM
		for(idx=0;idx<4;idx++){
			cl->dist[idx].shift=read_EEPROM(EEPROM_SHIFT_8BYTES+idx+(rbBit-4)*4);
			}
		}
}

void standalone_start() {

	stop_interrupts();
	ioc_stop();

	BITFLAG1(standalone);

	initChallengerData(&ch26data);

	// Setup status packet scanner
	add_ch26data_bit(4);
	add_ch26data_bit(5);

	BITFLAG1(scanner);
	
	start_interrupts();
	}

// On first ping
void standalone_stop(){
	
	BITFLAG0(standalone);
	#ifndef	MULTI_BUZZER
	buzzer_off();
	ledS.buzzerAlwaysOn=0;
	ledS.buzzerPeriod=0;
	ledS.buzzerCounter=0;
	#endif
	initChallengerData(&ch26data);
	}

#ifndef	MULTI_BUZZER
void buzzer_wait_for_5beeps()
{
	ledS.buzzerPeriod=0;
	ledS.buzzerAlwaysOn=0;
	ledS.buzzerCounter=5;

	while(ledS.buzzerCounter) Nop();
}

void buzzer_on(){
	T2CONbits.TMR2ON=1;
	}

void buzzer_off(){
	T2CONbits.TMR2ON=0;
	}
#endif

void grabMinDistance()
{
	unsigned char i=0;
	unsigned char j=0;
	unsigned char lineIdx=0;
	unsigned char dist=0xFF;
	struct ChallengerLine* l = NULL;

	ledS.minDistance=0xFF;
	// Cycle trough RB7-RB4 (interrupt pins that connect to 18F2550 Interrupt-on-change IOC feature)
	for(i=7;i>=4;i--){

		if((1<<i) & ch26data.usedBitMask) {

			lineIdx=ch26data.bit2line[i];

			l = &ch26data.line[lineIdx];

			if(l->signalTimeout) continue; // Line disconnected.

			for(j=0;j<l->usedHeadsCount;j++){
				dist=l->dist[j].dist;
				if(ledS.minDistance>dist) ledS.minDistance=dist;
				}
			}
		}
}

#ifndef	MULTI_BUZZER
void buzzer_distance()
{
	buzzer_off();

	ledS.buzzerAlwaysOn=0;
	ledS.buzzerPeriod=0;

	if(ledS.minDistance<30) { ledS.buzzerAlwaysOn=1; return; }
	else if(ledS.minDistance>200) { ledS.buzzerPeriod=0; ledS.buzzerAlwaysOn=0; ledS.buzzerCounter=0; }
	else ledS.buzzerPeriod=(ledS.minDistance / 20);

	buzzer_off();
}
#endif

unsigned char read_EEPROM(unsigned int addr)
{
	unsigned char eeTemp;

	EEADR = addr;
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;
	eeTemp = EEDATA;
	while(EECON1bits.RD) Nop();

	return eeTemp;
}

int set_EEPROM(unsigned int addr,unsigned char val)
{
	unsigned char attempt=0, _pa = PORTA,j=0;

	// 1. Make sure USB is connected
	if(USBDeviceState < CONFIGURED_STATE) return 0;
	if(USBSuspendControl==1) return 0;

	// 2. No way bootloader mode can be initiated while receiving data
	if(!flags.no_feed){
		// We are currently receiving data. That can be electromagnetically
		// unsafe to touch EEPROM at this point or this event could be caused by
		// EM noise or parasite power itself
		return 0;
		}

	colorOn(RED);
	colorOn(GREEN);
	colorOn(WHITE);

	for(attempt=0;attempt<3;attempt++) {
		// Write
		EEADR	= addr;
		EEDATA	= val;Nop();
		Nop();
		EECON1bits.EEPGD = 0;
		EECON1bits.CFGS = 0;
		EECON1bits.WREN = 1;
		INTCONbits.GIE = 0;
		EECON2 = 0x55;
		EECON2 = 0xAA;
		EECON1bits.WR = 1;
		while(EECON1bits.WR) Nop();
		INTCONbits.GIE = 1;
		EECON1bits.WREN = 0;
		for(j=0;j<255;j++) { Nop(); }
		// Veriry write
		if(read_EEPROM(addr)==val) {
			PORTA=_pa;
			return 1; // Okay! Looks good.
			}
		// write again .. ^
		}

	PORTA=_pa;
	return 0; // Fail
}

void pio_read_shift()
{
	unsigned char sz=sizeof(version_buf), i =0;

	version_buf[i]=CMD_READ_SHIFT;
	for(i=1;i<=8;i++) version_buf[i]=read_EEPROM(EEPROM_SHIFT_8BYTES+i-1);

	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
}

#ifndef	MULTI_BUZZER
void pio_read_buzzer()
{
	unsigned char sz=sizeof(version_buf), i =0, walker=0, buzzerFlags=0x0;

	buzzerFlags=read_EEPROM(EEPROM_BUZZER_FLAGS);

	for(i=sz-1;i>0;i--) version_buf[i]='B';
	version_buf[walker++]=CMD_READ_BUZZER;
	version_buf[walker++]=read_EEPROM(EEPROM_BUZZER_PR2);
	version_buf[walker++]=get_bit(buzzerFlags,0);
	version_buf[walker++]=get_bit(buzzerFlags,1);
	version_buf[walker++]=read_EEPROM(EEPROM_BUZZER_CCPR1L);
	version_buf[walker++]=get_bit(buzzerFlags,2);
	version_buf[walker++]=get_bit(buzzerFlags,3);

	USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(unsigned char*)(&version_buf[0]),64);
}
#endif

void save_shift(unsigned char arr[])
{
	// arr[0] - cmd
	// arr[1] - idx of the head
	// Where channel 4 - A,B,C,D, than Channel 5 - A,B,C,D
	set_EEPROM(EEPROM_SHIFT_8BYTES+arr[1],arr[2]);
}

#ifndef	MULTI_BUZZER
void save_buzzer(unsigned char arr[])
{
	unsigned char buzzerFlags = 0x0;
	unsigned char walk=1;
//	command[0]=cmd;

	// Encode packet information in here  //
//	command[1]=t.PR2;
	set_EEPROM(EEPROM_BUZZER_PR2,arr[walk++]);

//	command[2]=t.T2CON_T2CKPS0;
	if(arr[walk++]) set_bit(buzzerFlags,0); else clear_bit(buzzerFlags,0);
//	command[3]=t.T2CON_T2CKPS1;
	if(arr[walk++]) set_bit(buzzerFlags,1); else clear_bit(buzzerFlags,1);

//	command[5]=v.CCPR1L;
	set_EEPROM(EEPROM_BUZZER_CCPR1L,arr[walk++]);

//	command[6]=v.CCP1CON_DC1B0;
	if(arr[walk++]) set_bit(buzzerFlags,2); else clear_bit(buzzerFlags,2);
//	command[7]=v.CCP1CON_DC1B1;
	if(arr[walk++]) set_bit(buzzerFlags,3); else clear_bit(buzzerFlags,3);

	set_EEPROM(EEPROM_BUZZER_FLAGS,buzzerFlags);
}

// no multi-buzzer
void setup_buzzer()
{
	unsigned char buzzerFlags = read_EEPROM(EEPROM_BUZZER_FLAGS);

	ledS.buzzerAlwaysOn=0;
	ledS.buzzerCounter=0;
	ledS.buzzerPeriod=0;
	ledS.minDistance=0xFF;

	// Discharge timer
	T2CONbits.TMR2ON=0;

	PR2=read_EEPROM(EEPROM_BUZZER_PR2);//0xB8; // period

	T2CONbits.T2CKPS0=get_bit(buzzerFlags,0);//1;//  TMR2 prescale
	T2CONbits.T2CKPS1=get_bit(buzzerFlags,1);//1;

	// Output
	//--TRISBbits.TRISB3=0; // CCP2
	TRISCbits.TRISC2=0; // CCP1

	//--CCPR2L=0xA4; // duty cycle CCP
	//--CCP2CONbits.DC2B0=1;
	//--CCP2CONbits.DC2B1=0;

	CCPR1L=read_EEPROM(EEPROM_BUZZER_CCPR1L);//0x70; // duty cycle CCP1

	CCP1CONbits.DC1B0=get_bit(buzzerFlags,2);//0;
	CCP1CONbits.DC1B1=get_bit(buzzerFlags,3);;

	// Start CCP2
	//--CCP2CONbits.CCP2M0=1;
	//--CCP2CONbits.CCP2M1=0;
	//--CCP2CONbits.CCP2M2=1;
	//--CCP2CONbits.CCP2M3=1;

	// Start CCP1
	CCP1CONbits.CCP1M0=1;
	CCP1CONbits.CCP1M1=0;
	CCP1CONbits.CCP1M2=1;
	CCP1CONbits.CCP1M3=1;
}
#endif // MULTI_BUZZER

void main(void)
{
	InitializeSystem();

	#if defined(USB_INTERRUPT)
	USBDeviceAttach();
	#endif

	while(1)
	{
		// Check bus status and service USB interrupts.
		USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
			  // this function periodically.  This function will take care
			  // of processing and responding to SETUP transactions
			  // (such as during the enumeration process when you first
			  // plug in).  USB hosts require that USB devices should accept
			  // and process SETUP packets in a timely fashion.  Therefore,
			  // when using polling, this function should be called
			  // frequently (such as once about every 100 microseconds) at any
			  // time that a SETUP packet might reasonably be expected to
			  // be sent by the host to your device.  In most cases, the
			  // USBDeviceTasks() function does not take very long to
			  // execute (~50 instruction cycles) before it returns.

		// Application-specific tasks.
		// Application related code may be added here, or in the ProcessIO() function.
		ProcessIO();

		//
		// It will process commands one at time accordingly to flags
		//
		ProcessCommands();

	}//end while

	stop_timer3();

}//end main

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/

void ConfigureIOPinsForLowPower()
{
	// Stop all related interrupts
	stop_interrupts();
	stop_timer3();

	// Turn off the buzzer
	standalone_stop();

	// Hide leds
	TRISA=0x0; Nop();Nop();Nop(); PORTA=0x0;
}

void SaveStateOfAllInterruptEnableBits()
{
}

void DisableAllInterruptEnableBits()
{
}

void EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro()
{
	PIR2bits.USBIF=0;
	PIE2bits.USBIE=1;
	UIEbits.ACTVIE=1;
	UIRbits.ACTVIF=1;
}

void RestoreStateOfAllPreviouslySavedInterruptEnableBits()
{
}

void RestoreIOPinsToNormal()
{
	// Just reset all the user functions and set thing to standalone mode
	UserInit();
}

void USBCBSuspend(void)
{
/*
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.  If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:

	ConfigureIOPinsForLowPower();
	SaveStateOfAllInterruptEnableBits();
	DisableAllInterruptEnableBits();
	EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is
	//cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause
	//things to not work as intended.
*/
}


/******************************************************************************
 * Function:        void _USB1Interrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the USB interrupt bit is set
 *					In this example the interrupt is only used when the device
 *					goes to sleep when it receives a USB suspend command
 *
 * Note:            None
 *****************************************************************************/
#if 0
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
    #if !defined(self_powered)
        if(U1OTGIRbits.ACTVIF)
        {
            IEC5bits.USB1IE = 0;
            U1OTGIEbits.ACTVIE = 0;
            IFS5bits.USB1IF = 0;

            //USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
            USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
            //USBSuspendControl = 0;
        }
    #endif
}
#endif

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *
 *					This call back is invoked when a wakeup from USB suspend
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// a few milliseconds of wakeup time, after which the device must be
	// fully back to normal, and capable of receiving and processing USB
	// packets.  In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).

/*
	RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	RestoreIOPinsToNormal();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
*/
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void)
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

	// Typically, user firmware does not need to do anything special
	// if a USB error occurs.  For example, if the host sends an OUT
	// packet to your device, but the packet gets corrupted (ex:
	// because of a bad connection, or the user unplugs the
	// USB cable during the transmission) this will typically set
	// one or more USB error interrupt flags.  Nothing specific
	// needs to be done however, since the SIE will automatically
	// send a "NAK" packet to the host.  In response to this, the
	// host will normally retry to send the packet again, and no
	// data loss occurs.  The system will typically recover
	// automatically, without the need for application firmware
	// intervention.

	// Nevertheless, this callback function is provided, such as
	// for debugging purposes.
}


/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *****************************************************************************/
void USBCBCheckOtherReq(void)
{
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *****************************************************************************/
void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end


/******************************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This
 *					callback function should initialize the endpoints
 *					for the device's usage according to the current
 *					configuration.
 *
 * Note:            None
 *****************************************************************************/
void USBCBInitEP(void)
{
    USBEnableEndpoint(USBGEN_EP_NUM,USB_OUT_ENABLED|USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,HOST_COMMAND_SIZE);
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function should only be called when:
 *
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.
 *
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            Interrupt vs. Polling
 *                  -Primary clock
 *                  -Secondary clock ***** MAKE NOTES ABOUT THIS *******
 *                   > Can switch to primary first by calling USBCBWakeFromSuspend()

 *                  The modifiable section in this routine should be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of 1-13 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at lest 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void)
{
    static WORD delay_count;

    USBResumeControl = 1;                // Start RESUME signaling

    delay_count = 1800U;                // Set RESUME line for 1-13 ms
    do
    {
        delay_count--;
    }while(delay_count);
    USBResumeControl = 0;
}

BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
  switch(event)
  {
    case EVENT_CONFIGURED:
      USBCBInitEP();
      break;
    case EVENT_SET_DESCRIPTOR:
      USBCBStdSetDscHandler();
      break;
    case EVENT_EP0_REQUEST:
      USBCBCheckOtherReq();
      break;
    case EVENT_SOF:
      USBCB_SOF_Handler();
      break;
    case EVENT_SUSPEND:
      USBCBSuspend();
      break;
    case EVENT_RESUME:
      USBCBWakeFromSuspend();
      break;
    case EVENT_BUS_ERROR:
      USBCBErrorHandler();
      break;
    case EVENT_TRANSFER:
      Nop();
      break;
    default:
      break;
  }
  return TRUE;
}

/** EEPROM *********************************************/
#pragma romdata staff=0xF00000

const rom unsigned char rom_shifts[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/** EOF main.c ***************************************************************/
