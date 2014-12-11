// Copyright (c) 2008-2014, Intel Corporation
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
/// @file Utilities.h
/// @brief Handy little miscellaneous things needed in lots of places.
/// @ingroup AASUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/04/2008     HM       Initial version started
/// 11/20/2008     JG       Added SmartPtr home brew
/// 11/27/2008     HM       Added RDTSC (standard Intel code), + getTSC home brew
/// 01/04/2009     HM       Updated Copyright
/// 10/21/2011     JG       Added __linux__ for Windows port
/// 10/12/2014     HM       Moved memory #defines in from SingleAFUApp.h@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_UTILITIES_H__
#define __AALSDK_UTILS_UTILITIES_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup AASUtils
/// @{

#ifndef CACHELINE_BYTES
# define CACHELINE_BYTES 64
#endif // CACHELINE_BYTES
#ifndef LOG2_CL
# define LOG2_CL         6
#endif // LOG2_CL
#ifndef CACHELINE_ALIGNED_ADDR
#define CACHELINE_ALIGNED_ADDR(p) ((p) >> LOG2_CL)
#endif // CACHELINE_ALIGNED_ADDR
#ifndef CL
# define CL(x) ((x)   * CACHELINE_BYTES)
#endif // CL
#ifndef KB
# define KB(x) ((x)   * __UINT64_T_CONST(1024))
#endif // KB
#ifndef MB
# define MB(x) (KB(x) * __UINT64_T_CONST(1024))
#endif // MB
#ifndef GB
# define GB(x) (MB(x) * __UINT64_T_CONST(1024))
#endif // GB


#if !defined(NUM_ELEMENTS)
#  define NUM_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#endif

//#if __GNUC__ >= 3
//#  if !defined(likely)
//#     define likely(x)   __builtin_expect (!!(x), 1)
//#  endif
//#  if !defined(unlikely)
//#     define unlikely(x) __builtin_expect (!!(x), 0)
//#  endif
//#else
//#  if !defined(likely)
//#     define likely(x)   (x)
//#  endif
//#  if !defined(unlikely)
//#     define unlikely(x) (x)
//#  endif
//#endif

// Read Time Stamp Clock atomically
#ifndef RDTSC
#if defined( __GNUC__ )
	#define RDTSC(low, high) \
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#else
	#define RDTSC(low, high)
#endif
#endif

/// Read Time Stamp Clock unified. Doesn't affect the RDTSC, but makes math easier.
static inline unsigned long long getTSC(void)
{
   unsigned low, high;
   RDTSC( low, high);
   return static_cast<unsigned long long>(low) | (static_cast<unsigned long long>(high) << 32);
}

#if DEPRECATED
//=============================================================================
// Name: SmartPtr
// Description: Smart pointer template.
// Interface: public
// Comments:  Pro: More fool proof as the counted object must be derived from
//                 the special class. Can still screw up by deleting derived
//                 explicitly, passing an address of a temporary.
//                 Can safetly make a new SP from another SP or the original
//                 object. Can be mixed with direct use of the counted object
//            Con: Only works with special objects.
//      Can still screw up if you make an automatic, had a SP to it and
//      the let it delete.
//=============================================================================
template<class T> class SmartPtr
{
public:
   SmartPtr(){
      m_ptr=NULL;
   };

   SmartPtr(T *pObj) {
     //ASSERT(dynamic_cast<CCountedObject*>(pObj) != NULL);
     m_ptr=pObj; dynamic_cast<CCountedObject*>(m_ptr)->IncRef();
   }

   // Not  & const rOther since we do change rOther indirectly
   SmartPtr(SmartPtr &rOther){
      m_ptr=rOther.m_ptr;
      m_ptr->incRef();
   }

   ~SmartPtr(){
      m_ptr->decRef();
   }

   SmartPtr & operator = (const SmartPtr &rOther) {
      if(m_ptr!=NULL){
         dynamic_cast<CCountedObject*>(m_ptr)->DecRef();
      }
      m_ptr=rOther->m_ptr;
      m_ptr->incRef();
      return *this;
   }

  SmartPtr& operator =(T *pObj){
      //ASSERT(dynamic_cast<CCountedObject*>(pObj) != NULL);
      if(m_ptr!=NULL){
         dynamic_cast<CCountedObject*>(m_ptr)->DecRef();
      }
      m_ptr=pObj;
      dynamic_cast<CCountedObject*>(m_ptr)->IncRef();
      return *this;
   }

   T& operator*(){
      return *m_ptr;
   }

   T* operator->(){
      return m_ptr;
   }

private:
   T *m_ptr;
};
#endif // DEPRECATED

