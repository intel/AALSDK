// Copyright(c) 2013-2016, Intel Corporation
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
//****************************************************************************
// @file scanner.c
// @brief Unit Test for aalclp.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 07/20/2013     TSW      Initial version.
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <stdio.h>

#include "aalsdk/aalclp/aalclp.h"

int ParseCmdLine(int argc, char *argv[]);
int ParseFile(FILE *in, FILE *out);

int main(int argc, char *argv[])
{
   if ( argc > 1 ) {
      return ParseCmdLine(argc, argv);
   }

   return ParseFile(stdin, stdout);
}

#define ASSERT(x)                                                   \
do                                                                  \
{                                                                   \
   if ( !(x) ) {                                                    \
      fprintf(stderr, "(line %d) ASSERT fail: %s\n", __LINE__, #x); \
   }                                                                \
}while(0)

struct _my_user_defined_struct
{
   int x;
};
static struct _my_user_defined_struct user_defined;

static int
on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option) {
   ASSERT(&user_defined == user);
   printf("*nix long option: %s\n", option);
   return 0;
}
static aalclp_option_only nix_long_option_only = { on_nix_long_option_only, &user_defined, };

static int
on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   ASSERT(&user_defined == user);
   printf("*nix long option/value: %s %s\n", option, value);
   return 0;
}
static aalclp_option nix_long_option = { on_nix_long_option, &user_defined, };

static int
on_nix_short_option_only(AALCLP_USER_DEFINED user, const char *option) {
   ASSERT(&user_defined == user);
   printf("*nix short option: %s\n", option);
   return 0;
}
static aalclp_option_only nix_short_option_only = { on_nix_short_option_only, &user_defined, };

static int
on_nix_short_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   ASSERT(&user_defined == user);
   printf("*nix short option/value: %s %s\n", option, value);
   return 0;
}
static aalclp_option nix_short_option = { on_nix_short_option, &user_defined, };

static int
on_dash_only(AALCLP_USER_DEFINED user) {
   ASSERT(&user_defined == user);
   printf("dash only\n");
   return 0;
}
static aalclp_dash_only dash_only = { on_dash_only, &user_defined, };

static int
on_dash_dash_only(AALCLP_USER_DEFINED user) {
   ASSERT(&user_defined == user);
   printf("dash dash only\n");
   return 0;
}
static aalclp_dash_only dash_dash_only = { on_dash_dash_only, &user_defined, };

static int
on_win_long_option_only(AALCLP_USER_DEFINED user, const char *option) {
   ASSERT(&user_defined == user);
   printf("Win long option: %s\n", option);   
   return 0;
}
static aalclp_option_only win_long_option_only = { on_win_long_option_only, &user_defined, };

static int
on_win_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   ASSERT(&user_defined == user);
   printf("Win long option/value: %s %s\n", option, value);
   return 0;
}
static aalclp_option win_long_option = { on_win_long_option, &user_defined, };

static int
on_win_short_option_only(AALCLP_USER_DEFINED user, const char *option) {
   ASSERT(&user_defined == user);
   printf("Win short option: %s\n", option);
   return 0;
}
static aalclp_option_only win_short_option_only = { on_win_short_option_only, &user_defined, };

static int
on_win_short_option(AALCLP_USER_DEFINED user, const char *option, const char *value) {
   ASSERT(&user_defined == user);
   printf("Win short option/value: %s %s\n", option, value);
   return 0;
}
static aalclp_option win_short_option = { on_win_short_option, &user_defined, };

static int
on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   ASSERT(&user_defined == user);
   printf("Non-option: %s\n", nonoption);
   return 0;
}
static aalclp_non_option non_option = { on_non_option, &user_defined, };

static int
on_invalid(AALCLP_USER_DEFINED user, const char *invalid, size_t len) {
   size_t i;

   ASSERT(&user_defined == user);

   for ( i = 0 ; i < len ; ++i ) {
      printf("INVALID: %c\n", invalid[i]);
   }

   return 0;
}
static aalclp_invalid invalid = { on_invalid, &user_defined, };


int ParseCmdLine(int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_init() failed : %d %s\n", res, strerror(res));
      return res;
   }

   aalclp_add_nix_long_option_only(&clp,  &nix_long_option_only);
   aalclp_add_nix_long_option(&clp,       &nix_long_option);
   aalclp_add_nix_short_option_only(&clp, &nix_short_option_only);
   aalclp_add_nix_short_option(&clp,      &nix_short_option);
   aalclp_add_dash_only(&clp,             &dash_only);
   aalclp_add_dash_dash_only(&clp,        &dash_dash_only);
   aalclp_add_win_long_option_only(&clp,  &win_long_option_only);
   aalclp_add_win_long_option(&clp,       &win_long_option);
   aalclp_add_win_short_option_only(&clp, &win_short_option_only);
   aalclp_add_win_short_option(&clp,      &win_short_option);
   aalclp_add_non_option(&clp,            &non_option);
   aalclp_add_invalid(&clp,               &invalid);

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_scan_argv() failed : %d %s\n", res, strerror(res));
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      fprintf(stderr, "aalclp_destroy() failed : %d %s\n", clean, strerror(clean));
   }

   return res;
}

void help_message_cb(FILE *fp, aalclp_gcs_compliance_data *p)
{
   fprintf(fp, "Usage: %s ...\n", p->application);
}

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "scanner",
                             AALCLP_VERSION,
                             "License example",
                             help_message_cb,
                             NULL)

int ParseFile(FILE *in, FILE *out)
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_init() failed : %d %s\n", res, strerror(res));
      return res;
   }

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_add_gcs_compliance() failed : %d %s\n", res, strerror(res));
      goto CLEANUP;
   }

   res = aalclp_scan_file(&clp, in, out);
   if ( 0 != res ) {
      fprintf(stderr, "aalclp_scan_file() failed : %d %s\n", res, strerror(res));
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      fprintf(stderr, "aalclp_destroy() failed : %d %s\n", clean, strerror(clean));
   }

   return res;
}

