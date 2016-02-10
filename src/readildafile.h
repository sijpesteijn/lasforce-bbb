/*
 * readildafile.h
 *
 *  Created on: Feb 10, 2016
 *      Author: gijs
 */

#ifndef READILDAFILE_H_
#define READILDAFILE_H_

#include <stdio.h>

char *readFile(char *filename) {
	char * buffer = 0;
	long length;
	FILE * f = fopen (filename, "r");

	if (f)
	{
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  fseek (f, 0, SEEK_SET);
	  buffer = malloc (length);
	  if (buffer)
	  {
	    fread (buffer, 1, length, f);
	  }
	  fclose (f);
	}
	printf("%s", buffer);
	return buffer;
}

#endif /* READILDAFILE_H_ */
