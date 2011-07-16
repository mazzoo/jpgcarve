/*

jpegcarve
~~~~~~~~~

finds and dumps JPEG files in large blobs.

usage  : jpgscarf file
example: jpgscarf /dev/sda


licensed under GPL version 2
(c) 2011 by Matthias "mazzoo" Wenzel

*/

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>


static const char JPEG_MAGIC[] = "JFIF";

void usage(void)
{
	printf("usage  : jpgscarf file\n");
	printf("example: jpgscarf /dev/sda\n");
	exit(0);
}

int main(int argc, char ** argv)
{
	/* in */
	int fin;
	int filesize;
	uint8_t * fmap;

	/* out */
	uint32_t fcount = 0;
	char     fname[FILENAME_MAX];
	int      fout = -1;

	if (argc != 2)
		usage();
	fin = open(argv[1], O_RDONLY);
	if (fin<0)
		usage();

	filesize = lseek(fin, 0, SEEK_END);
	fmap = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fin, 0);

	int writing = 0;
	int i;
	for (i=0; i < filesize - strlen(JPEG_MAGIC); i++)
	{
		if (!memcmp(JPEG_MAGIC, &fmap[i], strlen(JPEG_MAGIC)))
		{
			if (writing)
			{
				write(fout, &fmap[writing-6], i-writing+6);
				close(fout);
			}
			writing = i;

			printf("new jpeg at %d\n", i);

			snprintf(fname, FILENAME_MAX, "scarf_%8.8x.jpg", fcount);
			printf("writing %s\n", fname);

			fout = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0600);

			if (fout < 0)
			{
				printf("ERROR: %s\n", strerror(errno));
				exit(1);
			}

			fcount++;
		}
	}
	if (writing)
	{
		write(fout, &fmap[writing-6], i-writing+6);
		close(fout);
	}
	return 0;
}
