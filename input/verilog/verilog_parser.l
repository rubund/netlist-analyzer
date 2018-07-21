%option noyywrap

%{
#include <stdio.h>
#include <string>

#define YY_DECL int yylex()

extern FILE * verilog_parser_mine;

int verilog_preprocess(char *buf, int n, FILE *filein);

#include "verilog_parser.tab.h"


#define YY_INPUT(buf,result,max_size) result = verilog_preprocess(buf, max_size, verilog_parser_mine)
%}

%option prefix="verilog_parser_yy"
%option yylineno

%x STRING

%%

\" { BEGIN(STRING);}
<STRING>\\. { yymore();}
<STRING>\" {
    BEGIN(0);
    char *yystr = strdup(yytext);
    yystr[strlen(yytext) - 1] = 0;
    int i = 0, j = 0;
    while (yystr[i]) {
        if (yystr[i] == '\\' && yystr[i + 1]) {
            i++;
            if (yystr[i] == 'n')
                yystr[i] = '\n';
            else if (yystr[i] == 't')
                yystr[i] = '\t';
            else if ('0' <= yystr[i] && yystr[i] <= '7') {
                yystr[i] = yystr[i] - '0';
                if ('0' <= yystr[i + 1] && yystr[i + 1] <= '7') {
                    yystr[i + 1] = yystr[i] * 8 + yystr[i + 1] - '0';
                    i++;
                }
                if ('0' <= yystr[i + 1] && yystr[i + 1] <= '7') {
                    yystr[i + 1] = yystr[i] * 8 + yystr[i + 1] - '0';
                    i++;
                }
            }
        }
        yystr[j++] = yystr[i++];
    }
    yystr[j] = 0;
    free(yystr);
    return TOK_STRING;
}
<STRING>.   { yymore(); }

[ \t\r\n]	; // ignore all whitespace
[0-9]*'[hbd][0-9A-Fa-f]* { verilog_parser_yylval.string = new std::string(yytext); return T_CONSTANT; }
[0-9]+\.[0-9]+ 	{verilog_parser_yylval.fval = atof(yytext); return T_FLOAT;}
[0-9]+		{verilog_parser_yylval.ival = atoi(yytext); return T_INT;}
"+"		{return T_PLUS;}
"-"		{return T_MINUS;}
"*"		{return T_MULTIPLY;}
"/"		{return T_DIVIDE;}
"("		{return T_LEFT;}
")"		{return T_RIGHT;}
"exit"		{return T_QUIT;}
"quit"		{return T_QUIT;}
"module"  {return T_MODULE;}
"endmodule"  {return T_ENDMODULE;}
";"		{return T_SEMICOLON;}
"," {return T_COMMA;}
"." {return T_DOT;}
"input" {return T_INPUT;}
"output" {return T_OUTPUT;}
"inout" {return T_INOUT;}
"logic" {return T_LOGIC;}
"reg" {return T_REG;}
"wire" {return T_WIRE;}
"assign" {return T_ASSIGN;}
"=" {return T_EQUAL;}
":" {return T_COLON;}
"[" {return T_LEFTBRACKET;}
"]" {return T_RIGHTBRACKET;}
"{" {return T_LEFTCURLY;}
"}" {return T_RIGHTCURLY;}


\\[a-zA-Z_$][a-zA-Z0-9_$]*\[[0-9]+\] { verilog_parser_yylval.string = new std::string(yytext); return T_STRANGE_NAME; }
[a-zA-Z_$][a-zA-Z0-9_$]* { verilog_parser_yylval.string = new std::string(yytext); return T_NAME; }

. { return *yytext; }

%%
