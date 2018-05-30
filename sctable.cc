/*

    Starcraft statistics table editor

*/

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "misc.h"
#include "cptables.h"

#define VERSION "StarCraft statistics table editor v0.78"

#define MAX_PATH 260

#define TAB_ADD 1
#define TAB_REPLACE 2
#define TAB_SUB 4

#define INVISIBLE 0
#define VISIBLE 255
#define EMPTY 85

#define PAGE_OVERVIEW 0
#define PAGE_UNITS 1
#define PAGE_STRUCTURES 2
#define PAGE_RESOURCES 3

#define PAGES_Y 25
#define PAGEOVERVIEW_ENABLE 247+PAGES_Y*640
#define PAGEUNITS_ENABLE 370+PAGES_Y*640
#define PAGESTRUCTURES_ENABLE 439+PAGES_Y*640
#define PAGERESOURCES_ENABLE 552+PAGES_Y*640

#define NAMESTART_X 36
#define FIRSTNAME 80*640+NAMESTART_X
#define NEXTNAME 40*640
#define NAMEMAX_X 184
#define SPACE_X 6
#define AFTERSPACE_X 1
#define FONT_HEIGHT 16

#define NUMBERSTART_X 192
#define FIRSTNUMBER 85*640+NUMBERSTART_X
#define NEXTNUMBER_Y 40*640
#define NEXTNUMBER_X 104
#define NUMBER_HEIGHT 7
#define NUMBERMAX_X 90
#define NUMBERS_END 248

#define MAX_ENTRYS 64

#define PROTOSS 0
#define TERRAN 1
#define ZERG 2

#define RACESIZE_X 26
#define RACESIZE_Y 12
#define RACESTART_X 45
#define FIRSTRACE 104*640+RACESTART_X
#define NEXTRACE 40*640

typedef struct forSTATS {
	char name[32];
	DWORD protoss;
	DWORD terran;
	DWORD zerg;
	// Results
	DWORD units;
	DWORD structures;
	DWORD resources;

	DWORD produced;
	DWORD killed;
	DWORD lost;

	DWORD constructed;
	DWORD razed;
	DWORD slost;

	DWORD gas;
	DWORD minerals;
	DWORD spent;
} STATS;

/*
    Name resolver
*/
int name_resolver(BYTE *pic, char *name, int number)
{
	// Variables
	int j, i;
	int cnt_x = 0;
	int cnt_space = 0;
	int cnt_cmpx = 0;
	int cnt_matches = 0;
	int cnt_mismatches = 0;
	int coords = FIRSTNAME + NEXTNAME * number;
	int cnt_char = 0;
	int cnt_chars = 0;
	int cnt_xx = 0;
	bool find_start = false;
	name[0] = '\0';

	// This lookup-table is used to find out the ASCII-code of the characters
	BYTE name_lookup[] = {
		"ABCDEFGHJKLMNOPQRSTUVWXYZÅÄÖabcdefghjklmnopqrstuvwxyzåäö?1234567890????!#?%()?;I@£${[]}~äëöüïÿáéíóúýàèìòùi"
	};

	// Set up the compare table
	PIC cmp;
	cmp.x = 1000;
	cmp.y = FONT_HEIGHT;
	cmp.pic = (BYTE*)&fonts_lookup;

	/*
	    cmp.pic=LoadPCX( "cmpfonts.pcx", (int*)&cmp.x, (int*)&cmp.y);
	    if (!cmp.pic) return (1);           // If failed then return

	    fstream cpt;
	    cpt.open("cpt.h",WRITE|TRUNC);
	    int cnt=0;
	    for (i=0; i<FONT_HEIGHT; i++)
	    {
	        for (j=0; j<cmp.x; j++)
	        {
	            cnt++;
	            if (cnt>16)
	            {
	                cpt<<endl;
	                cnt=0;
	            }
	            cpt<<(int)cmp.pic[j+i*cmp.x]<<", ";
	        }
	    }
	    cpt<<endl<<" End."<<endl;
	    cpt.close();
	*/

	// Start comparing the chars and find out the name
	while (cnt_x < NAMEMAX_X) {
		// See if there is an empty vertical line
		if (find_start == true) {
			for (j = 0; j < FONT_HEIGHT; j++) {
				if (pic[coords + cnt_x + j * 640] != INVISIBLE) {
					cnt_space = 0;
					find_start = false;
					break;
				}
			}
			if (cnt_space > SPACE_X) {
				name[cnt_chars] = ' ';
				name[cnt_chars + 1] = '\0';
				cnt_space = 0;
				cnt_chars++;
				cnt_x++;
				cnt_xx = cnt_x;
				continue;
			}
			if (find_start == true) {
				cnt_space++;
				cnt_x++;
				cnt_xx = cnt_x;
				continue;
			}
		}

		// Compare current vertical line to current character in compare table
		for (j = 0; j < FONT_HEIGHT; j++) {
			if (pic[coords + cnt_x + j * 640] == cmp.pic[cnt_cmpx + j * cmp.x]) {
				cnt_matches++;
			} else {
				cnt_mismatches++;
			}
		}

		// Increase current vertical line counter
		cnt_x++;

		// If this was last vertical line in compare table, then see if the char
		// compared matched to the compare table enough well
		if ((cnt_cmpx + 1) >= cmp.x || cmp.pic[cnt_cmpx + 1] == EMPTY) {
			if (cnt_mismatches == 0) {
				// Matching char found. Add it to the name-string
				name[cnt_chars] = name_lookup[cnt_char];
				name[cnt_chars + 1] = '\0';
				cnt_chars++;
				cnt_char = -1;
				cnt_cmpx = -1;
				cnt_xx = cnt_x;
				find_start = true;
			} else if ((cnt_cmpx + 1) >= cmp.x) {
				cnt_xx++;
				cnt_cmpx = -1;
				cnt_char = -1;
			} else {
				while (cmp.pic[cnt_cmpx + 1] == EMPTY) {
					cnt_cmpx++;
					if ((cnt_cmpx + 1) >= cmp.x) {
						cnt_cmpx = -1;
						cnt_char = -1;
						cnt_xx++;
						break;
					}
				}
			}
			cnt_matches = 0;
			cnt_mismatches = 0;
			cnt_x = cnt_xx;
			cnt_char++;
			cnt_space = 0;
		}

		// Increase compare table vertical line counter
		cnt_cmpx++;
	}

	// Remove any spare spaces from end of the name
	i = strlen(name) - 1;
	for (; i > 0; i--) {
		if (name[i] != ' ') break;
		name[i] = '\0';
	}

	return (0);
}

