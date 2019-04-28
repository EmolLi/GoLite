/* The first section of a flex file contains:
 *    1. A code section for includes and other arbitrary C code. Copied to the scanner verbatim
 *    2. Helper definitions for regexes
 *    3. Scanner options
 */

/* Block comment regex provided by Stackoverflow user Campfire
   https://stackoverflow.com/questions/16160190/regular-expression-to-find-c-style-block-comments
*/

/* Code section */
%{

#include "parser.tab.h" // Token definitions from the parser
#include <string.h>
#include <stdlib.h>
#include <math.h>


  /* Returns a literal/raw copy of a string (no escape sequences) */
  char* string_to_raw(char* str){
    char* raw = (char*)malloc((2*(strlen(str)+1)) * sizeof(char));
    char* current = str;
    char* rawcur = raw;
    while(*current != '\0'){
        if (*current == '\\'){
            *rawcur = '\\';
            rawcur++;
        }
        *rawcur = *current;
        rawcur++;
        current++;
    }
    *rawcur = '\0';
    return raw;
  }

  char get_rune_val(){
      switch(yytext[2]) {
      case 'a':
        return '\a';
      case 'b':
        return '\b';
      case 'f':
        return '\f';
      case 'n':
        return '\n';
      case 'r':
        return '\r';
      case 't':
        return '\t';
      case 'v':
        return '\v';
      default:
        return yytext[1];
      }
  }

  int lastToken = -1;
  #define RETURN(token) lastToken = token; return token;
  int needsSemiColon()
  {
      return lastToken == tIDENTIFIER ||
          lastToken == tINTVAL ||
          lastToken == tFLOAT64VAL ||
          lastToken == tRUNEVAL ||
          lastToken == tSTRINGVAL ||
          lastToken == tBREAK ||
          lastToken == tCONTINUE ||
          lastToken == tFALLTHROUGH ||
          lastToken == tRETURN ||
          lastToken == tINC ||
          lastToken == tDEC ||
          lastToken == tRPAR ||
          lastToken == tRBRACE ||
          lastToken == tRBRACK;
  }

  // Stores the line number with each token sent to bison
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

/* Helper definitions */
DIGIT [0-9]
OCT [0-7]
HEX [0-9a-fA-F]
LETTER [a-zA-Z]

/* With ` */
SYMBOL []_~@#$%&+/‘'<>=|’.;:!?[{}(),^*'`-]
/* Without ` */
RAW_SYMBOL []_~@#$%&+/‘<>=|'’.;:!?[{}(),^*'-]

WS [ \t]
NEW_LINE[\n\r]
ESCAPE_CHAR [abfnrtv\\'"]
/*
'
""
 */


/* Scanner options, line number generation */
%option yylineno

/* Removes warnings about input/output functions */
%option noinput
%option nounput

  /* Multi-Line C Comments */
%x C_COMMENT
%%

"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>\n   { }
<C_COMMENT>.    { }
<C_COMMENT><<EOF>>    {fprintf (stderr, "Error: (line %d) unexpected EOF while reading block comment\n", yylineno); exit(1);}

"\/\/".*    { if(needsSemiColon()) { if(mode == tokens) printf("<inserted semicolon>\n"); RETURN(tSEMICOL); } }
{NEW_LINE}  { if(needsSemiColon()) { if(mode == tokens) printf("<inserted semicolon>\n"); RETURN(tSEMICOL); } }
<<EOF>>     { if(needsSemiColon()) {
                if(mode == tokens) printf("<inserted semicolon>\n");
                RETURN(tSEMICOL);
                }
              return 0;}

[ \t\r]

defer       { if(mode == tokens) printf("tDEFER\n"); RETURN(tDEFER); }
go          { if(mode == tokens) printf("tGO\n"); RETURN(tGO); }
map         { if(mode == tokens) printf("tMAP\n"); RETURN(tMAP); }
chan        { if(mode == tokens) printf("tCHAN\n"); RETURN(tCHAN); }
goto        { if(mode == tokens) printf("tGOTO\n"); RETURN(tGOTO); }
const       { if(mode == tokens) printf("tCONST\n"); RETURN(tCONST); }
fallthrough { if(mode == tokens) printf("tFALLTHROUGH\n"); RETURN(tFALLTHROUGH); }
range       { if(mode == tokens) printf("tRANGE\n"); RETURN(tRANGE); }
interface   { if(mode == tokens) printf("tINTERFACE\n"); RETURN(tINTERFACE); }
select      { if(mode == tokens) printf("tSELECT\n"); RETURN(tSELECT); }
import      { if(mode == tokens) printf("tIMPORT\n"); RETURN(tIMPORT); }



struct  {if(mode == tokens) printf("STRUCT\n"); RETURN(tSTRUCT);}


var     {if(mode == tokens) printf("tVAR\n"); RETURN(tVAR);}
type    {if(mode == tokens) printf("tTYPE\n"); RETURN(tTYPE);}
func    {if(mode == tokens) printf("tFUNC\n"); RETURN(tFUNC);}


for     {if(mode == tokens) printf("tFOR\n"); RETURN(tFOR);}
switch  {if(mode == tokens) printf("tSWITCH\n"); RETURN(tSWITCH);}
case    {if(mode == tokens) printf("tCASE\n"); RETURN(tCASE);}
break   {if(mode == tokens) printf("tBREAK\n"); RETURN(tBREAK);}
continue {if(mode == tokens) printf("tCONTINUE\n"); RETURN(tCONTINUE);}
default {if(mode == tokens) printf("tDEFAULT\n"); RETURN(tDEFAULT);}


if      {if(mode == tokens) printf("tIF\n"); RETURN(tIF);}
else    {if(mode == tokens) printf("tELSE\n"); RETURN(tELSE);}


append  {if(mode == tokens) printf("tAPPEND\n"); RETURN(tAPPEND);}
return  {if(mode == tokens) printf("tRETURN\n"); RETURN(tRETURN);}
println {if(mode == tokens) printf("tPRINTLN\n"); RETURN(tPRINTLN);}
print   {if(mode == tokens) printf("tPRINT\n"); RETURN(tPRINT);}


package {if(mode == tokens) printf("tPACKAGE\n"); RETURN(tPACKAGE);}


"="     {if(mode == tokens) printf("tASSIGN\n"); RETURN(tASSIGN);}
":="    {if(mode == tokens) printf("tSHORTVARDEC\n"); RETURN(tSHORTVARDEC);}

"+"     {if(mode == tokens) printf("tPLUS\n"); RETURN(tPLUS);}
"-"     {if(mode == tokens) printf("tMINUS\n"); RETURN(tMINUS);}
"*"     {if(mode == tokens) printf("tMULT\n"); RETURN(tMULT);}
"/"     {if(mode == tokens) printf("tDIV\n"); RETURN(tDIV);}
"%"     {if(mode == tokens) printf("tMOD\n"); RETURN(tMOD);}

"+="    {if(mode == tokens) printf("tPLUSEQ\n"); RETURN(tPLUSEQ);}
"-="    {if(mode == tokens) printf("tMINUSEQ\n"); RETURN(tMINUSEQ);}
"*="    {if(mode == tokens) printf("tMULTEQ\n"); RETURN(tMULTEQ);}
"/="    {if(mode == tokens) printf("tDIVEQ\n"); RETURN(tDIVEQ);}
"%="    {if(mode == tokens) printf("tMODEQ\n"); RETURN(tMODEQ);}

"++"    {if(mode == tokens) printf("tINC\n"); RETURN(tINC);}
"--"    {if(mode == tokens) printf("tDEC\n"); RETURN(tDEC);}


"<"     {if(mode == tokens) printf("tLT\n"); RETURN(tLT);}
"<="    {if(mode == tokens) printf("tLEQ\n"); RETURN(tLEQ);}
">"     {if(mode == tokens) printf("tGT\n"); RETURN(tGT);}
">="    {if(mode == tokens) printf("tGEQ\n"); RETURN(tGEQ);}
"!"     {if(mode == tokens) printf("tBANG\n"); RETURN(tBANG);}
"&&"    {if(mode == tokens) printf("tAND\n"); RETURN(tAND);}
"||"    {if(mode == tokens) printf("tOR\n"); RETURN(tOR);}
"=="    {if(mode == tokens) printf("tEQ\n"); RETURN(tEQ);}
"!="    {if(mode == tokens) printf("tNEQ\n"); RETURN(tNEQ);}

"&"     {if(mode == tokens) printf("tBITAND\n"); RETURN(tBITAND);}
"|"     {if(mode == tokens) printf("tBITOR\n"); RETURN(tBITOR);}
"\^"     {if(mode == tokens) printf("tXOR\n"); RETURN(tXOR);}

"&="    {if(mode == tokens) printf("tBITANDEQ\n"); RETURN(tBITANDEQ);}
"|="    {if(mode == tokens) printf("tBITOREQ\n"); RETURN(tBITOREQ);}
"&\^"    {if(mode == tokens) printf("tBITCLEAR\n"); RETURN(tBITCLEAR);}
"&\^="   {if(mode == tokens) printf("tBITCLEAREQ\n"); RETURN(tBITCLEAREQ);}
"^="    {if(mode == tokens) printf("tXOREQ\n"); RETURN(tXOREQ);}

"<<="   {if(mode == tokens) printf("tLSHIFTEQ\n"); RETURN(tLSHIFTEQ);}
"<<"    {if(mode == tokens) printf("tLSHIFT\n"); RETURN(tLSHIFT);}
">>="   {if(mode == tokens) printf("tRSHIFTEQ\n"); RETURN(tRSHIFTEQ);}
">>"    {if(mode == tokens) printf("tRSHIFT\n"); RETURN(tRSHIFT);}


"("     {if(mode == tokens) printf("tLPAR\n"); RETURN(tLPAR);}
")"     {if(mode == tokens) printf("tRPAR\n"); RETURN(tRPAR);}
"["     {if(mode == tokens) printf("tLBRACK\n"); RETURN(tLBRACK);}
"]"     {if(mode == tokens) printf("tRBRACK\n"); RETURN(tRBRACK);}
"{"     {if(mode == tokens) printf("tLBRACE\n"); RETURN(tLBRACE);}
"}"     {if(mode == tokens) printf("tRBRACE\n"); RETURN(tRBRACE);}

";"     {if(mode == tokens) printf("tSEMICOL\n"); RETURN(tSEMICOL);}
":"     {if(mode == tokens) printf("tCOL\n"); RETURN(tCOL);}
","     {if(mode == tokens) printf("tCOMMA\n"); RETURN(tCOMMA);}
"."     {if(mode == tokens) printf("tDOT\n"); RETURN(tDOT);}


"<-"    { if(mode == tokens) printf("tLArrow\n"); RETURN(tLArrow); }
"..."   { if(mode == tokens) printf("tThreeDots\n"); RETURN(tThreeDots); }



0|([1-9]{DIGIT}*) {
/* Decimal Integer */
    yylval.intval = atoi(yytext);
    if(mode == tokens) printf("tINTVAL(%s)\n", yytext);
    RETURN(tINTVAL);
}

0{OCT}+ {
/* Octal Integer */
  yylval.intval = strtol(yytext, NULL, 8);
  if(mode == tokens) printf("tINTVAL(%d)\n", yylval.intval);
  RETURN(tINTVAL);
}

0x{HEX}+ {
/* Hex Integer */
  yylval.intval = strtol(yytext, NULL, 16);
  if(mode == tokens) printf("tINTVAL(%d)\n", yylval.intval);
  RETURN(tINTVAL);
}

({DIGIT}+"."{DIGIT}*)|("."{DIGIT}*)|({DIGIT}+".") {
/* Float64 */
    yylval.float64val = atof(yytext);
    if(mode == tokens) printf("tFLOAT64VAL(%s)\n", yytext);
    RETURN(tFLOAT64VAL);
}

\'([^']|\\{ESCAPE_CHAR})\' {
/* ' Rune */
    yylval.runeval = get_rune_val();
    if(mode == tokens) printf("tRUNEVAL(%s)\n", yytext);
    RETURN(tRUNEVAL);
}


\"({DIGIT}|{LETTER}|{SYMBOL}|{WS}|\\{ESCAPE_CHAR})*\" {
/* Interpreted (Normal) String */
    yylval.stringval = strdup(yytext);
    if(mode == tokens) printf("tSTRINGVAL(%s)\n", yytext);
    RETURN(tSTRINGVAL);
}

`[^`]*` {
/* Raw String */
    /* yylval.stringval = strdup(yytext); */
    yylval.stringval = string_to_raw(yytext);
    if(mode == tokens) printf("tSTRINGVAL(%s) - RAW\n", yytext);
    /* printf("aaa %s", yylval.stringval); */
    RETURN(tSTRINGVAL);
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.identifier = strdup(yytext);
    if(mode == tokens) printf("tIDENTIFIER(%s)\n", yytext);
    RETURN(tIDENTIFIER);
}

. 	{ fprintf (stderr, "Error: (line %d) unexpected character '%s'\n", yylineno, yytext); exit(1); }
%%
