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
static inline double parsetime(char *str) {
    double minutes = 0;
    double seconds = 0;
    char *colon = strchr(str,':');
    if (colon) {
        *colon++ = '\0';
        if (*str != ':') minutes = atof(str);
        seconds = atof(colon);
    } else {
        seconds = atof(str);
    }
    seconds = minutes*60.0 + seconds;
    return seconds;
}
%}

digit     [0-9]
letter    [A-Za-z]
whitespace [ ]
hexdigit  [0-9A-Fa-f]

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
"color"         return tCOLOR;
"option"        return tOPTION;
"reverse"       return tREVERSE;
"{"             return '{';
"}"             return '}';
\;              return ';';
\,              return ',';

{letter}({digit}|{letter}|_)*      { yylval.str = strdup(yytext); return tIDENT; }
{digit}+\.{digit}*                 { yylval.f    = atof(yytext); return tFLOAT; }
{digit}+\:{digit}+\.{digit}+       { yylval.f    = parsetime(yytext); return tFLOAT; }
{digit}+                           { yylval.w    = atoi(yytext); return tWHOLE; }
\".*\"                             { yylval.str = unquote(yytext); return tSTRING; }
0x{hexdigit}+                      { yylval.w    = strtol(yytext,NULL,0); return tWHOLE; }
\/\/.*$                            ;
^[ \t]*\n                          ;


%%



