

/*
 * ROFS - The read-only filesystem for FUSE.
 * Copyright 2005,2006,2008 Matthew Keller. kellermg@potsdam.edu and others.
 * v2008.09.24
 *
 * Mount any filesytem, or folder tree read-only, anywhere else.
 * No warranties. No guarantees. No lawyers.
 *
 * I read (and borrowed) a lot of other FUSE code to write this.
 * Similarities possibly exist- Wholesale reuse as well of other GPL code.
 * Special mention to Rémi Flament and his loggedfs.
 *
 * Consider this code GPLv2.
 *
 * Compile: gcc -o rofs -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lfuse rofs.c
 * Mount: rofs readwrite_filesystem mount_point
 *
 */
#include <ansilove.h>
#include <magic.h>
#include <stdio.h>
#define MIME_DB "/usr/share/file/magic.mgc"
#define FUSE_USE_VERSION 26

static const char* rofsVersion = "2008.09.24";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/xattr.h>
#include <dirent.h>
#include <unistd.h>
#include <fuse.h>

// Global to store our read-write path
char *rw_path;

//Check if file has .png extension 
int is_png(const char * file){
	char* a = strrchr(file, '.');
	if (a == NULL) return 0;
	return ((strcmp(a, ".png") == 0) ? 1 : 0); 
}

//Compare two file paths without taking extensions and directories in consideration
int smart_cmp(char * a, char * b) {
	char * start_a = strrchr(a, '/');
	if (start_a == NULL)
		start_a = a;
	else {
		if (start_a[0] == '/') {
			start_a++;
		}
	}
	char * end_a = strrchr(a, '.');
	int len_a = 0;
	if (end_a == NULL)
		len_a = strlen(a);
		
	len_a = end_a - start_a;
	char * start_b = strrchr(b, '/');
	if (start_b == NULL)
		start_b = b;
	else {
		if (start_b[0] == '/') {
			start_b++;
		}
	}
	char * end_b = strrchr(b, '.');
	
	int len_b = 0;
	if (end_b == NULL)
		len_b = strlen(b);
	len_b = end_b - start_b;
	if (len_a != len_b) return 1;
	return strncmp(start_a, start_b, len_a - 1);
}

