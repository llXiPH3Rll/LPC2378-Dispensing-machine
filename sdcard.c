#include <RTL.h>                      /* RTL kernel functions & defines      */
#include <stdio.h>
#include <string.h>
#include "File_Config.h"
#include "sdcard.h"
#include "LCD.h"

void init_card(){
	U32 retv;
	lcd_clear();
  while ((retv = finit (NULL)) != 0) {        /* Wait until the Card is ready*/
    if (retv == 1) {
			lcd_print("Card init failed");
    }
    else {
      lcd_print("Card init done");
    }
  }
}

void read_file(char *data){
	FILE *f;
	char ch;
	data[0] = 0;
	f = fopen("a.txt","r");
	while ((ch = fgetc (f)) != EOF) {
    strncat(data, &ch, 1);
  }
}

void write_file(char *data){
	FILE *f;
	
	f = fopen("a.txt", "w");
	fprintf(f, data);
	fclose(f);
}
