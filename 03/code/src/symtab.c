#include "symtab.h"
#include <stdlib.h>

// These variables is located in vslc.c
extern int arch;
extern int outputStage; 

static char **strings; // the array of strings
static int strings_size = 16, //this is the number of strings the array can fit, right? yeah let's go with that
		   strings_index = -1; //number of strings in the array. Wait, what? I think you mean "number of strings in the array -1"
// oh, no you don't, you've just set it to -1 instead of 0 because of reasons, haven't you?
// sheesh


void symtab_init ( void )
{
	// should perform any needed initialization
	// i'm just going to have this double strings_size and malloc a new char** and then copy the contents of strings over to it
	// and then free the old strings
	// and then set new_strings to strings

	strings_size *= 2; //double the space!
	char** new_strings = (char**) malloc(sizeof(char*) * strings_size);
	for (int i = 0; i < strings_index; i++) {
		new_strings[i] = strings[i];
		strings[i] = NULL;
	}
	free(strings);
	strings = new_strings;

	return 0;
}


void symtab_finalize ( void )
{
	// should free any memory allocated for the table and its contents
	// make sure you avoid double freeing anything

	for (int i = 0; i < strings_index; i++) {
		free(strings[i]);
	}
	free(strings);
	return 0;
}


int strings_add ( char *str )
{

	// should add a string to the table and return the index

    if(outputStage == 7)
        fprintf ( stderr, "Add strings (%s), index: %d \n", str, strings_index );

	if (strings_index == strings_size) {
		symtab_init(); //increase the size of the array
	}

	// add str to the array
	strings[strings_index] = str;
	strings_index++; 
	return (strings_index - 1); //return the index of the string we just put into the array
}

// Prints the data segment of the assembly code
// which includes all the string constants found
// ARM and x86 have different formats
void strings_output ( FILE *stream )
{
    if(arch == 1) { //ARM
    	 fputs (
			".syntax unified\n"
			".cpu cortex-a15\n"
			".fpu vfpv3-d16\n"
			".data\n"
			".align	2\n"
			".DEBUG: .ascii \"Hit Debug\\n\\000\"\n"
			".DEBUGINT: .ascii \"Hit Debug, r0 was: %d\\n\\000\"\n"
		    ".INTEGER: .ascii \"%d \\000\"\n"
			".FLOAT: .ascii \"%f \\000\"\n"
			".NEWLINE: .ascii \"\\n \\000\"\n",
		    stream
		);
		for ( int i=0; i<=strings_index; i++ ) {
		    fprintf ( stream, ".STRING%d: .ascii %s\n", i, strings[i] );
		    fprintf ( stream, ".ascii \"\\000\"\n", i, strings[i] ); // ugly hack
		}
		fputs ( ".globl main\n", stream );
		fputs ( ".align	2\n", stream );
    }
    else { //x86
		fputs (
		    ".data\n"
		    ".INTEGER: .string \"%d \"\n"
			".FLOAT: .string \"%f \"\n"
			".NEWLINE: .string \"\\n \"\n",
		    stream
		);
		for ( int i=0; i<=strings_index; i++ )
		    fprintf ( stream, ".STRING%d: .string %s\n", i, strings[i] );
		fputs ( ".globl main\n", stream );
    }
}

