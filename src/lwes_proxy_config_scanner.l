%{
#include <stdio.h>
#include <string.h>
#include "lwes_proxy_config_parser.h"

/* I don't understand the need for this section at all, I believe it's
   just because of an older flex implementation
 */
#define YY_NO_UNPUT
#undef YY_DECL
#define YY_DECL int lwes_proxy_config_lex (void)
int lwes_proxy_config_lex (void);

/* defined in parser */
extern void lwes_proxy_config_error(const char *s, ...);
%}

%option noyywrap yylineno never-interactive

%x EXPECT_COLON
%x STRATEGY_DEFINITION

ip (([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\.){3}([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])

%%

[ \t\n]+               ; /* ignore whitespace */
"frontend"           { return FRONTEND; }
"default_backend"    { return DEFAULT_BACKEND; }
"backend"            { return BACKEND; }
"server"             { return SERVER; }
<STRATEGY_DEFINITION>[ \t]+ ; /* ignore whitespace here too */
<STRATEGY_DEFINITION>[0-9]+ {
                              lwes_proxy_config_lval.shortval =
                                (short) atoi (yytext);
                              return NOFM;
                            }
<STRATEGY_DEFINITION>"ofN"  ; /* ignore */
<STRATEGY_DEFINITION>"all"  { return ALL; }
<STRATEGY_DEFINITION>_      { return '_'; }
<STRATEGY_DEFINITION>.|\n   { BEGIN 0; }
"strategy"           { BEGIN STRATEGY_DEFINITION;
                       return STRATEGY;
                     }
"filter"             { return FILTER; }
"in"                 { return IN; }
"out"                { return OUT; }
<EXPECT_COLON>":"    {
                       BEGIN 0;
                       return ':';
                     }
{ip}                 {
                       BEGIN EXPECT_COLON;
                       lwes_proxy_config_lval.strval = strdup (yytext);
                       return IPADDRESS;
                     }
[0-9]+               {
                       lwes_proxy_config_lval.shortval = (short) atoi (yytext);
                       return PORT;
                     }
[a-zA-Z0-9_:]+       {
                       lwes_proxy_config_lval.strval = strdup (yytext);
                       return WORD;
                     }
#.*                  ; /* ignore comments */
.                    { lwes_proxy_config_error ("unrecognized %s\n",yytext);
                       /* FIXME: I need to reject something otherwise I
                                 get a compile warning */
                       REJECT
                     }
%%
