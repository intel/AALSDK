// Copyright (c) 2011-2014, Intel Corporation
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
//        FILE: Sample_EncoderAFUService.h
//     CREATED: 08/17/2011
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Definitions for Sample_Encoder AFU Service.
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __SAMPLEENCODEAFUSERVICE_H__
#define __SAMPLEENCODEAFUSERVICE_H__
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/aas/AALDeviceService.h> // Device Service framework
#include <aalsdk/faptrans/FAP10.h>       // Used for AllocateWorkspace transactions
#include <aalsdk/kernel/encoder_sample_kbae/encoder_kbae-public.h> // Public AFU device interface
#include <aalsdk/service/UtilsAFU.h>

#if defined ( __AAL_WINDOWS__ )
# define SAMPLEENC_VERSION          "0.0.0"
# define SAMPLEENC_VERSION_CURRENT  0
# define SAMPLEENC_VERSION_REVISION 0
# define SAMPLEENC_VERSION_AGE      0
# ifdef SAMPLEENC_EXPORTS
#    define SAMPLEENC_API __declspec(dllexport)
# else
#    define SAMPLEENC_API __declspec(dllimport)
# endif // SAMPLEENC_EXPORTS
#else
# define SAMPLEENC_API    __declspec(0)
#endif // __AAL_WINDOWS__


#define SAMPLEENC_SVC_MOD         "libSampleEncoderAFUService" AAL_SVC_MOD_EXT
#define SAMPLEENC_SVC_ENTRY_POINT "libSampleEncoderAFUService" AAL_SVC_MOD_ENTRY_SUFFIX

AAL_DECLARE_MOD(libSampleEncoderAFUService, SAMPLEENC_API)


BEGIN_NAMESPACE(AAL)

#define iidEncode __INTC_IID(INTC_sysSampleAFU,0x0005)
class IEncode : public IWorkspace
{
public:

   virtual void Encode(char code, TransactionID const &rTranID) = 0;
   virtual ~IEncode() {}
};
#define tranevtEncodeComplete   __INTC_TranEvt  (INTC_sysSampleAFU, 0x0015)
#define extranevtEncodeComplete __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0015)

// AFU ID GUID
#define ENCODER_AFU_IDKEY "00000000-0000-0000-FFFF-0000FFFD0004"

//=============================================================================
// Name: SampleEncoder
// Description: Simple sample AFU (i.e., Device) AAL Service
// Interface: IEncode
// Comments:
//=============================================================================
class SampleEncoder : public DeviceServiceBase, public IEncode
{
public:

   // Macro defines the constructor for a loadable AAL service.
   //  The first argument is your class name, the second argument is the
   //  name of the Service base class this service is derived from. In this
   //  example we use DeviceServiceBase as it is the class that provides the
   //  support for devices.  Software only services might use ServiceBase
   //  instead.
   //
   // Note that initializers can be declared here but are preceded by a comma
   //  rather than a colon.
   //
   // The design pattern is that the constructor do minimal work. Here we are
   //  registering the interfaces the service implements. The default (Subclass)
   //  interface is IEncode.  DeviceServiceBase provides an init() method that
   //  can be used where more sophisticated initialization is required. The
   //  init() method is called by the factory AFTER construction but before use.
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SampleEncoder, DeviceServiceBase)
   ,m_datap(NULL) //NOTE to initialize members use , not :
   {
      SetSubClassInterface(iidEncode, dynamic_cast<IEncode*>(this));
   }

   // Encode interface, its static event handler method and non-static
   //  convenience method
   void Encode(char code, TransactionID const &rTranID);
   static void _EncoderHandler(IEvent const& theEvent);
   void EncoderHandler(IEvent const& theEvent);

   // Workspace interface, its static event handler method and non-static
   //  convenience method
   void AllocateWorkSpace(btWSSize   Size,
                          TransactionID const &TranID=TransactionID());
   void AllocateWorkspaceHandler(IEvent const& theEvent);
   static void _AllocateWorkspaceHandler(IEvent const& theEvent);

   void FreeWorkSpace(btVirtAddr WorkspaceAddress,
                      TransactionID const &TranID=TransactionID());
   static void _FreeWorkSpaceHandler(IEvent const& theEvent);
   void FreeWorkSpaceHandler(IEvent const& theEvent);

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Quite RElease. Used when Service is unloaded
   btBool Release(btTime timeout=AAL_INFINITE_WAIT);

   void init( TransactionID const&rtid )
   {
      QueueAASEvent(new ObjectCreatedEvent(dynamic_cast<IBase*>(this),rtid));
      return;
   }

   virtual ~SampleEncoder() {}

protected:
   struct encoder_data  *m_datap;
};

END_NAMESPACE(AAL)

#endif // __SAMPLEENCODEAFUSERVICE_H__
