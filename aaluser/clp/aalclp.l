%{
// Copyright (c) 2013-2015, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#if HAVE_STDLIB_H
# include <stdlib.h>
#else
# error Required system header stdlib.h not found.
#endif // HAVE_STDLIB_H
#if HAVE_ERRNO_H
# include <errno.h>
#else
# error Required system header errno.h not found.
#endif // HAVE_ERRNO_H
#if HAVE_CTYPE_H
# include <ctype.h>
#else
# error Required system header ctype.h not found.
#endif // HAVE_CTYPE_H

#ifndef HAVE_STRCHR
#error Required system library fn strchr() not found.
#endif // HAVE_STRCHR

#include "aalsdk/aalclp/aalclp.h"

/* All fn's return non-zero on error, unless otherwise noted. */

static int aalclp_on_nix_long_option_only(yyscan_t yyscanner, const char *option);
static int aalclp_on_nix_long_option(yyscan_t yyscanner, const char *option, const char *value);
static int aalclp_on_nix_short_option_only(yyscan_t yyscanner, const char *option);
static int aalclp_on_nix_short_option(yyscan_t yyscanner, const char *option, const char *value);
static int aalclp_on_dash_only(yyscan_t yyscanner);
static int aalclp_on_dash_dash_only(yyscan_t yyscanner);
static int aalclp_on_win_long_option_only(yyscan_t yyscanner, const char *option);
static int aalclp_on_win_long_option(yyscan_t yyscanner, const char *option, const char *value);
static int aalclp_on_win_short_option_only(yyscan_t yyscanner, const char *option);
static int aalclp_on_win_short_option(yyscan_t yyscanner, const char *option, const char *value);
static int aalclp_on_non_option(yyscan_t yyscanner, const char *nonoption);
static int aalclp_on_invalid(yyscan_t yyscanner, const char *invalid, size_t len);

/* flex-to-aalclp API */
/* ************************************************************************** */
/* flex scanner internals */

#define MAX_OPTION   256
static char option[MAX_OPTION] = { 0, };
static int  option_len         = 0;

#define begin_option(ch) \
do                       \
{                        \
   option[0]  = ch;      \
   option[1]  = 0;       \
   option_len = 1;       \
}while(0)

#define cont_option(str, len)                         \
do                                                    \
{                                                     \
   strncat(option, str, sizeof(option) - option_len); \
   option_len += len;                                 \
}while(0)

#define end_option() \
do                   \
{                    \
   option[0]  = 0;   \
   option_len = 0;   \
}while(0)

#define MAX_ARGUMENT 512
static char argument[MAX_ARGUMENT] = { 0, };
static int  argument_len           = 0;

#define cont_argument(str, len)                             \
do                                                          \
{                                                           \
   strncat(argument, str, sizeof(argument) - argument_len); \
   argument_len += len;                                     \
}while(0)

#define end_argument() \
do                     \
{                      \
   argument[0]  = 0;   \
   argument_len = 0;   \
}while(0)

#define my_begin(x) BEGIN(x)

%}

WS                   [ \t\n]
PRINT_MINUS_WS       [[:print:]]{-}[[:space:]]
PRINT_MINUS_WS_DASH  [[:print:]]{-}[[:space:]\-]
PRINT_MINUS_WS_EQU   [[:print:]]{-}[[:space:]=]

%option 8bit reentrant
%option warn nodefault
%option noyywrap

%x DASH
%x DASHDASH
%x NONOPTION

%x NIXSHORTOPT
%x NIXSHORTOPTWS

%x NIXSHORTOPTSEQ
%x NIXSHORTOPTSEQWS

%x NIXLONGOPT
%x NIXLONGOPTWS

%%


<INITIAL>{WS}+ /* eat whitespace */

<INITIAL>"-"                                             {
                                                            begin_option('-');
                                                            my_begin(DASH);
                                                         }

<INITIAL>{PRINT_MINUS_WS_DASH}                           {
                                                            yyless(0);
                                                            my_begin(NONOPTION);
                                                         }

