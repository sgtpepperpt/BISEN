#include <stdio.h>
#include <stdlib.h>
#include "../Iee/types.h"

// 0x1 : setup secret key
byte cmd1[] = {0x01};

// 0x2 : create or reset file
byte cmd2[] = {0x02};

// 0x3 : add byte string to file
byte cmd3_1[] = {0x03, 0x03, 0xAA, 0xBB, 0xCC};
byte cmd3_2[] = {0x03, 0x02, 0xAA, 0xBB};
byte cmd3_3[] = {0x03, 0x01, 0xAA};

// 0x4 : get byte string from file
byte cmd4_1[] = {0x04, 0x02};
byte cmd4_2[] = {0x04, 0x02};
byte cmd4_3[] = {0x04, 0x03};
byte cmd4_4[] = {0x04, 0x03};
byte cmd4_5[] = {0x04, 0x01};

// exported 
#define TL 10 
size test_len = TL;
bytes commands[TL] = { cmd1, cmd2, cmd3_1, cmd3_2, cmd3_3, cmd4_1, cmd4_2, cmd4_3, cmd4_4, cmd4_5 };
size commands_sizes[TL] = {1,1,   5,4,3,   2,2,2,2,2};

void generate_commands()
{
	FILE *f = fopen("output","rb+");
    if (!f) {
		printf("Unable to open file!");
		exit(-1);
	}

    bytes buff = (bytes)malloc(sizeof(byte)*4);
    buff[0] = 99;
    buff[1] = 104;
    buff[2] = 81;
    buff[3] = 82;

    fwrite(buff, sizeof(unsigned char), 4, f);
    fseek(f, 0, SEEK_SET);

    for (int i = 0; i < 4; i++) {
        byte a = -1;
		int x = fread(&a, sizeof(byte), 1, f);
		printf("%d . %c\n", x, a);
	}
    printf("o\n");
    fclose(f);
    free(buff);
}


int main()
{
    generate_commands();
}
