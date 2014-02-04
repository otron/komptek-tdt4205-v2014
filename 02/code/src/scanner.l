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


WHITESPACE	[\t\n\ ]
DIGIT		[0-9]
FLOATZ		[0-9]+\.[0-9]+
IDENTIFIER	[a-zA-Z_][a-zA-Z_0-9]*
PUNCT		[\(\);]





%%
"FALSE"				{ RETURN( FALSE_CONST ); }
"TRUE"				{ RETURN( TRUE_CONST ); }


"+"					{ RETURN( yytext[0] ); }
"-"					{ RETURN( yytext[0] ); }
"/"					{ RETURN( yytext[0] ); }
"*"					{ RETURN( yytext[0] ); }
">"					{ RETURN( yytext[0] ); }
"<"					{ RETURN( yytext[0] ); }
"!"					{ RETURN( yytext[0] ); }
"=="				{ RETURN( EQUAL ); }
":="				{ RETURN( ASSIGN ); }
"<="				{ RETURN( LEQUAL ); }
">="				{ RETURN( GEQUAL ); }
"!="				{ RETURN( NEQUAL ); }
"||"				{ RETURN( OR ); }
"&&"				{ RETURN( AND ); }

"FUNC"				{ RETURN( FUNC ); }
"PRINT"				{ RETURN( PRINT ); }
"RETURN"			{ RETURN( RETURN ); }
"IF"				{ RETURN( IF ); }
"THEN"				{ RETURN( THEN ); }
"ELSE"				{ RETURN( ELSE ); }
"END"				{ RETURN( END ); }
"WHILE"				{ RETURN( WHILE ); }
"DO"				{ RETURN( DO ); }
"INT"				{ RETURN( INT ); }
"BOOL"				{ RETURN( BOOL ); }
"FLOAT"				{ RETURN( FLOAT ); }
"CLASS"				{ RETURN( _CLASS_ ); }
"HAS"				{ RETURN( HAS ); }
"WITH"				{ RETURN( WITH ); }
"START"				{ RETURN( START ); }
"VOID"				{ RETURN( VOID ); }
"THIS"				{ RETURN( THIS ); }
"NEW"				{ RETURN( NEW ); }

[0-9]+\.[0-9]+		{ RETURN( FLOAT_CONST ); }
{DIGIT}+			{ RETURN( INT_CONST ); }

[a-zA-Z_][a-zA-Z_0-9]* 	{ RETURN( IDENTIFIER ); }
\"([^\n\"]|\\\")*\"		{ RETURN( STRING_CONST ); }

"//"[^\n]*\n	{}
{WHITESPACE}+	{}
.      	     	{ RETURN( yytext[0] ); }
%%
