/*
 * readildafile.h
 *
 *  Created on: Feb 10, 2016
 *      Author: gijs
 */

#ifndef READILDAFILE_H_
#define READILDAFILE_H_

#include <stdio.h>

int readFile(const char *filename, char **result)
{
	int size = 0;
	FILE *f = fopen(filename, "r");
	if (f == NULL)
	{
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *)malloc(size+1);
	if (size != fread(*result, sizeof(char), size, f))
	{
		free(*result);
		return -2; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}

#endif /* READILDAFILE_H_ */