#if DEPRECATED
//=============================================================================
// Name: SmartPtrWrapper
// Description: Smart pointer wrapper template.
// Interface: public
// Comments: Pros: Can point to anything
//           Cons: Expects an object that was newed because IT WILL delete.
//                 You can create 2 independent SPs to this object and be hosed.
// Care must be taken using the object without the SP because it can be deleted
// by the last "known" referencer.
//=============================================================================
template<typename T> class SmartPtrWrapper
{

   // Counter template class that wraps the normal pointer
   template<typename U> class CountedObject
   {
   public:
      CountedObject(U* const pObj): m_ptr(pObj), m_count(1){};

      void incRef() {m_count++;}

      // Dec ref will delete the object when it goes to zero
      void decRef(){
         if(--m_count == 0){
            // delete the object so it had better have been from a deletable object (e.g., newed);
            delete m_ptr;
         }
      }

      U* ptr() {return m_ptr;}

   private:
      U* m_ptr;
      unsigned int m_count;

   protected:
   virtual ~CountedObject(){};

   };

   //==========================================================================
   // Rest of the smart pointer class
   //==========================================================================
public:
   // Empty constructor
   SmartPtrWrapper(){
      m_ptr=NULL;
   };

   //Constructor from an object pointer - Wraps the pointer in a counter
   SmartPtrWrapper(T *pObj) {
      //Creates an instance of the wrapper
      m_ptr=new CountedObject<T>(pObj);
   }

   // Copy contructor
   SmartPtrWrapper(SmartPtrWrapper & rOther){
      m_ptr=rOther.m_ptr;
      if(m_ptr != NULL){
         m_ptr->incRef();
      }
   }

   virtual ~SmartPtrWrapper(){
      // Decrement the wrapper - It will self destruct if it goes to 0
      if(m_ptr != NULL){
         m_ptr->decRef();
      }
   }

   // Assigment checks to see if the pointer has been assigned and desc it
   SmartPtrWrapper & operator = (const SmartPtrWrapper &rOther){
      if(m_ptr!=NULL){
        m_ptr->decRef();
      }
      m_ptr=rOther.m_ptr;
      m_ptr->incRef();
      return *this;
   }

   // Assigning a new type will make a new counter object
   SmartPtrWrapper& operator =(T *pObj){
      if(m_ptr!=NULL){
         m_ptr->decRef();
      }

      // Assigning NULL to the pointer will free the previously pointed to object
      if(pObj==NULL){
         // Don't bother wrapping it
         m_ptr=NULL;
      }else{
         m_ptr=new CountedObject<T>(pObj);
         m_ptr->incRef();
      }
      return *this;
   }

   // Operator to make the smart pointer look pointer-ish.

   // Dereference will return a reference to the inner object
   T& operator  *(){
      return *m_ptr->ptr();
   }

   T* operator ->(){
      return m_ptr->ptr();
   }

//   operator int() {return 1;}

protected:
   operator SmartPtrWrapper<T> &(){};
   CountedObject<T> *m_ptr;

};
#endif // DEPRECATED

/// @} group AASUtils

#endif // __AALSDK_UTILS_UTILITIES_H__

