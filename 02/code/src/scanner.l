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

/* TOKENS *

%token INT_CONST FLOAT_CONST TRUE_CONST FALSE_CONST STRING_CONST
%token INT FLOAT BOOL VOID IDENTIFIER
%token ASSIGN FUNC START PRINT RETURN IF THEN ELSE END WHILE DO
%token EQUAL GEQUAL LEQUAL NEQUAL AND OR  
%token _CLASS_ THIS HAS WITH NEW

*/

WHITESPACE	[\t\n\ ]
COMMENT		"//"[^\n]*\n
DIGIT		[:digit:]
FLOAT		[0-9]+\.[0-9]+
STRING		\"[^\n\"]+\"
IDENTIFIER	[a-zA-Z][a-zA-Z_]+




%%
{DIGIT}+			{ RETURN( INT ); }
FLOAT				{ RETURN( FLOAT ); }
"TRUE"|"FALSE"		{ RETURN( BOOL ); }
STRING				{ RETURN( STRING ); }
IDENTIFIER			{ RETURN( IDENTIFIER ); }




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


COMMENT			{}
{WHITESPACE}+	{}
.      	     	{ RETURN( yytext[0] ); }
%%
