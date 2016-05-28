// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_STREAM_H__
#define __GTCOMMON_STREAM_H__

// A mix-in class instantiated/inherited by TNVSTester and TArrayNVSTester (gtNVSTester.h)
// Instantiating as IOStreamMixin< std::stringstream > gives the ability to easily exercise
// the output streamer fn's in NamedValueSet, then immediately consume from the same object for
// verification.
// Member functions allow querying and clearing the common ios_base::iostate flags and determining
// the number of bytes available on the stream object.
template <typename S>
class IOStreamMixin
{
public:
   IOStreamMixin() :
      m_IOStream(std::ios_base::in|std::ios_base::out|std::ios_base::binary)
   {}

   std::ostream & os() { return dynamic_cast<std::ostream &>(m_IOStream); }
   std::istream & is() { return dynamic_cast<std::istream &>(m_IOStream); }

   const std::ostream & os() const { return dynamic_cast<const std::ostream &>(m_IOStream); }
   const std::istream & is() const { return dynamic_cast<const std::istream &>(m_IOStream); }

   void CheckO(std::ios_base::iostate check) const
   {
      const std::ios_base::iostate common = os().rdstate() & check;
      if ( 0 != common ) {
         std::string flags;
         if ( common & std::ios_base::eofbit ) {
            flags += "eofbit ";
         }
         if ( common & std::ios_base::failbit ) {
            flags += "failbit ";
         }
         if ( common & std::ios_base::badbit ) {
            flags += "badbit ";
         }
         FAIL() << "ostream state: " << flags;
      }
   }

   void CheckI(std::ios_base::iostate check) const
   {
      const std::ios_base::iostate common = is().rdstate() & check;
      if ( 0 != common ) {
         std::string flags;
         if ( common & std::ios_base::eofbit ) {
            flags += "eofbit ";
         }
         if ( common & std::ios_base::failbit ) {
            flags += "failbit ";
         }
         if ( common & std::ios_base::badbit ) {
            flags += "badbit ";
         }
         FAIL() << "istream state: " << flags;
      }
   }

   bool  eof() const { return is().eof(); }
   bool fail() const { return os().fail() || is().fail(); }
   bool  bad() const { return os().bad()  || is().bad();  }

   void ClearEOF()
   {
      os().clear( os().rdstate() & ~std::ios_base::eofbit );
      is().clear( is().rdstate() & ~std::ios_base::eofbit );
   }
   void ClearFail()
   {
      os().clear( os().rdstate() & ~std::ios_base::failbit );
      is().clear( is().rdstate() & ~std::ios_base::failbit );
   }
   void ClearBad()
   {
      os().clear( os().rdstate() & ~std::ios_base::badbit );
      is().clear( is().rdstate() & ~std::ios_base::badbit );
   }

   std::ios_base::streampos InputBytesRemaining()
   {
      const std::ios_base::streampos curpos = is().tellg();

      is().seekg(0, std::ios_base::end);

      const std::ios_base::streampos endpos = is().tellg();

      is().seekg(curpos, std::ios_base::beg);

      return endpos - curpos;
   }

protected:
   S m_IOStream;
};


// A mix-in class inherited by TNVSTester and TArrayNVSTester (gtNVSTester.h)
// Implementing this class provides a facility for creating one or more temporary C-style
// FILE objects.
// The class is used in testing the FILE * support provided by INamedValueSet when preprocessor
// constant NVSFileIO (aaluser/include/aalsdk/INamedValueSet.h) is defined.
class GTCOMMON_API FILEMixin
{
public:
   FILEMixin() {}
   virtual ~FILEMixin();

   FILE * fopen_tmp();
   btBool    fclose(FILE * );

   void rewind(FILE * ) const;
   int    feof(FILE * ) const;
   int  ferror(FILE * ) const;

   long InputBytesRemaining(FILE * ) const;

protected:
   struct FILEInfo
   {
      FILEInfo(std::string fname, int fd) :
         m_fname(fname),
         m_fd(fd)
      {}
      std::string m_fname;
      int         m_fd;
   };

   typedef std::map< FILE * , FILEInfo > map_type;
   typedef map_type::iterator            iterator;
   typedef map_type::const_iterator      const_iterator;

   map_type m_FileMap;
};

#endif // __GTCOMMON_STREAM_H__

