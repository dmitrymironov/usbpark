//
// $Id: challenger26.c,v 1.14 2012/03/26 16:36:25 dmi Exp $
//
#define	MULTI_BUZZER

#include	"challenger26.h"

// Prototypes
int isValidChallengerPacket(const struct ChallengerLine*);
void processChallengerPacket(const struct ChallengerLine*);
unsigned char widthSilence(unsigned long l);

// returns number of bits read
unsigned char writeToChallengerData(struct ChallengerLine*,unsigned long l, unsigned long h);

void initDistData(struct DistData*);
void initChallengerLine(struct ChallengerLine*);

//
// Challenger 26 logic
//

#define	CHALLENGER_0_MIN	36000
#define	CHALLENGER_0_MAX	156000
#define	CHALLENGER_1_MIN	160000
#define	CHALLENGER_1_MAX	800000
#define	CHALLENGER_IDLE_MIN	2000000
#define	CHALLENGER_IDLE_MAX	10800000
#define	CHALLENGER_NOISE	8000
#define	CHALLENGER_DATA_TIMEOUT	(CHALLENGER_IDLE_MAX*10)

unsigned char isNoise(unsigned long l)
{
	return l <= CHALLENGER_NOISE;
}

unsigned char widthLow(unsigned long l)
{
	return l>=CHALLENGER_0_MIN && l<=CHALLENGER_0_MAX;
}

unsigned char widthHigh(unsigned long h)
{
	return h>=CHALLENGER_1_MIN && h<=CHALLENGER_1_MAX;
}

unsigned char widthDataTimeout(unsigned long l)
{
	return l>CHALLENGER_DATA_TIMEOUT;
}

unsigned char widthSilence(unsigned long l)
{
	return l>CHALLENGER_IDLE_MIN && l<=CHALLENGER_IDLE_MAX;
}

int detectImpulse(unsigned long l, unsigned long h)
{
	if(widthLow(l) && widthLow(h)) return TI_ZERO;

	if(widthHigh(l) && widthHigh(h)) return TI_ONE;

	// Main Challenger 26 heuristic is here
	return TI_INVALID;
}

#define	set_bit(val,b) 		(val|=(1<<b))
#define	clear_bit(val,b)	(val&=~(1<<b))

void writeBit(unsigned char* a,unsigned char bn, unsigned char val)
{
	// Invert bit order for PC
	unsigned char word_num = bn / 8, bit_num = (bn - word_num*8);

	if(val) set_bit(a[word_num],bit_num);
	else clear_bit(a[word_num],bit_num);
}

// returns number of bits read
unsigned char writeToChallengerData(
	struct ChallengerLine* c,
	unsigned long l,
	unsigned long h)
{
	int ti=TI_INVALID;

	#if	defined	LINE_TIMEOUT
	c->signalTimeout = 0;
	#endif

	// ERROR; will not collect any further
	if(c->bitCount>=24) return c->bitCount;

	ti=detectImpulse(l,h);

	if(ti==TI_INVALID){
		// We got a wrong impulse (probably noise)
		c->prevInterval=0;
		return c->bitCount=0;
		}

	if(ti==TI_ZERO) writeBit(c->data.bitArr,c->bitCount,0);
	else writeBit(c->data.bitArr,c->bitCount,1);

	return ++c->bitCount;
}

int isValidChallengerPacket(const struct ChallengerLine* c)
{
	if(c->bitCount<24) return 0;

	// Check inverted values
	return
		0xf==(c->data.cp24.id^c->data.cp24.invertedId)
					&&
		0xff==(c->data.cp24.value^c->data.cp24.invertedValue);
}

// it's like make_in_packet, but instead of sending raw data,
// we will compress and send distances to the heads of parktronic
// interfaces connected; So it's like a probe of current distance
// condition.
// By using this approach, USB traffic can be dramatically minimized
// Instead of sending data scan, we're sending only processed data
unsigned char reverse4BitOrder(unsigned char arg)
{
	unsigned char i=0, rv=0, mask=1, invertedMask=0x8; // bin 1000,0000

	for(i=0;i<4;i++){
		if(mask&arg) rv |= invertedMask;
		mask = mask << 1;
		invertedMask = invertedMask >> 1;
		}

	return rv;
}