/*
    Number resolver
*/
int number_resolver(BYTE *pic, DWORD *numb, int number, int select)
{
	// Variables
	int j, i;
	int cnt_x = 0;
	int cnt_cmpx = 0;
	int cnt_matches = 0;
	int cnt_mismatches = 0;
	int coords = FIRSTNUMBER + NEXTNUMBER_Y * number + NEXTNUMBER_X * select;
	int cnt_char = 0;
	int cnt_chars = 0;
	int cnt_xx = 0;
	bool find_start = true;
	char numberch[11] = "\0";

	// This lookup-table is used to find out the ASCII-code of the numbers
	BYTE number_lookup[] = {
		"1234567890"
	};

	// Set up the compare table
	PIC cmp;
	cmp.x = 64;
	cmp.y = 7;
	cmp.xn = 1;
	cmp.yn = 1;
	cmp.pic = (BYTE*)&numbers_lookup;

	// Start comparing the chars and find out the number
	while (cnt_x < NUMBERMAX_X) {
		// See if there is an empty vertical line
		if (find_start == true) {
			for (j = 0; j < NUMBER_HEIGHT; j++) {
				if (pic[coords + cnt_x + j * 640] != INVISIBLE) {
					find_start = false;
					break;
				}
			}
			if (find_start == true) {
				cnt_x++;
				cnt_xx = cnt_x;
				continue;
			}
		}

		// Compare current vertical line to current character in compare table
		for (j = 0; j < NUMBER_HEIGHT; j++) {
			if (pic[coords + cnt_x + j * 640] == cmp.pic[cnt_cmpx + j * cmp.x]) {
				cnt_matches++;
			} else {
				cnt_mismatches++;
			}
		}

		// Increase current vertical line counter
		cnt_x++;

		// If this was last vertical line in compare table, then see if the char
		// compared matched to the compare table enough well
		if (cmp.pic[cnt_cmpx + 1] == EMPTY) {
			if (cnt_mismatches == 0) {
				// Matching char found. Add it to the number-string
				numberch[cnt_chars] = number_lookup[cnt_char];
				numberch[cnt_chars + 1] = '\0';
				cnt_chars++;
				cnt_char = -1;
				cnt_cmpx = -1;
				cnt_xx = cnt_x;
				find_start = true;
			} else {
				while (cmp.pic[cnt_cmpx + 1] == EMPTY) {
					cnt_cmpx++;
					if (cmp.pic[cnt_cmpx + 1] == NUMBERS_END) {
						cnt_cmpx = -1;
						cnt_char = -1;
						cnt_xx++;
						break;
					}
				}
			}
			cnt_matches = 0;
			cnt_mismatches = 0;
			cnt_x = cnt_xx;
			cnt_char++;
		}

		// Increase compare table vertical line counter
		cnt_cmpx++;
	}

	// Then convert the number-string to DWORD type number
	DWORD ret = ConvTextToInt((char*)&numberch, 0, strlen(numberch));

	return (ret);
}

