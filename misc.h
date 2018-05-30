#ifndef MISC_H
#define MISC_H

#include "typedefs.h"

#ifndef FOR_PIC
#define FOR_PIC
typedef struct forPIC{
	WORD x;
	WORD y;
	WORD xn;
	WORD yn;
	BYTE* pic;
	BYTE* palette;
	bool pic_enable;
	bool palette_enable;
	BYTE depth;
} PIC;
#endif

int PrintFile(char *text,char *name,int flags);
int PrintFileInt(int nInt,char *name,int flags);

// Save and load routines for pictures
// First defines for return codes and such
#define PIC_OK		0
#define PIC_ERROR	1
#define PIC_ERROPEN	2

// TGA
int SaveTGA(PIC*,char*,int);
int LoadTGA(PIC*,char*,int);
// PCX
int SavePCX(PIC*,char*,int);
int LoadPCX(PIC*,char*,int);

int ConvTextToInt(char* text, int start, int lenght);

#endif
