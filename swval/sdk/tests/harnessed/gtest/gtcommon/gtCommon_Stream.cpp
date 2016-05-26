// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

FILEMixin::~FILEMixin()
{
   iterator iter;
   for ( iter = m_FileMap.begin() ; iter != m_FileMap.end() ; ++iter ) {
      ::fclose((*iter).first);
      ::remove((*iter).second.m_fname.c_str());
   }
}

FILE * FILEMixin::fopen_tmp()
{
   int   fd;
   FILE *fp = NULL;

#if   defined( __AAL_WINDOWS__ )

   char tmpdir[265];
   char fname[MAX_PATH];
   const char *prefix = "gtest";

   if ( !GetTempPath(sizeof(tmpdir), tmpdir) ) {
      return NULL;
   }

   if ( !GetTempFileName(tmpdir, prefix, 0, fname) ) {
      return NULL;
   }

   if ( ::fopen_s(&fp, fname, "w+b") ) {
      return NULL;
   }

   fd = _fileno(fp);

#elif defined( __AAL_LINUX__ )

   char fname[13] = { 'g', 't', 'e', 's', 't', '.', 'X', 'X', 'X', 'X', 'X', 'X', 0 };

   fd = ::mkstemp(fname);

   if ( -1 == fd ) {
      return NULL;
   }

   fp = ::fdopen(fd, "w+b");

   if ( NULL == fp ) {
      ::close(fd);
      ::remove(fname);
      return NULL;
   }

#endif // OS

   FILEInfo info(fname, fd);

   std::pair<iterator, bool> res = m_FileMap.insert(std::make_pair(fp, info));

   if ( !res.second ) {
      ::fclose(fp);
      ::remove(fname);
      return NULL;
   }

   return fp;
}

btBool FILEMixin::fclose(FILE *fp)
{
   iterator iter = m_FileMap.find(fp);

   if ( m_FileMap.end() == iter ) {
      // fp not found
      return false;
   }

   ::fclose(fp);
   ::remove((*iter).second.m_fname.c_str());

   m_FileMap.erase(iter);

   return true;
}

void FILEMixin::rewind(FILE *fp) const
{
   ::rewind(fp);
}

int FILEMixin::feof(FILE *fp) const
{
   return ::feof(fp);
}

int FILEMixin::ferror(FILE *fp) const
{
   return ::ferror(fp);
}

long FILEMixin::InputBytesRemaining(FILE *fp) const
{
   long curpos = ::ftell(fp);

   ::fseek(fp, 0, SEEK_END);

   long endpos = ::ftell(fp);

   ::fseek(fp, curpos, SEEK_SET);

   return endpos - curpos;
}