/*
    Race resolver
*/
int race_resolver(BYTE *pic, int number, int styling)
{
	// Init variables
	int coords = FIRSTRACE + NEXTRACE * number;

	// Lookup table for different palettes
	BYTE lookup[] = {
		// Protoss
		140, 115, 120, 155, 25, 129,
		// Terran
		19, 14, 19, 16, 143, 16,
		// Zerg
		46, 37, 37, 43, 52, 46
	};

	// Start comparing
	// Protoss
	if (pic[coords] == lookup[styling]) return (PROTOSS);
	// Terran
	else if (pic[coords] == lookup[styling + 6]) return (TERRAN);
	// Zerg
	else if (pic[coords] == lookup[styling + 12]) return (ZERG);

	return (-1);
}

/*
    Take the statistics picture into pieces and get the important information out of it
*/
int stat_pcx_demolish(char *filename, STATS **stats)
{
	// Some variables
	int ret, i, j;
	int page = 0;
	int races[8];

	// Reset the stat table
	for (i = 0; i < 8; i++) {
		memset(stats[i], 0, sizeof(STATS));
	}

	// In this table are defined usable color by the color index
	BYTE usable_colors[] = {
		3,
		127, 175, 255,
		94, 180, 255,
		104, 172, 255,
		146, 184, 255,
		130, 176, 255,
		117, 163, 255
	};

	// Load the picture into buffer
	PIC pic;

	ret = LoadPCX(&pic, filename, 0);

	if (ret || !pic.pic || pic.x != 640 || pic.y != 480) {
		delete pic.pic;
		return (-1);
	}

	// Find out the palette which this picture is using
	int styling = 0;
	switch (pic.pic[257 + 25 * 640]) {
	case 127:
		styling = 0;
		break;

	case 94:
		styling = 1;
		break;

	case 104:
		styling = 2;
		break;

	case 146:
		styling = 3;
		break;

	case 130:
		styling = 4;
		break;

	case 117:
		styling = 5;
		break;

	default:
		styling = 0;
		break;
	}

	// Find out races
	for (i = 0; i < 8; i++) {
		races[i] = race_resolver(pic.pic, i, styling);
	}

	// Drop out every unusable pixels
	bool usable;
	for (i = 0; i < 640 * 480; i++) {
		usable = false;
		for (j = 1; j <= usable_colors[0]; j++) {
			if (pic.pic[i] == usable_colors[j + styling * usable_colors[0]]) usable = true;
		}
		if (usable == false) pic.pic[i] = INVISIBLE;
		else pic.pic[i] = VISIBLE;
	}

	// Then find out which of four pages in Starcraft statistic table
	// pages this picture is representing
	i = 0;
	j = 0;
	if (pic.pic[PAGEOVERVIEW_ENABLE] == VISIBLE) {
		page = PAGE_OVERVIEW;
		i++;
	}
	if (pic.pic[PAGEUNITS_ENABLE] == VISIBLE) {
		page = PAGE_UNITS;
		i++;
	}
	if (pic.pic[PAGESTRUCTURES_ENABLE] == VISIBLE) {
		page = PAGE_STRUCTURES;
		i++;
	}
	if (pic.pic[PAGERESOURCES_ENABLE] == VISIBLE) {
		page = PAGE_RESOURCES;
		i++;
	}

	if (i != 1) {   // If more than one page found as 'enabled', then this picture is not valid
		delete pic.pic;
		return (-1);
	}

	cout << "Page number " << page + 1 << " was found as enabled." << endl;

	// Next start resolving names
	for (i = 0; i < 8; i++) {
		ret = name_resolver(pic.pic, (char*)&stats[i]->name, i);
		if (ret) {
			delete pic.pic;
			return (-1);
		}
		if (strlen(stats[i]->name) < 1) break;
	}

	// Resolve points
	for (j = 0; j < i; j++) {
		((DWORD*)&stats[j]->units)[page * 3 + 0] = number_resolver(pic.pic, NULL, j, 0);
		((DWORD*)&stats[j]->units)[page * 3 + 1] = number_resolver(pic.pic, NULL, j, 1);
		((DWORD*)&stats[j]->units)[page * 3 + 2] = number_resolver(pic.pic, NULL, j, 2);
		if (page == 0) {
			switch (races[j]) {
			case PROTOSS:
				stats[j]->protoss = 1;
				break;

			case TERRAN:
				stats[j]->terran = 1;
				break;

			case ZERG:
				stats[j]->zerg = 1;
				break;

			default:
				delete pic.pic;
				return (-1);
				break;
			}
		} else {
			stats[j]->protoss = 0;
			stats[j]->terran = 0;
			stats[j]->zerg = 0;
		}
	}

	delete pic.pic;

	// Exit
	return (page);
}

