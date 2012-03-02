/*

jpegcarve
~~~~~~~~~

finds and dumps JPEG files in large blobs.

usage  : jpgscarf file
example: jpgscarf /dev/sda


licensed under GPL version 2
(c) 2011 by Matthias "mazzoo" Wenzel

*/
#define _GNU_SOURCE

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
	printf("usage  : jpgcarve file\n");
	printf("example: jpgcarve /dev/sda\n");
	exit(0);
}

int main(int argc, char ** argv)
{
	/* in */
	int fin;
	int64_t filesize;
	uint8_t * fmap;

	/* out */
	uint32_t fcount = 0;
	char     fname[FILENAME_MAX];
	int      fout = -1;

	if (argc != 2)
		usage();
	fin = open(argv[1], O_RDONLY | O_LARGEFILE);
	if (fin<0)
		usage();

	filesize = lseek64(fin, 0, SEEK_END);
	fmap = mmap64(NULL, filesize, PROT_READ, MAP_SHARED, fin, 0);
	if (fmap == MAP_FAILED)
	{
		printf("falling back to malloc\n");
		fmap = malloc(filesize);
		if (!fmap)
		{
			printf("out of mem\n");
			exit(1);
		}
		int ret = read(fin, fmap, filesize);
		if (ret != filesize)
		{
			printf("read()\n");
			exit(1);
		}
	}

	/* printf("start scanning\n"); */
	int64_t writing = 0;
	int64_t i;
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

			printf("new jpeg at %ld\n", i);

			snprintf(fname, FILENAME_MAX, "carve_%8.8x.jpg", fcount);
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
	printf("done\n");
	if (writing)
	{
		write(fout, &fmap[writing-6], i-writing+6);
		close(fout);
	}
	return 0;
}
