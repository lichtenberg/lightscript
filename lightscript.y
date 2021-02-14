/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Parser (BISON) script              file: lightscript.y
    *  
    *  Our language rules are defined here.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */

%{
#include <stdio.h>
#include "lightscript.h"
%}

%union {
    node_t *n;
    double f;
    int w;
    char *str;
}

/* our tokens */
%token <f> tFLOAT
%token <w> tWHOLE
%token <str> tIDENT tSTRING

%token tMUSIC tFROM tTO tAT tDO tON tCOUNT tIDLE tSPEED tCASCADE tDELAY tBRIGHTNESS tDEFINE tAS tMACRO tPALETTE tREVERSE

%type <n> idlist top optlist option scriptcmd scriptlist


/* %start script */
%%

top : scriptlist { tree = $1; }
    ;

scriptlist :
     scriptcmd ';' { $$ = newnode($1,NULL); }
     | scriptcmd ';' scriptlist { $$ = newnode($1,$3); } 
     ;

idlist : tIDENT { $$ = newidlist($1, NULL) ; }
    | tIDENT  ',' idlist { $$ = newidlist($1, $3); }
    ;

scriptcmd :
     tAT tFLOAT optlist  {  $$ = newcmd_sched(sAT, $2, $2, $3); }
     | tFROM tFLOAT tTO tFLOAT optlist { $$ = newcmd_sched(sFROM, $2, $4, $5); }
     | tMUSIC tSTRING  { $$ = newcmd_str(sMUSIC,$2); }
     | tIDLE tIDENT  { $$ = newcmd_str(sIDLE, $2); }
     | tDEFINE tIDENT tAS tWHOLE  { $$ = newcmd_defval($2, $4); }
     | tDEFINE tIDENT tAS idlist  { $$ = newcmd_defidl($2, $4); }
     | tDEFINE tIDENT '{' scriptlist '}' { $$ = newcmd_defmacro($2, $4); }
     ;


option : tON idlist  { $$ = newoption_idl(oON, $2); }
    | tCASCADE tIDENT { $$ = newoption_idl(oCASCADE, newidlist($2,NULL)); }
    | tDO tIDENT { $$ = newoption_idl(oDO, newidlist($2, NULL)); }
    | tMACRO tIDENT { $$ = newoption_idl(oMACRO, newidlist($2, NULL)); }
    | tBRIGHTNESS tWHOLE { $$ = newoption_w(oBRIGHTNESS, $2); }
    | tDELAY tFLOAT { $$ = newoption_f(oDELAY, $2); }
    | tSPEED tWHOLE { $$ = newoption_w(oSPEED, $2); }
    | tCOUNT tWHOLE { $$ = newoption_w(oCOUNT, $2); }
    | tPALETTE tWHOLE { $$ = newoption_w(oPALETTE, $2); }
    | tREVERSE { $$ = newoption_w(oREVERSE, 0); }
    ;

optlist : 
    option optlist { $$ = newnode($1, $2); }
    | option { $$ = newnode($1, NULL); }
    ;