/*
    Add table to another
*/
void add2another(STATS *to, STATS *from, int mode)
{
	switch (mode) {
	default:
	case TAB_ADD:
		to->protoss += from->protoss;
		to->terran += from->terran;
		to->zerg += from->zerg;
		to->units += from->units;
		to->structures += from->structures;
		to->resources += from->resources;
		to->produced += from->produced;
		to->killed += from->killed;
		to->lost += from->lost;
		to->constructed += from->constructed;
		to->razed += from->razed;
		to->slost += from->slost;
		to->gas += from->gas;
		to->minerals += from->minerals;
		to->spent += from->spent;
		break;

	case TAB_SUB:
		to->protoss -= from->protoss;
		to->terran -= from->terran;
		to->zerg -= from->zerg;
		to->units -= from->units;
		to->structures -= from->structures;
		to->resources -= from->resources;
		to->produced -= from->produced;
		to->killed -= from->killed;
		to->lost -= from->lost;
		to->constructed -= from->constructed;
		to->razed -= from->razed;
		to->slost -= from->slost;
		to->gas -= from->gas;
		to->minerals -= from->minerals;
		to->spent -= from->spent;
		break;

	case TAB_REPLACE:
		to->protoss = from->protoss;
		to->terran = from->terran;
		to->zerg = from->zerg;
		to->units = from->units;
		to->structures = from->structures;
		to->resources = from->resources;
		to->produced = from->produced;
		to->killed = from->killed;
		to->lost = from->lost;
		to->constructed = from->constructed;
		to->razed = from->razed;
		to->slost = from->slost;
		to->gas = from->gas;
		to->minerals = from->minerals;
		to->spent = from->spent;
		break;
	}
	return;
}

/*
    Add stats to statistics-table
*/
int stat_add(STATS **table, STATS **add, int mode)
{
	int i, j, k;
	for (k = 0; k < 8; k++) {
		if (strlen(add[k]->name) < 1) break;
		bool found = false;

		for (i = 0; i < MAX_ENTRYS; i++) {
			if (strlen(table[i]->name) < 1) continue;
			if (!strcmp(table[i]->name, add[k]->name)) {
				found = true;
				break;
			}
		}

		if (found == true) {
			add2another(table[i], add[k], mode);
			continue;
		}

		if (mode == TAB_SUB) continue;

		for (i = 0; i < MAX_ENTRYS; i++) {
			if (strlen(table[i]->name) > 0) continue;
			strcpy(table[i]->name, add[k]->name);
			add2another(table[i], add[k], mode);
			break;
		}
	}
	return (0);
}

/*
    Create a txt-file from the stats
*/
int stat_create_txt(STATS **st, char* name)
{
	fstream file;
	int i, j;

	remove(name);
	file.open(name, WRITE | TRUNC);
	if (file.fail()) {
		return (1);
	}

	file << "Nickname" << endl;
	for (i = 0; i < MAX_ENTRYS; i++) {
		if (strlen(st[i]->name) < 1) continue;
		int all_races = st[i]->protoss + st[i]->terran + st[i]->zerg;
		// Name
		file << st[i]->name << endl;
		// Races
		file << "	Games played:	" << all_races << endl;
		if (all_races < 1) all_races++;
		file << "	Protoss:	";
		file << st[i]->protoss << " (" << st[i]->protoss * 100 / all_races << "%)" << endl;
		file << "	Terran:		";
		file << st[i]->terran << " (" << st[i]->terran * 100 / all_races << "%)" << endl;
		file << "	Zerg:		";
		file << st[i]->zerg << " (" << st[i]->zerg * 100 / all_races << "%)" << endl;
		// Overview
		file << "	Overview:	Units		Structures	Resources	Total score" << endl;
		file << "			" << st[i]->units << "		" << st[i]->structures << "		" << st[i]->resources;
		file << "		" << st[i]->units + st[i]->structures + st[i]->resources << endl;
		// Units
		file << "	Units:		Produced	Killed		Lost		Total score" << endl;
		file << "			" << st[i]->produced << "		" << st[i]->killed << "		" << st[i]->lost;
		file << "		" << st[i]->units << endl;
		// Structures
		file << "	Structures:	Constructed	Razed		Lost		Total score" << endl;
		file << "			" << st[i]->constructed << "		" << st[i]->razed << "		" << st[i]->slost;
		file << "		" << st[i]->structures << endl;
		// Resources
		file << "	Resources:	Gas		Minerals	Total spent	Total score" << endl;
		file << "			" << st[i]->gas << "		" << st[i]->minerals << "		" << st[i]->spent;
		file << "		" << st[i]->resources << endl;
	}

	file.close();
	return (0);
}

