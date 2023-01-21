// screenify.c
// read in an Apple II disk file and a text file
// write them to a given block number
// probably clobber the directory if you're not careful

#include <stdio.h>
#include <string.h>

char buffer[143360]; // size of Apple II disk
char line[128];	// max 64 chars per line, but leave space for overflow

void main() {
	char *pos;
	int wptr;
	FILE *disk = fopen("forth.dsk", "rb");
	FILE *text = fopen("screens.fs", "r");

	fread(&buffer, sizeof(unsigned char), 143360, disk);
	fclose(disk);
	
	wptr = (3 * 16 * 256); // start of track 3, 16 sectors of 256 bytes
	printf("buffer at %d\n",wptr);
	while(!feof(text)) {
		fgets(line, 128, text);
		for (int i=0; i<65; i++) {
			printf("%02x ",line[i]);
		}
		printf("\n");
		pos = strchr(line, '\n');
		printf("%p %d\n", pos, 64-(pos-line));
		if (pos) {
			*pos = ' ';
			memset(pos, ' ', 64-(pos-line));
		}
		printf("pos-line=%d\t%\s\n", pos-line, line);
		strncpy(buffer+wptr, line, 64);
		wptr += 64;

	}
	fclose(text);
	disk = fopen("output.dsk", "wb");
	fwrite(&buffer, sizeof(unsigned char), 143360, disk);
	fclose(disk);
}
