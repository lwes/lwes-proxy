/*
 * Parses the lwes-proxy config file, during parsing the following structures
 * are used
 *
 * Hash Backend
 *   Backend -> index
 *
 * ArrayList Backend
 *   index -> Backend
 *
 * Trie Filters
 *   EventName -> { Set In, Set Out }
 *
 * ArrayList Frontend
 *   index -> Frontend
 *
 * Frontend
 *   Name
 *   ArrayList Backends
 */

%{
#include <stdio.h>
#include <stdarg.h>
#include "lwes_proxy_config.h"

void lwes_proxy_config_error (const char *s, ...);

/* interface to the lexer */
extern FILE *lwes_proxy_config_in;
extern int lwes_proxy_config_lineno;
extern void lwes_proxy_config_restart (FILE *);
extern int lwes_proxy_config_lex (void);
%}

%union {
  short  shortval;
  char * strval;
}

%token FRONTEND
%token DEFAULT_BACKEND
%token BACKEND
%token SERVER
%token STRATEGY
%token FILTER
%token IN
%token OUT
%token ALL
%token <strval> WORD
%token <shortval> PORT
%token <strval> IPADDRESS
%token <shortval> NOFM

%%

channel_list: /* nothing */
            | channel_list frontend
            | channel_list backend
            ;

frontend : frontend_defintion frontend_options
         ;

frontend_defintion : FRONTEND WORD IPADDRESS ':' PORT {
                       printf ("frontend %s => %s:%d\n", $2, $3, $5);
                       free ($2);
                       free ($3);
                     }
                   | FRONTEND WORD IPADDRESS ':' IPADDRESS ':' PORT  {
                       printf ("frontend %s => %s:%s:%d\n", $2, $3, $5, $7);
                       free ($2);
                       free ($3);
                       free ($5);
                     }
  ;

frontend_options: DEFAULT_BACKEND WORD { printf ("default_backend %s\n",$2),
                                         free ($2);
                                       }
                | frontend_options DEFAULT_BACKEND WORD { printf ("default_backend %s\n",$3),
                                         free ($3);
                                       }
  ;

backend : backend_definition backend_options
        ;

backend_definition : BACKEND WORD { printf ("backend %s\n",$2),
                                    free ($2);
                                  }
                   ;

backend_options:
               | backend_options strategy_definition
               | backend_options server_defintion
               | backend_options filter_definition
               ;

strategy_definition : STRATEGY NOFM { printf ("strategy %dofM\n",$2); }
                    | STRATEGY ALL { printf ("strategy all\n"); }
                    | STRATEGY NOFM '_' ALL { printf ("strategy %dofM_all\n",$2); }
                    | STRATEGY NOFM '_' NOFM { printf ("strategy %dofM_%dofM\n",$2,$4); }
                    | STRATEGY ALL '_' ALL { printf ("strategy all_all\n"); }
                    | STRATEGY ALL '_' NOFM { printf ("strategy all_%dofM\n",$4); }
                    ;

server_defintion: SERVER WORD IPADDRESS ':' PORT {
                       printf ("server %s => %s:%d\n", $2, $3, $5);
                       free ($2);
                       free ($3);
                     }
                | SERVER WORD IPADDRESS ':' IPADDRESS ':' PORT  {
                       printf ("server %s => %s:%s:%d\n", $2, $3, $5, $7);
                       free ($2);
                       free ($3);
                       free ($5);
                     }

                ;

filter_definition: FILTER IN WORD { printf ("filter in %s\n",$3); free ($3); }
                 | FILTER OUT WORD{ printf ("filter out %s\n",$3); free ($3); }
                 ;

%%

int
lwes_proxy_config_parse_file (const char *file)
{
  FILE *fd = fopen (file, "r");

  if (fd != NULL)
    {
      lwes_proxy_config_in = fd;
      lwes_proxy_config_restart (lwes_proxy_config_in);
      lwes_proxy_config_parse ();
      fclose (fd);
      return 0;
    }
  else
    {
      return 1;
    }
}

void lwes_proxy_config_error (const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  fprintf(stderr, "Error line %d: ", lwes_proxy_config_lineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
