
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ios>
#include <sys/stat.h>
#include "typedefs.h"
#include "misc.h"

// Saves a PIC to TGA-file.
int SaveTGA(PIC* pic, char* name, int style)
{
	int i = 0, j = 0, k = 0, l = 0, x = 0, y = 0;

	fstream tga;
	tga.open(name, BINARY | WRITE | TRUNC);
	if (tga.fail()) {
		tga.close();
		return (PIC_ERROPEN);
	}

	x = pic->x * pic->xn;
	y = pic->y * pic->yn;

	BYTE header[] = {
		0, 0, 2,
		0, 0, 0, 0, 0,
		0, 0, 0, 0,
		x & 0xff, x >> 8, y & 0xff, y >> 8,
		24, 0
	};

	BYTE* temp = new BYTE[x * y * 3];

	for (i = 0; i < y; i++) {
		k = x * 3 - 1;
		l = i * x * 3;
		for (j = 0; j < (x * 3); j += 3) {
			temp[l + j] = pic->pic[l + k - 2];
			temp[l + j + 1] = pic->pic[l + k - 1];
			temp[l + j + 2] = pic->pic[l + k];
			k -= 3;
		}
	}

	tga.write((char*)&header, sizeof(header));
	tga.write((char*)temp, x * y * 3);
	delete temp;

	tga.close();

	return (PIC_OK);
}

// Load a PCX-file into buffer and set size into x and y.
// xn and yn are set to 1.
int LoadPCX(PIC *pic, char *filename, int style)
{
	int xsize, ysize, tavu1, tavu2, position = 0;
	FILE *handle = fopen(filename, "rb");
	BYTE *tmp;
	BYTE *buffer;
	int fpos;
	struct stat st;

	if (handle == (FILE *) - 1) {
		return (PIC_ERROPEN);
	}

    fstat(fileno(handle), &st);
	tmp = new BYTE[st.st_size];

	fread(tmp, st.st_size, 1, handle);
	fclose(handle);

	fpos = 8;

	xsize = tmp[fpos] + (int)(tmp[fpos + 1] << 8) + 1; fpos += 2;
	ysize = tmp[fpos] + (int)(tmp[fpos + 1] << 8) + 1; fpos += 2;

	buffer = new BYTE[xsize * ysize];

	fpos = 128;
	while (position < (xsize * ysize)) {
		tavu1 = tmp[fpos++];
		if (tavu1 > 192) {
			tavu2 = tmp[fpos++];
			for (; tavu1 > 192; tavu1--) buffer[position++] = tavu2;
		} else buffer[position++] = tavu1;
	}

	pic->x = xsize;
	pic->xn = 0;
	pic->y = ysize;
	pic->yn = 0;
	pic->pic = buffer;

	delete tmp;

	return (PIC_OK);
}

int ConvTextToInt(char* text, int start, int lenght)
{
	int multiplier = 1, count = 0, pw = 0, result = 0, stop = 0;
	if (text[start] == '-') {
		multiplier = -1;
		count++;
	}
	int loop0 = lenght;
	for (loop0 = lenght; loop0 > 0; loop0--) {
		if ((text[start + count + loop0 - 1] == ' ') && (pw != 0)) stop = 1;
		if ((text[start + count + loop0 - 1] != ' ') && (stop == 0)) {
			result += (int)(pow(10, pw) * (text[start + count + loop0 - 1] - CHARZERO));
			pw++;
		}
	}
	result *= multiplier;
	return (result);
}