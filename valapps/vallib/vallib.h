// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __VALLIB_H__
#define __VALLIB_H__
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/utils/NLBVAFU.h>

using namespace std;
using namespace AAL;

#define HWAFU
//#define  ASEAFU


#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


// Definition of an object responsible for allocating an AAL Service.
// Subclasses override virtual member fn Allocate() to specify which AAL Service to allocate.
/*
Usage:
   MyAllocatesClass Allocator;

   Allocator.Allocate(&m_Runtime); // pass a pointer to your started AAL Runtime object.
   Allocator.Wait(); // must wait for the Service to be available before calling Service().
   IBase *pAALService = Allocator.Service();

   ASSERT(NULL != pAALService);
   ASSERT(0 == Allocator.Errors());

   (use pAALService)

   Allocator.Free();
   Allocator.Wait(); // must wait for the Service to be freed.
*/
class AllocatesAALService : public AAL::CAASBase,
                            public AAL::IServiceClient
{
public:
   AllocatesAALService();
   virtual ~AllocatesAALService() {}

   virtual void                   Allocate(AAL::IRuntime * ) = 0;
   virtual const char * ServiceDescription() const           = 0;

   void            Wait()       { m_Sem.Wait();         }
   AAL::IBase * Service() const { return m_pAALService; }
   AAL::btInt    Errors() const { return m_Errors;      }

   void Free();

   // <IServiceClient>
   void      serviceAllocated(AAL::IBase               *pServiceBase,
                              AAL::TransactionID const &rTranID);
   void serviceAllocateFailed(const AAL::IEvent        &rEvent);
   void       serviceReleased(const AAL::TransactionID &rTranID);
   void serviceReleaseRequest(AAL::IBase               *pServiceBase,
                              const AAL::IEvent        &rEvent);
   void  serviceReleaseFailed(const AAL::IEvent        &rEvent);
   void          serviceEvent(const AAL::IEvent        &rEvent);
   // </IServiceClient>

protected:
   AAL::IBase      *m_pAALService;
   AAL::btInt       m_Errors;
   AAL::CSemaphore  m_Sem;
};

// Allocates an NLB Lpbk1 AAL Service.
// keyRegAFU_ID : C000C966-0D82-4272-9AEF-FE5F84570612
class AllocatesNLBLpbk1AFU : public AllocatesAALService
{
public:
   AllocatesNLBLpbk1AFU() {}
   virtual void                   Allocate(AAL::IRuntime * );
   virtual const char * ServiceDescription() const { return "NLB Lpbk1"; }
};

// Allocates the FME AAL Service.
// keyRegAFU_ID : BFAF2AE9-4A52-46E3-82FE-38F0F9E17764
class AllocatesFME : public AllocatesAALService
{
public:
   AllocatesFME() {}
   virtual void                   Allocate(AAL::IRuntime * );
   virtual const char * ServiceDescription() const { return "FME"; }
};


// Allocates the PORT AAL Service.
// keyRegAFU_ID : 3AB49893-138D-42EB-9642-B06C6B355B87
class AllocatesPort : public AllocatesAALService
{
public:
   AllocatesPort() {}
   virtual void                   Allocate(AAL::IRuntime * );
   virtual const char * ServiceDescription() const { return "PORT"; }
};

////////////////////////////////////////////////////////////////////////////////

// Definition of an object that can perform a basic NLB Lpbk1 (mode 0) transfer.
// The IBase * parameter to the constructor must be that for an NLB Lpbk1 Service. (see AllocatesNLBLpbk1AFU)
// Subclasses can override virtual member fn AllocateBuffers() to perform specialized
//  buffer initialization.
/*
Usage:
   DoesNLBLpbk1 lpbk1(pAALService);
   btInt Errors = lpbk1.NLBLpbk1();
   if ( 0 == Errors ) {
      // Success
   }
*/
class DoesNLBLpbk1
{
public:
   DoesNLBLpbk1(AAL::IBase   *pAALService,
                AAL::btWSSize TransferSize=CL(1),    // 1 cache line by default
                AAL::btWSSize DSMSize=NLB_DSM_SIZE); // from NLBVAFU.h

   AAL::btInt NLBLpbk1(); // return 0 on success

protected:

   virtual void AllocateBuffers();
   virtual void FreeBuffers();

   AAL::IBase      *m_pAALService;    ///< IBase for valid AAL Service to NLB VAFU.
   AAL::btInt       m_Errors;         ///< Tracks number of errors encountered.
   AAL::btWSSize    m_TransferSize;   ///< Input/Output workspace size in bytes.
   AAL::btWSSize    m_DSMSize;        ///< DSM workspace size in bytes.

   AAL::IALIBuffer *m_pALIBufferIfc;  ///< Interface for allocating / freeing buffers.

   AAL::btVirtAddr  m_DSMVirt;
   AAL::btPhysAddr  m_DSMPhys;
   AAL::btVirtAddr  m_InputVirt;
   AAL::btPhysAddr  m_InputPhys;
   AAL::btVirtAddr  m_OutputVirt;
   AAL::btPhysAddr  m_OutputPhys;
};

////////////////////////////////////////////////////////////////////////////////

// Object to convert a NamedValueSet output of IALIPerf::performanceCountersGet() into
//  an index-able array for easier access and more readable streaming.
class PerfCounters
{
public:
   enum Index
   {
      VERSION = 0,
      READ_HIT,
      WRITE_HIT,
      READ_MISS,
      WRITE_MISS,
      EVICTIONS,
      PCIE0_READ,
      PCIE0_WRITE,
      PCIE1_READ,
      PCIE1_WRITE,
      UPI_READ,
      UPI_WRITE
   };

   PerfCounters(const AAL::NamedValueSet & );

   AAL::btBool             Valid(PerfCounters::Index i) const { return m_Values[i].Valid; }
   AAL::btUnsigned64bitInt Value(PerfCounters::Index i) const { return m_Values[i].Value; }

   AAL::btBool operator == (const PerfCounters &rhs) const;
   AAL::btBool operator != (const PerfCounters &rhs) const { return ! this->PerfCounters::operator == (rhs); }

   friend std::ostream & operator << (std::ostream & , const PerfCounters & );

protected:
   struct
   {
      AAL::btBool             Valid;
      AAL::btUnsigned64bitInt Value;
   } m_Values[12];
};

std::ostream & operator << (std::ostream & , const PerfCounters & );

#endif // __VALLIB_H__

