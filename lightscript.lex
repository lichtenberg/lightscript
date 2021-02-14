/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Tokenizer (LEX/FLEX) script              file: lightscript.lex
    *  
    *  defines the tokens used by the parser.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */

%option noyywrap yylineno
%{
#include "lightscript.h"
#include "lightscript.tab.h"

static char *unquote(char *str) {
    char *x = strchr(str+1,'"');
    if (x) *x = '\0';
    return strdup(str+1);
}
%}

digit     [0-9]
letter    [A-Za-z]
whitespace [ ]

%%

"music"         return tMUSIC;
"from"          return tFROM;
"to"            return tTO;
"at"            return tAT;
"do"            return tDO;
"on"            return tON;
"count"         return tCOUNT;
"idle"          return tIDLE;
"speed"         return tSPEED;
"cascade"       return tCASCADE;
"delay"         return tDELAY;
"brightness"    return tBRIGHTNESS;
"define"        return tDEFINE;
"macro"         return tMACRO;
"as"            return tAS;
"palette"       return tPALETTE;
"reverse"       return tREVERSE;
"{"             return '{';
"}"             return '}';
\;              return ';';
\,              return ',';

{letter}({digit}|{letter}|_)*      { yylval.str = strdup(yytext); return tIDENT; }
{digit}+\.{digit}*                 { yylval.f    = atof(yytext); return tFLOAT; }
{digit}+                           { yylval.w    = atoi(yytext); return tWHOLE; }
\".*\"                             { yylval.str = unquote(yytext); return tSTRING; }
\/\/.*$                            ;
^[ \t]*\n                          ;


%%



