//
// $Id: challenger26.h,v 1.12 2012/03/26 16:36:26 dmi Exp $
//

/////////////////////////////////////////////////////////////////////////////////
//
// API description
//
// ChallengerLine is super structure that controls all operations on the line.
// To initialize it, call [1] initChallengerLine(ChallengerLine*);
// That will pretty much do all the initialization job.
//
// During the pin change interrupt, analyze when start conditions are met.
// for the 0-impulse, check wheather [2] widthSilence(duration in cycles)
// returns true. Once long low impulse is met, start processing.
//
// ______________________________|''''''|_______
// widthSilence()==true           Thigh   Tlow
//
// Call [3] writeToChallengerData(ChallengerLine*, Tlow, Thigh); It returns
// number of bits collected. When the number reaches 24(25th is control unused
// bit) process starts over.
//
// [4] isValidChallengerPacket(ChallengerLine*) checks if packet is legally
// received (with parity and the redundant bit check); That function must be
// called upon receiption of the packet
//
///////////////////////////////////////////////////////////////////////////////

#ifndef	CHALLENGER26_H
#define	CHALLENGER26_H

#include	"p18f2550.h"

// Affects size of the data structure
#define	NUM_HEADS_PER_PARKTRONIC 4

// Reserved 24 bit pack for storing a package
struct ChallengerPack24 {

	// Actual construction of challenger packet

	// ID
	unsigned char id: 4;
	unsigned char invertedId: 4;

	// DISTANCE
	unsigned char value;
	unsigned char invertedValue;
	//unsigned char reserved: 1;
	};

// Keep parsed protocol data here
struct DistData {
	// 4 bits because id word only has 4 bits (look in ChallengerPack24)
	unsigned char headId: 4;
	unsigned char dist;

	// Shift will be substracted out of every distance
	unsigned char shift;

	// Multi-buzzer support. Id of a buzzer
	unsigned char buzzerId: 4;
	};

// Resulted structure
struct ChallengerLine {

	// Time mark of the last change. We need that to measure the time difference
	unsigned long timeOfLastChange;

	// Interrupt processing: store last low-impulse width
	unsigned long prevInterval;

	// Mark when line is idle
	unsigned char signalTimeout:1;

	// Id of the challenger line (i.e. controller pin id)
	// 3 bits fit from 0 to 7
	unsigned char lineId: 3;

	// Data for the line - 5 bits fit from 0 to 31 (11,111)
	// Number of bits collected in the challenger packet
	unsigned char bitCount: 5;

	// There can only be up to 8 heads (0 to 111)
	unsigned char usedHeadsCount: 3;

	// Union collects single bits in challenger packet
	union {
		// 3-byte structure
		struct ChallengerPack24 cp24;
		// Bit by bit recording array
		unsigned char bitArr[3];
		} data;

	struct DistData dist[NUM_HEADS_PER_PARKTRONIC];
	};

struct ChallengerDeliveryPackage {
	unsigned char lineId;
	unsigned char usedHeadsCount;
	struct DistData dist[NUM_HEADS_PER_PARKTRONIC];
	};


enum T_IMPULSE { TI_ZERO, TI_ONE, TI_INVALID };

#endif //CHALLENGER26_H
