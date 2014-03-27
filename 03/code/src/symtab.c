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
// haha, what the hell are you even doing
// if strings_index is supposed to be "the number of strings in the array" (i.e. it's like the length of the array, the number of elements in it)
// then why are you doing i<=strings_index in the s8 output-stuff?
// because if you want to do that, then strings_index needs to be the last index of strings that contains a valid string reference
// but that's not what you've said it should be
// so I changed your s8 output code
// idk why you guys do these things


void symtab_init ( void )
{
	// should perform any needed initialization
	// i'm just going to have this double strings_size and malloc a new char** and then copy the contents of strings over to it
	// and then free the old strings
	// and then set new_strings to strings

	// hmm no I'll just have this do the init and then string_add could deal with the expansion, ok?
	// ok
	// also this has everything to do with me fudging it up and getting weird errors because I realized I have to set strings_index to 0 at some point

	strings_index = 0;
	strings = (char**) malloc(sizeof(char*) * strings_size);


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
		strings_size *= 2; // double the size! Twice the fun!
		//strings = (char**) realloc(strings, sizeof(char*) * strings_size);
		
		char** new_strings = (char**) malloc(sizeof(char*) * strings_size);
		for (int i = 0; i < strings_index; i++) {
			new_strings[i] = strings[i];
			strings[i] = NULL;
		}
		free(strings);
		strings = new_strings;
		
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
		for ( int i=0; i<strings_index; i++ ) {
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
		for ( int i=0; i<strings_index; i++ )
		    fprintf ( stream, ".STRING%d: .string %s\n", i, strings[i] );
		fputs ( ".globl main\n", stream );
    }
}

