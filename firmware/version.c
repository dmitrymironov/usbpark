//
// $Id: version.c,v 1.4 2010/10/01 18:47:04 dmi Exp $ implement automatic versioning for Firmware
//

#include	"version.h"

rom const char vs[]=VERSION_STRING;

void str2ram(static char *dest, static char rom* src)
{
  while((*dest++ = *src++) != '\0');
}

void get_firmware_version(char* d,unsigned char lg){
	unsigned char n=sizeof(vs),i=0;	
	d[-1]=n;
	d[-2]=lg;
	str2ram(d,vs);
	}