/*
    Create HTML-file from stats
*/
char html_header[] = {
	"<html><head><title>StarCraft multiplayer game statistics table HTML-page</title></head>\n"
	"<body text=\"#10c010\" bgcolor=\"#000020\">\n"
	"<font color=\"#10c010\" face=\"Arial\" size=\"5\">\n"
	"<hr>StarCraft multiplayer game statistics HTML-page<hr>\n"
	"</h2></font>\n"
};

int stat_create_html(STATS **st, char* name, int flags)
{
	fstream file;
	int i, j;
	STATS gb;   // Stat for global counting of races
	memset(&gb, 0, sizeof(STATS));

	remove(name);
	file.open(name, WRITE | TRUNC);
	if (file.fail()) {
		return (1);
	}

	// Create the html files header
	for (i = 0; i < sizeof(html_header); i++) file.write((char*)&html_header[i], 1);

	// Start creating tables
	for (i = 0; i < MAX_ENTRYS; i++) {
		if (strlen(st[i]->name) < 1) continue;
		// Create info table
		file << "<table border=\"2\" cellpading=\"2\" cellspacing=\"1\" width=\"800\" bordercolor=\"#a0a0a0\" bgcolor=\"#000020\" bordercolordark=\"#303030\" bordercolorlight=\"#f0f0f0\">\n";
		file << "<tr><td width=\"100\" valign=\"top\" align=\"left\"><p><font color=\"#40f040\" face=\"Arial\" size=\"4\">\n";
		// Print name
		file << st[i]->name << endl;
		file << "</font></td>\n";
		// Create statistics table inside the info table
		file << "<td width=\"700\" valign=\"center\" align=\"center\">\n";
		file << "<table border=\"2\" cellpading=\"2\" cellspacing=\"1\" width=\"100%\" bordercolor=\"#c02040\" bgcolor=\"#000020\" bordercolordark=\"#c02040\" bordercolorlight=\"#c02040\">\n";

		// Print statistics to this table

		// Games played...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Games played:\n</font></td>\n";
		// Protoss
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << "Protoss\n</font></td>\n";
		// Terran
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0f0\" face=\"Arial\" size=\"4\">\n";
		file << "Terran\n</font></td>\n";
		// Zerg
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c01010\" face=\"Arial\" size=\"4\">\n";
		file << "Zerg\n</font></td></tr>\n";
		// ...numbers...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0f0f0\" face=\"Arial\" size=\"4\">\n";
		j = st[i]->protoss + st[i]->terran + st[i]->zerg;
		file << "Total: " << j << "\n</font></td>\n";
		if (j < 1) j++;
		// Protoss
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->protoss << " (" << st[i]->protoss * 100 / j << "%)\n</font></td>\n";
		gb.protoss += st[i]->protoss;
		// Terran
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->terran << " (" << st[i]->terran * 100 / j << "%)\n</font></td>\n";
		gb.terran += st[i]->terran;
		// Zerg
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->zerg << " (" << st[i]->zerg * 100 / j << "%)\n</font></td></tr>\n";
		gb.zerg += st[i]->zerg;

		// Overview...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Overview:\n</font></td>\n";
		// Units
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Units\n</font></td>\n";
		// Structures
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Stuctures\n</font></td>\n";
		// Resources
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Resources\n</font></td></tr>\n";
		// ...numbers...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0f0f0\" face=\"Arial\" size=\"4\">\n";
		file << "Total: " << st[i]->units + st[i]->structures + st[i]->resources << "\n</font></td>\n";
		// Units
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->units << "\n</font></td>\n";
		// Structures
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->structures << "\n</font></td>\n";
		// Resources
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->resources << "\n</font></td></tr>\n";
		// ...averages...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0f0f0\" face=\"Arial\" size=\"4\">\n";
		file << "Average: " << (st[i]->units + st[i]->structures + st[i]->resources) / j << "\n</font></td>\n";
		// Units
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->units / j << "\n</font></td>\n";
		gb.units += st[i]->units;
		// Structures
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->structures / j << "\n</font></td>\n";
		gb.structures += st[i]->structures;
		// Resources
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->resources / j << "\n</font></td></tr>\n";
		gb.resources += st[i]->resources;

		// Units...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Units:\n</font></td>\n";
		// Produced
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Produced\n</font></td>\n";
		// Killed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Killed\n</font></td>\n";
		// Lost
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Lost\n</font></td></tr>\n";
		// ...numbers...
		file << "<tr>\n<td width=\"25%\"></td>\n";
		// Produced
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->produced << "\n</font></td>\n";
		gb.produced += st[i]->produced;
		// Killed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->killed << "\n</font></td>\n";
		gb.killed += st[i]->killed;
		// Lost
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->lost << "\n</font></td></tr>\n";
		gb.lost += st[i]->lost;
		// ...averages...
		if (flags == 1) {
			file << "<tr>\n<td width=\"25%\"></td>\n";
			// Units
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->produced / j << "\n</font></td>\n";
			// Structures
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->killed / j << "\n</font></td>\n";
			// Resources
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->lost / j << "\n</font></td></tr>\n";
		}

		// Structures...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Structures:\n</font></td>\n";
		// Constructed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Constructed\n</font></td>\n";
		// Razed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Razed\n</font></td>\n";
		// Lost
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Lost\n</font></td></tr>\n";
		// ...numbers...
		file << "<tr>\n<td width=\"25%\"></td>\n";
		// Constructed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->constructed << "\n</font></td>\n";
		gb.constructed += st[i]->constructed;
		// Razed
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->razed << "\n</font></td>\n";
		gb.razed += st[i]->razed;
		// Lost
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->slost << "\n</font></td></tr>\n";
		gb.slost += st[i]->slost;
		// ...averages...
		if (flags == 1) {
			file << "<tr>\n<td width=\"25%\"></td>\n";
			// Units
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->constructed / j << "\n</font></td>\n";
			// Structures
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->razed / j << "\n</font></td>\n";
			// Resources
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->slost / j << "\n</font></td></tr>\n";
		}

		// Resources...
		file << "<tr>\n";
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Resources:\n</font></td>\n";
		// Gas mined
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Gas mined\n</font></td>\n";
		// Minerals mined
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Minerals mined\n</font></td>\n";
		// Total spent
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
		file << "Total spent\n</font></td></tr>\n";
		// ...numbers...
		file << "<tr>\n<td width=\"25%\"></td>\n";
		// Gas mined
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->gas << "\n</font></td>\n";
		gb.gas += st[i]->gas;
		// Minerals mined
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->minerals << "\n</font></td>\n";
		gb.minerals += st[i]->minerals;
		// Total spent
		file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
		file << st[i]->spent << "\n</font></td></tr>\n";
		gb.spent += st[i]->spent;
		// ...averages...
		if (flags == 1) {
			file << "<tr>\n<td width=\"25%\"></td>\n";
			// Units
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->gas / j << "\n</font></td>\n";
			// Structures
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->minerals / j << "\n</font></td>\n";
			// Resources
			file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
			file << st[i]->spent / j << "\n</font></td></tr>\n";
		}

		// Close the tables
		file << "</table></tr></table>\n<br>\n";
	}

	// Create info table for global stats
	file << "<br><hr><br><table border=\"2\" cellpading=\"2\" cellspacing=\"1\" width=\"800\" bordercolor=\"#a0a0a0\" bgcolor=\"#000020\" bordercolordark=\"#303030\" bordercolorlight=\"#f0f0f0\">\n";
	file << "<tr><td width=\"100\" valign=\"top\" align=\"left\"><p><font color=\"#40f040\" face=\"Arial\" size=\"4\">\n";
	// Print name
	file << "Global stats" << endl;
	file << "</font></td></tr>\n";
	// Create statistics table inside the info table
	file << "<td width=\"700\" valign=\"center\" align=\"center\">\n";
	file << "<table border=\"2\" cellpading=\"2\" cellspacing=\"1\" width=\"100%\" bordercolor=\"#c02040\" bgcolor=\"#000020\" bordercolordark=\"#c02040\" bordercolorlight=\"#c02040\">\n";

	// Print statistics to this table

	// Games played...
	file << "<tr>\n<td width=\"25%\"></td>\n";
	// Protoss
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << "Protoss\n</font></td>\n";
	// Terran
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0f0\" face=\"Arial\" size=\"4\">\n";
	file << "Terran\n</font></td>\n";
	// Zerg
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c01010\" face=\"Arial\" size=\"4\">\n";
	file << "Zerg\n</font></td></tr>\n";
	// ...numbers...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0f0f0\" face=\"Arial\" size=\"4\">\n";
	j = gb.protoss + gb.terran + gb.zerg;
	file << "</font></td>\n";
	if (j < 1) j++;
	// Protoss
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
	file << gb.protoss << " (" << gb.protoss * 100 / j << "%)\n</font></td>\n";
	// Terran
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
	file << gb.terran << " (" << gb.terran * 100 / j << "%)\n</font></td>\n";
	// Zerg
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c0c0\" face=\"Arial\" size=\"4\">\n";
	file << gb.zerg << " (" << gb.zerg * 100 / j << "%)\n</font></td></tr>\n";

	// Overview...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Overview:\n</font></td>\n";
	// Units
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Units\n</font></td>\n";
	// Structures
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Stuctures\n</font></td>\n";
	// Resources
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Resources\n</font></td></tr>\n";
	// ...numbers...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0f0f0\" face=\"Arial\" size=\"4\">\n";
	file << "Total: " << gb.units + gb.structures + gb.resources << "\n</font></td>\n";
	// Units
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.units << "\n</font></td>\n";
	// Structures
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.structures << "\n</font></td>\n";
	// Resources
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.resources << "\n</font></td></tr>\n";

	// Units...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Units:\n</font></td>\n";
	// Produced
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Produced\n</font></td>\n";
	// Killed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Killed\n</font></td>\n";
	// Lost
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Lost\n</font></td></tr>\n";
	// ...numbers...
	file << "<tr>\n<td width=\"25%\"></td>\n";
	// Produced
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.produced << "\n</font></td>\n";
	// Killed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.killed << "\n</font></td>\n";
	// Lost
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.lost << "\n</font></td></tr>\n";

	// Structures...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Structures:\n</font></td>\n";
	// Constructed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Constructed\n</font></td>\n";
	// Razed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Razed\n</font></td>\n";
	// Lost
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Lost\n</font></td></tr>\n";
	// ...numbers...
	file << "<tr>\n<td width=\"25%\"></td>\n";
	// Constructed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.constructed << "\n</font></td>\n";
	// Razed
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.razed << "\n</font></td>\n";
	// Lost
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.slost << "\n</font></td></tr>\n";

	// Resources...
	file << "<tr>\n";
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70f0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Resources:\n</font></td>\n";
	// Gas mined
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Gas mined\n</font></td>\n";
	// Minerals mined
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Minerals mined\n</font></td>\n";
	// Total spent
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#70c0c0\" face=\"Arial\" size=\"4\">\n";
	file << "Total spent\n</font></td></tr>\n";
	// ...numbers...
	file << "<tr>\n<td width=\"25%\"></td>\n";
	// Gas mined
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.gas << "\n</font></td>\n";
	// Minerals mined
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.minerals << "\n</font></td>\n";
	// Total spent
	file << "<td width=\"25%\" valign=\"center\" align=\"center\"><p><font color=\"#c0c010\" face=\"Arial\" size=\"4\">\n";
	file << gb.spent << "\n</font></td></tr>\n";

	// Close the tables
	file << "</table></table>\n<br><br><hr>\n";

	// End of the html
	file << "<br><font color=\"#c0c0ff\" face=\"Arial black\" size=\"3\">\n";
	file << "(This file was created by " << VERSION << ")\n</font><br><br>\n</body></html>\n";

	file.close();
	return (0);
}

