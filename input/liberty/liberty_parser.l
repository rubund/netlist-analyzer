%option noyywrap

%{
#include <stdio.h>
#include <string>

#define YY_DECL int yylex()

extern FILE * liberty_parser_mine;

int verilog_preprocess(char *buf, int n, FILE *filein);

#include "liberty_parser.tab.h"

#define isodigit(x) ((x) >= '0' && (x) <= '7') 
#define hextoint(x) (isdigit((x)) ? (x) - '0' : ((x) - 'A') + 10)  

static char *buffer      = NULL;
static int  buffer_size  = 0;

#define YY_INPUT(buf,result,max_size) result = verilog_preprocess(buf, max_size, liberty_parser_mine)
%}

%option prefix="liberty_parser_yy"
%option yylineno

%x STRING
%x NUMTABLE
%x SWITCHING_CONDITION

hex (x|X)[0-9a-fA-F]{1,2}
oct [0-7]{1,3}

%%

\"                 { 
                     buffer      = (char*) malloc(1); 
                     buffer_size = 1; strcpy(buffer,"");
                     BEGIN(STRING);
                   }
<STRING>\n         {
                      printf("Unterminated string\n");       
                      free(buffer);
                      BEGIN(INITIAL);
                   }
<STRING><<EOF>>    {
                      printf("EOF in string\n");       
                      free(buffer);
                      BEGIN(INITIAL);
                   }
<STRING>[^\\\n"]   {
                     buffer = (char*)realloc(buffer,buffer_size+yyleng+1);
                     buffer_size += yyleng;
                     strcat(buffer,yytext);
                   }
<STRING>\\\n       /* ignore this */
<STRING>\\{hex}    {
                     int temp =0,loop = 0;
                     for(loop=yyleng-2; loop>0; loop--){
                       temp  <<= 4;
                       temp  += hextoint(toupper(yytext[yyleng-loop]));
                     } 
                     buffer = (char*)realloc(buffer,buffer_size+1);
                     buffer[buffer_size-1] = temp;
                     buffer[buffer_size]   = '\0';
                     buffer_size += 1;
                   }
<STRING>\\{oct}    {
                     int temp =0,loop = 0;
                     for(loop=yyleng-1; loop>0; loop--){
                       temp  <<= 3;
                       temp  += (yytext[yyleng-loop] - '0');
                     } 
                     buffer = (char*)realloc(buffer,buffer_size+1);
                     buffer[buffer_size-1] = temp;
                     buffer[buffer_size]   = '\0';
                     buffer_size += 1;
                   }
<STRING>\\[^\n]    {
                     buffer = (char*)realloc(buffer,buffer_size+1);
                     switch(yytext[yyleng-1]){
                     case 'b' : buffer[buffer_size-1] = '\b';  break;
                     case 't' : buffer[buffer_size-1] = '\t';  break;
                     case 'n' : buffer[buffer_size-1] = '\n';  break;
                     case 'v' : buffer[buffer_size-1] = '\v';  break;
                     case 'f' : buffer[buffer_size-1] = '\f';  break;
                     case 'r' : buffer[buffer_size-1] = '\r';  break;
                     default  : buffer[buffer_size-1] = yytext[yyleng-1];
                     }
                     buffer[buffer_size] = '\0';
                     buffer_size += 1;
                   }
<STRING>\"         {
                     //printf("string = \"%s\"",buffer); 
                     liberty_parser_yylval.string = new std::string(buffer);
                     free(buffer);
                     BEGIN(INITIAL);
                     return TOK_STRING;
                   }

<NUMTABLE>\" { return T_QUOTATION; }
<NUMTABLE>";" { BEGIN(INITIAL); return T_SEMICOLON; }
<NUMTABLE>[ \t\r\n\\]	; // ignore all whitespace
<NUMTABLE>"("		{return T_LEFT;}
<NUMTABLE>")"		{return T_RIGHT;}

<SWITCHING_CONDITION>[ \t\r\n\\]	; // ignore all whitespace
<SWITCHING_CONDITION>")"		{BEGIN(INITIAL);}
<SWITCHING_CONDITION>. ;

[ \t\r\n]	; // ignore all whitespace
<INITIAL,NUMTABLE>\-?[0-9]+\.[0-9]+(e-[0-9]+)? 	{liberty_parser_yylval.fval = atof(yytext); return T_FLOAT;}
<INITIAL,NUMTABLE>\-?[0-9]+(e-[0-9]+)?		{liberty_parser_yylval.ival = atoi(yytext); return T_INT;}
"("		{return T_LEFT;}
")"		{return T_RIGHT;}
(cell_rise)|(rise_transition)|(cell_fall)|(fall_transition) {liberty_parser_yylval.string = new std::string(yytext); return T_USEFUL_TABLE;}
cell {return T_CELL;}
pin {return T_PIN;}
";"		{return T_SEMICOLON;}
<INITIAL,NUMTABLE>"," {return T_COMMA;}
"." {return T_DOT;}
index_[0-9] {char * ind = strchr(yytext,'_'); liberty_parser_yylval.ival = atoi(ind+1); BEGIN(NUMTABLE); return T_INDEX;}
values {BEGIN(NUMTABLE); return T_VALUES;}
":" {return T_COLON;}
"[" {return T_LEFTBRACKET;}
"]" {return T_RIGHTBRACKET;}
"library" {return T_LIBRARY;}
"{" {return T_LEFTCURLY;}
"}" {return T_RIGHTCURLY;}
"\"" {return T_QUOTATION;}
"\\" ; //{return T_BACKSLASH;}

rise_capacitance { liberty_parser_yylval.string = new std::string(yytext); return T_RISE_CAPACITANCE; }
fall_capacitance { liberty_parser_yylval.string = new std::string(yytext); return T_FALL_CAPACITANCE; }
fall_capacitance_range { liberty_parser_yylval.string = new std::string(yytext); return T_FALL_CAPACITANCE_RANGE; }
rise_capacitance_range { liberty_parser_yylval.string = new std::string(yytext); return T_RISE_CAPACITANCE_RANGE; }
direction { return T_DIRECTION; }
(input_switching_condition)|(output_switching_condition) { BEGIN(SWITCHING_CONDITION); return T_SWITCHING_CONDITION; }
[a-zA-Z_$][a-zA-Z0-9_\-$]* { liberty_parser_yylval.string = new std::string(yytext); return T_NAMEMINUS; }
[a-zA-Z_$][a-zA-Z0-9_$]* { liberty_parser_yylval.string = new std::string(yytext); return T_NAME; }
<NUMTABLE>. ;

%%
