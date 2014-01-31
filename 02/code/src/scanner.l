%{
/* This %{ ... %} section is code that is glued into the generated scanner. */
#include "parser.h"
#define YYDEBUG 1
extern int outputStage; // This variable is located in vslc.c
    #define RETURN(t) do {                                             \
        if ( outputStage == 1 )                                        \
            fprintf ( stderr, "TOKEN ( %d,\t'%s' )\n", t, yytext );    \
        return t;                                                      \
    } while ( 0 )
    
    
%}

%option array
%option noyywrap
%option yylineno


WHITESPACE	[\t\n\ ]+
COMMENT		"//"[^\n]*\n
INTEGER		[:digit:]+
FLOAT		[0-9]+\.[0-9]+
STRING		[^\n"\\\""]+
IDENTIFIER	[a-zA-Z][a-zA-Z_]+


%%
COMMENT		{}
INTEGER		{ RETURN( INT ); }
.           { RETURN( yytext[0] ); }
%%