/*
    Print version
*/
void version()
{
	cout << VERSION << endl;
}

/*
    Print help
*/
void help()
{
	cout << "sctab options\n";
	cout << "options:\n";
	cout << " a - set add mode to add\n";
	cout << " r - set add mode to replace\n";
	cout << " s - set add mode to substract\n";
	cout << " n - don't add anything\n";
	cout << " g - edd extra average points when creating html-file\n";
	cout << " b filename - use statistics table file\n";
	cout << " f filename - use a single file for adding\n";
	cout << " m filename - creat html-file from the statistics table\n";
	cout << " t filename - create txt-file from the statistics table\n";
	cout << " h - print this help\n";
	cout << " v - print version\n";
	cout << "Add mode specifies the way the statistics read\nfrom file(s) should be added to the main table.\n";
	cout << "If option n is specified, then nothing will be add,\nreplaced or substracted.\n";
	cout << "In default every files which ends with .pcx will be\nprocessed and if valid the information from them will\nbe added to statistics.\n";
	version();
}

/*
    main()-function
*/
int main(int args, char *argch[])
{
	// Define and reset variables
	int i, j;
	bool error = false;
	bool add = false;
	bool table_file = false;
	bool create_txt = false;
	bool create_html = false;
	bool all_files = true;
	int flags = 0;
	int add_mode = TAB_ADD;
	fstream stat_file;
	DIR *dir_pointer = NULL;        // Holds pointer to working directory
	struct dirent *dir_entry = NULL;    // Stores directory info for current file
	char dir_current[MAX_PATH] = "\0";      // Stores current working directory
	char stat_table_name[MAX_PATH] = "\0";  // Stores statistics table filename
	char from_file[MAX_PATH] = "\0";
	char txt_name[MAX_PATH] = "\0";
	char html_name[MAX_PATH] = "\0";
	STATS *stats[8];
	for (i = 0; i < 8; i++) {
		stats[i] = new STATS;
	}

	STATS *stat_table[MAX_ENTRYS];
	for (i = 0; i < MAX_ENTRYS; i++) {
		stat_table[i] = new STATS;
		memset(stat_table[i], 0, sizeof(STATS));
		stat_table[i]->name[0] = '\0';
	}

	// Handle arguments given from command line
	char *argtmp = NULL; // Temporary pointer to the argument currently in process.
	int argcnt = 1;     // Argument counter. First argument excluded.

	// Some options should be given
	if (argcnt >= args) {
		cout << "No options given." << endl;
		help();
		error = true;
		return (0);
	}

	// Start handling of arguments
	while (argcnt <= (args - 1)) {
		argtmp = argch[argcnt];

		if (strlen(argch[argcnt]) > 1) {
			cout << "Invalid option \"" << argch[argcnt] << "\" given.";
			error = true;
			return (0);
		}

		switch (argtmp[0]) {
		case 'a':
			add_mode = TAB_ADD;
			add = true;
			break;

		case 'b':
			argcnt++;
			strncpy(stat_table_name, argch[argcnt], MAX_PATH);
			table_file = true;
			break;

		case 'f':
			argcnt++;
			strncpy(from_file, argch[argcnt], MAX_PATH);
			all_files = false;
			break;

		case 'g':
			flags = 1;
			break;

		case 'm':
			argcnt++;
			strncpy(html_name, argch[argcnt], MAX_PATH);
			create_html = true;
			break;

		case 'n':
			add = false;
			break;

		case 'r':
			add_mode = TAB_REPLACE;
			add = true;
			break;

		case 's':
			add_mode = TAB_SUB;
			add = true;
			break;

		case 't':
			argcnt++;
			strncpy(txt_name, argch[argcnt], MAX_PATH);
			create_txt = true;
			break;

		case 'v':
			version();
			break;

		case 'h':
			help();
			break;

		default:
			cout << "Invalid option \"" << argch[argcnt] << "\" given.";
			error = true;
			return (0);
			break;
		}
		argcnt++;
	}

	if (table_file == true) {
		// Open statistics file for reading
		stat_file.open(stat_table_name, READ | BINARY);
		if (stat_file.fail()) {
			stat_file.open(stat_table_name, WRITE | TRUNC | BINARY);
			if (stat_file.fail()) {
				cout << "Cannot create statistics file." << endl;
				error = true;
				return (0);
			}
		} else {
			for (i = 0; i < MAX_ENTRYS; i++) stat_file.read((char*)stat_table[i], sizeof(STATS));
		}
		stat_file.close();
	}

	// If working directory was omitted the use current dir.
	if (!strlen(dir_current)) getcwd(dir_current, MAX_PATH);

	// Open working directory
	dir_pointer = opendir(dir_current);
	if (!dir_pointer) {
		cout << "Error when opening working directory." << endl;
		error = true;
		return (0);
	}

	// If  no error, do what requested
	if (error == false) {
		// Add stats from wanted files to main stats
		if (add == true) {
			if (all_files == false) {
				cout << "Processing file " << from_file << "..." << endl;
				j = stat_pcx_demolish(from_file, stats);
				if (j >= 0) stat_add(stat_table, stats, add_mode);
			}
			while (all_files == true && (dir_entry = readdir(dir_pointer))) {
				i = strlen(dir_entry->d_name) - 1;
				if (i < 4) continue;
				if (dir_entry->d_name[i - 3] != '.' ||
				        (dir_entry->d_name[i - 2] != 'P' &&
				         dir_entry->d_name[i - 2] != 'p') ||
				        (dir_entry->d_name[i - 1] != 'C' &&
				         dir_entry->d_name[i - 1] != 'c') ||
				        (dir_entry->d_name[i] != 'X' &&
				         dir_entry->d_name[i] != 'x')) continue;
				cout << "Processing file " << dir_entry->d_name << "..." << endl;
				j = stat_pcx_demolish(dir_entry->d_name, stats);
				if (j >= 0) stat_add(stat_table, stats, add_mode);
			}
		}

		// Create txt from stats if wanted
		if (create_txt == true) {
			cout << "Printing statistics to file " << txt_name << "..." << endl;
			stat_create_txt(stat_table, (char*)&txt_name);
		}

		// Create html from stat if wanted
		if (create_html == true) {
			cout << "Creating html statistics file " << html_name << "..." << endl;
			stat_create_html(stat_table, (char*)&html_name, flags);
		}

		if (table_file == true) {
			// Save the statistics file
			stat_file.open(stat_table_name, WRITE | BINARY | TRUNC);
			for (i = 0; i < MAX_ENTRYS; i++) stat_file.write((char*)stat_table[i], sizeof(STATS));
			stat_file.close();
		}
	}

	// Free and close resources
	if (dir_pointer) closedir(dir_pointer);
	for (i = 0; i < 8; i++) if (stats[i]) delete stats[i];
	for (i = 0; i < MAX_ENTRYS; i++) if (stat_table[i]) delete stat_table[i];

	return (0);
}