<INITIAL><<EOF>> yyterminate(); /* success */

<INITIAL>(?s:.)                                          {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<DASH>{WS}                                               { // -
                                                            yyless(0);
                                                            if ( aalclp_on_dash_only(yyscanner) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<DASH>"-"                                                { // --
                                                            cont_option(yytext, yyleng);
                                                            my_begin(DASHDASH);
                                                         }

<DASH>{PRINT_MINUS_WS_DASH}                              { // -a
                                                            cont_option(yytext, yyleng);
                                                            my_begin(NIXSHORTOPT);
                                                         }

<DASH><<EOF>>                                            { // -<EOF>
                                                            if ( aalclp_on_dash_only(yyscanner) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<DASH>(?s:.)                                             {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXSHORTOPT>{WS}+                                       { // -a
                                                            my_begin(NIXSHORTOPTWS);
                                                         }

<NIXSHORTOPT>{PRINT_MINUS_WS}+                           { // -ab
                                                            cont_option(yytext, yyleng);
                                                            my_begin(NIXSHORTOPTSEQ);
                                                         }

<NIXSHORTOPT><<EOF>>                                     { // -a
                                                            if ( aalclp_on_nix_short_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPT>(?s:.)                                      {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXSHORTOPTWS>"-"                                       { // -a -
                                                            if ( aalclp_on_nix_short_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            begin_option('-');
                                                            my_begin(DASH);
                                                         }

<NIXSHORTOPTWS>{PRINT_MINUS_WS_DASH}{PRINT_MINUS_WS}*    { // -a abc
                                                            strncpy(argument, yytext, sizeof(argument));
                                                            if ( aalclp_on_nix_short_option(yyscanner, option, argument) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            end_argument();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPTWS><<EOF>>                                   { // -a <EOF>
                                                            if ( aalclp_on_nix_short_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPTWS>(?s:.)                                    {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXSHORTOPTSEQ>{WS}+                                    { // -abc
                                                            my_begin(NIXSHORTOPTSEQWS);
                                                         }

<NIXSHORTOPTSEQ>{PRINT_MINUS_WS}+                        { // -abcd
                                                            cont_option(yytext, yyleng);
                                                         }

<NIXSHORTOPTSEQ><<EOF>>                                  { // -abc<EOF>
                                                            char *p = option + 1;
                                                            char buf[3];

                                                            while ( *p ) {
                                                               buf[0] = '-';
                                                               buf[1] = *p;
                                                               buf[2] = 0;

                                                               if ( aalclp_on_nix_short_option_only(yyscanner, buf) ) {
                                                                  yyterminate();
                                                               }

                                                               ++p;
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPTSEQ>(?s:.)                                   {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXSHORTOPTSEQWS>"-"                                    { // -abc -
                                                            char *p = option + 1;
                                                            char buf[3];

                                                            while ( *p ) {
                                                               buf[0] = '-';
                                                               buf[1] = *p;
                                                               buf[2] = 0;

                                                               if ( aalclp_on_nix_short_option_only(yyscanner, buf) ) {
                                                                  yyterminate();
                                                               }

                                                               ++p;
                                                            }

                                                            begin_option('-');
                                                            my_begin(DASH);
                                                         }

<NIXSHORTOPTSEQWS>{PRINT_MINUS_WS_DASH}{PRINT_MINUS_WS}* { // -abc xyz
                                                            char *p = option + 1;
                                                            char buf[3];

                                                            while ( *p ) {
                                                               buf[0] = '-';
                                                               buf[1] = *p;
                                                               buf[2] = 0;

                                                               strncpy(argument, yytext, sizeof(argument));

                                                               if ( aalclp_on_nix_short_option(yyscanner, buf, argument) ) {
                                                                  yyterminate();
                                                               }

                                                               ++p;
                                                            }

                                                            end_option();
                                                            end_argument();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPTSEQWS><<EOF>>                                { // -abc <EOF>
                                                            char *p = option + 1;
                                                            char buf[3];

                                                            while ( *p ) {
                                                               buf[0] = '-';
                                                               buf[1] = *p;
                                                               buf[2] = 0;

                                                               if ( aalclp_on_nix_short_option_only(yyscanner, buf) ) {
                                                                  yyterminate();
                                                               }

                                                               ++p;
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXSHORTOPTSEQWS>(?s:.)                                 {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }

<DASHDASH>{WS}                                           { // --
                                                            yyless(0);

                                                            if ( aalclp_on_dash_dash_only(yyscanner) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<DASHDASH>"-"                                            { // ---
                                                            cont_option(yytext, yyleng);
                                                            my_begin(NONOPTION);
                                                         }
 
<DASHDASH>{PRINT_MINUS_WS_DASH}                          {  // --abc
                                                            cont_option(yytext, yyleng);
                                                            my_begin(NIXLONGOPT);
                                                         }

<DASHDASH><<EOF>>                                        { // --<EOF>
                                                            if ( aalclp_on_dash_dash_only(yyscanner) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<DASHDASH>(?s:.)                                         {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXLONGOPT>{WS}+                                        { // --abc
                                                            my_begin(NIXLONGOPTWS);
                                                         }

<NIXLONGOPT>"="                                          { // --abcd=
                                                            my_begin(NIXLONGOPTWS);
                                                         }

<NIXLONGOPT>{PRINT_MINUS_WS_EQU}+                        { // --abcd
                                                            cont_option(yytext, yyleng);
                                                         }

<NIXLONGOPT><<EOF>>                                      { // --abc<EOF>
                                                            if ( aalclp_on_nix_long_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXLONGOPT>(?s:.)                                       {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NIXLONGOPTWS>"-"                                        { // --abc -
                                                            if ( aalclp_on_nix_long_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            begin_option('-');
                                                            my_begin(DASH);
                                                         }

<NIXLONGOPTWS>{PRINT_MINUS_WS_DASH}{PRINT_MINUS_WS}*     { // --abc xyz
                                                            strncpy(argument, yytext, sizeof(argument));
                                                            if ( aalclp_on_nix_long_option(yyscanner, option, argument) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            end_argument();
                                                            my_begin(INITIAL);
                                                         }

<NIXLONGOPTWS><<EOF>>                                    { // --abc <EOF>
                                                            if ( aalclp_on_nix_long_option_only(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NIXLONGOPTWS>(?s:.)                                     {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }


<NONOPTION>{WS}                                          {
                                                            yyless(0);
                                                            if ( aalclp_on_non_option(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NONOPTION>{PRINT_MINUS_WS}+                             {
                                                            cont_option(yytext, yyleng);
                                                         }

<NONOPTION><<EOF>>                                       {
                                                            if ( aalclp_on_non_option(yyscanner, option) ) {
                                                               yyterminate();
                                                            }

                                                            end_option();
                                                            my_begin(INITIAL);
                                                         }

<NONOPTION>(?s:.)                                        {
                                                            if ( aalclp_on_invalid(yyscanner, yytext, yyleng) ) {
                                                               yyterminate();
                                                            }
                                                         }

%%

#define aalclp_offset_of(__cont_type, __linksname) \
(unsigned long)( &( ((__cont_type *)((void *)0))->__linksname ) )

#define aalclp_container_of(__cont_type, __linksptr, __linksname) \
(__cont_type *) ( (unsigned char *)(__linksptr) - aalclp_offset_of(__cont_type, __linksname) )

#define aalclp_list_init(__l) (__l)->prev = (__l)->next = (__l)

#define aalclp_list_enqueue(__l, __x) \
do                                    \
{                                     \
   (__x)->prev       = (__l)->prev;   \
   (__x)->next       = (__l);         \
   (__l)->prev->next = (__x);         \
   (__l)->prev       = (__x);         \
}while(0)

#define aalclp_list_for_each_entry(__etype, __eptr, __l, __i)                      \
for ( (__i) = (__l)->next, (__eptr) = aalclp_container_of(__etype, __i, links) ;   \
         (__i) != (__l) ;                                                          \
            (__i) = (__i)->next, (__eptr) = aalclp_container_of(__etype, __i, links) )

static int
aalclp_init_callbacks(aalclp_callbacks *cb) {
   aalclp_list_init(&cb->listhead_nix_long_option_only.links);
   aalclp_list_init(&cb->listhead_nix_long_option.links);
   aalclp_list_init(&cb->listhead_nix_short_option_only.links);
   aalclp_list_init(&cb->listhead_nix_short_option.links);
   aalclp_list_init(&cb->listhead_dash_only.links);
   aalclp_list_init(&cb->listhead_dash_dash_only.links);
   aalclp_list_init(&cb->listhead_win_long_option_only.links);
   aalclp_list_init(&cb->listhead_win_long_option.links);
   aalclp_list_init(&cb->listhead_win_short_option_only.links);
   aalclp_list_init(&cb->listhead_win_short_option.links);
   aalclp_list_init(&cb->listhead_non_option.links);
   aalclp_list_init(&cb->listhead_invalid.links);
   return 0;
}

#define AALCLP_ADD(__what, __type)                                                        \
int aalclp_add_##__what(aalclp *scanner, __type *node) {                                  \
   if ( (NULL != node) && (NULL != node->cb) ) {                                          \
      aalclp_list_enqueue(&(scanner->callbacks.listhead_##__what.links), &(node)->links); \
      return 0;                                                                           \
   }                                                                                      \
   errno = EFAULT;                                                                        \
   return 1;                                                                              \
}


AALCLP_ADD(nix_long_option_only,  aalclp_option_only)
AALCLP_ADD(nix_long_option,       aalclp_option)
AALCLP_ADD(nix_short_option_only, aalclp_option_only)
AALCLP_ADD(nix_short_option,      aalclp_option)
AALCLP_ADD(dash_only,             aalclp_dash_only)
AALCLP_ADD(dash_dash_only,        aalclp_dash_only)
AALCLP_ADD(win_long_option_only,  aalclp_option_only)
AALCLP_ADD(win_long_option,       aalclp_option)
AALCLP_ADD(win_short_option_only, aalclp_option_only)
AALCLP_ADD(win_short_option,      aalclp_option)
AALCLP_ADD(non_option,            aalclp_non_option)
AALCLP_ADD(invalid,               aalclp_invalid)

int
aalclp_init(aalclp *scanner) {
   if ( NULL == scanner ) {
      errno = EFAULT;
      return 1;
   }

   scanner->scanner = NULL;
   aalclp_init_callbacks(&scanner->callbacks);

   return yylex_init_extra((YY_EXTRA_TYPE)scanner, &scanner->scanner);
}

int
aalclp_destroy(aalclp *scanner) {
   int res;

   if ( NULL == scanner ) {
      errno = EFAULT;
      return 1;
   }

   res = yylex_destroy(scanner->scanner);

   aalclp_init_callbacks(&scanner->callbacks);
   scanner->scanner = NULL;

   return res;
}

int
aalclp_scan_argv(aalclp *scanner, int argc, char *argv[]) {
   int             i;
   size_t          len;
   char           *scanbuf;
   char           *end;
   YY_BUFFER_STATE buf;

   len = 0;
   for ( i = 1 ; i < argc ; ++i ) {
      len += strlen(argv[i]);
      if ( i < (argc - 1) ) {
         ++len;
      }
   }

   if ( 0 == len ) {
      return 1;
   }

   scanbuf = (char *)malloc(len + 2);
   if ( NULL == scanbuf ) {
      return 2;
   }
   *scanbuf = 0;

   for ( i = 1 ; i < argc ; ++i ) {
      strcat(scanbuf, argv[i]);
      if ( i < (argc - 1) ) {
         strcat(scanbuf, " ");
      }
   }

   end = scanbuf + strlen(scanbuf);
   *end++ = YY_END_OF_BUFFER_CHAR;
   *end   = YY_END_OF_BUFFER_CHAR;

   buf = yy_scan_string(scanbuf, scanner->scanner);
   free(scanbuf);

   i = yylex(scanner->scanner);
   yy_delete_buffer(buf, scanner->scanner);

   return i;
}

int
aalclp_scan_file(aalclp *scanner, FILE *in, FILE *out) {
   yyset_in(in, scanner->scanner);
   yyset_out(out, scanner->scanner);
   return yylex(scanner->scanner);
}


#define AALCLP_HANDLER0(__what, __type)                            \
static int                                                         \
aalclp_on_##__what(yyscan_t yyscanner) {                           \
   aalclp       *clp  = (aalclp *)yyget_extra(yyscanner);          \
   aalclp_links *head = &(clp->callbacks.listhead_##__what.links); \
   aalclp_links *iter;                                             \
   __type       *node;                                             \
   int           res;                                              \
   aalclp_list_for_each_entry(__type, node, head, iter) {          \
      res = node->cb(node->user);                                  \
      if ( 0 != res ) {                                            \
         return res;                                               \
      }                                                            \
   }                                                               \
   return res;                                                     \
}

#define AALCLP_HANDLER1(__what, __type, __param1type, __param1)    \
static int                                                         \
aalclp_on_##__what(yyscan_t yyscanner, __param1type __param1) {    \
   aalclp       *clp  = (aalclp *)yyget_extra(yyscanner);          \
   aalclp_links *head = &(clp->callbacks.listhead_##__what.links); \
   aalclp_links *iter;                                             \
   __type       *node;                                             \
   int           res;                                              \
   aalclp_list_for_each_entry(__type, node, head, iter) {          \
      res = node->cb(node->user, __param1);                        \
      if ( 0 != res ) {                                            \
         return res;                                               \
      }                                                            \
   }                                                               \
   return res;                                                     \
}

#define AALCLP_HANDLER2(__what, __type, __param1type, __param1, __param2type, __param2) \
static int                                                                              \
aalclp_on_##__what(yyscan_t yyscanner, __param1type __param1, __param2type __param2) {  \
   aalclp       *clp  = (aalclp *)yyget_extra(yyscanner);                               \
   aalclp_links *head = &(clp->callbacks.listhead_##__what.links);                      \
   aalclp_links *iter;                                                                  \
   __type       *node;                                                                  \
   int           res;                                                                   \
   aalclp_list_for_each_entry(__type, node, head, iter) {                               \
      res = node->cb(node->user, __param1, __param2);                                   \
      if ( 0 != res ) {                                                                 \
         return res;                                                                    \
      }                                                                                 \
   }                                                                                    \
   return res;                                                                          \
}

AALCLP_HANDLER1(nix_long_option_only,  aalclp_option_only, const char *, option)
AALCLP_HANDLER2(nix_long_option,       aalclp_option,      const char *, option, const char *, value)

AALCLP_HANDLER1(nix_short_option_only, aalclp_option_only, const char *, option)
AALCLP_HANDLER2(nix_short_option,      aalclp_option,      const char *, option, const char *, value)

AALCLP_HANDLER0(dash_only,             aalclp_dash_only)
AALCLP_HANDLER0(dash_dash_only,        aalclp_dash_only)

AALCLP_HANDLER1(win_long_option_only,  aalclp_option_only, const char *, option)
AALCLP_HANDLER2(win_long_option,       aalclp_option,      const char *, option, const char *, value)

AALCLP_HANDLER1(win_short_option_only, aalclp_option_only, const char *, option)
AALCLP_HANDLER2(win_short_option,      aalclp_option,      const char *, option, const char *, value)

AALCLP_HANDLER1(non_option,            aalclp_non_option,  const char *, nonoption)

AALCLP_HANDLER2(invalid,               aalclp_invalid,     const char *, invalid, size_t, len)