unsigned char reverse8BitOrder(unsigned char arg)
{
	unsigned char i=0, rv=0, mask=1, invertedMask=0x80; // bin 1000,0000

	for(i=0;i<8;i++){
		if(mask&arg) rv |= invertedMask;
		mask = mask << 1;
		invertedMask = invertedMask >> 1;
		}

	return rv;
}

void processChallengerPacket(const struct ChallengerLine* c)
{
	unsigned char i=0, idist=0;
	unsigned char headId=reverse4BitOrder(c->data.cp24.invertedId);

	// ERROR
	// That must not happen. If we return here, someone is trying
	// to process packet that is not yet fully received
	if(c->bitCount<24) return;

	// Try finding a proper id of a head
	for(i=0;i<c->usedHeadsCount;i++){

		// LINEAR SEARCH.
		// Stop when id matches
		if(c->dist[i].headId==headId) break;
		}

	if(i>=NUM_HEADS_PER_PARKTRONIC){
                                                                                                                                               		// ERROR
		// That really shoud never happen. It means
		// that we detected more heads that has been reserved
		// for the storage

		c->prevInterval=0; // clears a packet
		c->bitCount=0;

		return;
		}

	// Detected new id of a head for this session
	if(i>=c->usedHeadsCount){
		c->usedHeadsCount++;
		c->dist[i].headId=headId;		
		// Multi-buzzer support: hard coded for challenger lines 4 and 5
		// quick and dirty
		// ---- 1->1 coding
		c->dist[i].buzzerId=(c->lineId-4)*4+headId;		
		// 2->1 (join channels) 0=(4:0,4:2) 1=(4:1,4:3) 2=(5:0,5:2) 3=(5:1,5:3)
		#if	0
		switch(headId){
			case 0: case 2: c->dist[i].buzzerId=(c->lineId-4)*2; break;
			case 1: case 3: c->dist[i].buzzerId=(c->lineId-4)*2+1; break;
			}
		#endif
		}

	idist=reverse8BitOrder(c->data.cp24.value);

	// Adjust distance accordingly to user pre-programmed shift
	// stored in EEPROM
	if(idist<c->dist[i].shift) c->dist[i].dist=0;

		else c->dist[i].dist=idist-c->dist[i].shift;

	// Clear things to prepare for next packet
	c->prevInterval=0;
	c->bitCount=0;

	
	#ifdef	0
	// MULTI_BUZZER
	// Multi-buzzer support. Update multi-buzzer distance
	// 0=(4:0,4:1) 1=(4:2,4:3) 2=(5:0,5:1) 3=(5:2,5:3)
	#define	MIN(a,b) (a<b?a:b)
	idist=0;
	switch(c->dist[i].buzzerId){
		case 0: case 2: idist=MIN(c->dist[0 /* 4:0,4:2 or 5:0,5:2 */].dist,c->dist[2].dist); break;
		case 1: case 3: idist=MIN(c->dist[1].dist,c->dist[3].dist); break;
		}
	setBuzzDevDistance(c->dist[i].buzzerId,idist);
	#endif
	
	setBuzzDevDistance(c->dist[i].buzzerId,c->dist[i].dist);
}

//
// Intialize data (with 0-s pretty much)
//
void initDistData(struct DistData* d)
{
	d->headId=0;
	d->dist=0xFF;
	d->shift=0x0;
	d->buzzerId=0x0;
}

void initChallengerLine(struct ChallengerLine* c)
{
	unsigned char i=0;

	c->lineId=0;
	c->bitCount=0;
	c->usedHeadsCount=0;

	#if	defined	LINE_TIMEOUT
	c->signalTimeout=0;
	#endif

	for(i=0;i<NUM_HEADS_PER_PARKTRONIC;i++) initDistData(&c->dist[i]);

	c->data.bitArr[0]=0;
	c->data.bitArr[1]=0;
	c->data.bitArr[2]=0;

	c->timeOfLastChange=0;
}

////////////////// CHALLENGER_26_C
