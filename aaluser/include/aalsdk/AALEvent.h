// Copyright(c) 2005-2016, Intel Corporation
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
/// @file AALEvent.h
/// @brief Definition of IEvent interface.
/// @ingroup Events
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/20/2005     JG       Initial version started
/// 03/19/2007     JG       Linux Port
/// 04/24/2007     JG       Modified event definition to take
///                         IBase rather than I_LRBase
/// 05/28/2007     JG       Added cached interfaces (SubClass)
/// 06/08/2007     JG       Converted to btID keys
/// 06/28/2007     JG       Converted to use NamedValueSets
/// 07/15/2007     JG       Changed reason and error to btID
/// 02/25/2008     HM       Made Parms be NamedValueSet instead of INamedValueSet
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 07/10/2012     TW       Updated definitions to new format
/// 07/15/2012     HM       Added #defines for backward compatibility
/// 10/06/2015     JG       Removed ObjectxyzEvents@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALEVENT_H__
#define __AALSDK_AALEVENT_H__
#include <aalsdk/AALDefs.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>

BEGIN_NAMESPACE(AAL)

#ifdef __cplusplus

class IBase;

//=============================================================================
// Typedefs and Constants
//=============================================================================

/// @addtogroup Events
/// @{

/// Base interface for all Event interfaces.
class AASLIB_API IEvent
{
public:
   /// IEvent Destructor.
   virtual ~IEvent() {}
   /// Query interface for a given interface id.
   /// @param[in] Interface The Interface ID to query.
   /// @returns An Interface.
   virtual btGenericInterface    Interface(btIID Interface)      const = 0;
   /// Determine whether this event contains the specified interface.
   /// @param[in] Interface The Interface ID to find.
   /// @retval True if the Event has the Interface specified.
   /// @retval False if the Event does not have the Interface specified.
   virtual btBool                      Has(btIID Interface)      const = 0;
   /// Retrieve the native subclass id.
   /// @returns An Interface ID.
   virtual btIID                SubClassID()                     const = 0;
   /// IEvent inequality.
   /// @param[in] rother A reference to an IEvent to compare.
   /// @retval True if the two events are not equal.
   /// @retval False if the two events are equal.
   virtual btBool             operator != (const IEvent &rother) const = 0;
   /// IEvent equality.
   /// @param[in] rother A reference to an IEvent to compare.
   /// @retval True if the two events are equal.
   /// @retval  if the two events are not equal.
   virtual btBool             operator == (const IEvent &rother) const = 0;
   /// Conversion to IBase &.
   /// @returns A reference to an IBase Interface.
   virtual IBase &                  Object()                     const = 0;
   /// Conversion to IBase *.
   /// @returns A pointer to an IBase Interface.
   virtual IBase *                 pObject()                     const = 0;
   /// Internal state check.
   /// @retval True if the internal state of the Event is OK.
   /// @retval False if the internal state of the Event is not OK.
   virtual btBool                     IsOK()                     const = 0;
   /// Clone an event.
   /// @return An identical copy of this event object.
   virtual IEvent *                  Clone()                     const = 0;
protected:
   virtual btBool       ProcessEventTranID()                           = 0;
};

/// Base interface for all Transaction Event interfaces.
class AASLIB_API ITransactionEvent
{
public:
   // ITransactionEvent Destructor.
   virtual ~ITransactionEvent() {}
   /// Retrieve the TransactionID associated with this Transaction Event.
   /// @return The transaction ID.
   virtual TransactionID TranID() const                = 0;
   /// Set the TransactionID for this Transaction Event.
   /// @param[in] TranID A reference to the TransactionID to set into the Event.
   /// @return void
   virtual void SetTranID(TransactionID const &TranID) = 0;
};

/// Base interface for all Exception Event interfaces.
class AASLIB_API IExceptionEvent
{
public:
   // IExceptionEvent Destructor.
   virtual ~IExceptionEvent() {}
   /// Retrieve the Exception id.
   /// @returns The Exception id.
   virtual btID ExceptionNumber() const = 0;
   /// Retrieve the Exception reason code.
   /// @returns The Exception reason code.
   virtual btID          Reason() const = 0;
   /// Retrieve a string description of the Exception.
   /// @returns The description of the Exception.
   virtual btString Description() const = 0;
};

/// Base interface for all Exception Transaction Event interfaces.
class AASLIB_API IExceptionTransactionEvent : public ITransactionEvent
{
public:
   /// Retrieve the Exception id.
   /// @returns The Exception id.
   virtual btID ExceptionNumber() const = 0;
   /// Retrieve the Exception reason code.
   /// @returns The Exception reason code.
   virtual btID          Reason() const = 0;
   /// Retrieve a string description of the Exception.
   /// @returns The description of the Exception.
   virtual btString Description() const = 0;
};


//=============================================================================
// Application defined event interfaces
//=============================================================================

/// Base interface for Application-specific Events.
class AASLIB_API IApplicationEvent
{
public:
   // IApplicationEvent Destructor.
   virtual ~IApplicationEvent() {}
   /// Retrieve the parameters associated with this Application-specific event.
   /// @returns A reference to a NamedValueSet containing the parameters.
   virtual NamedValueSet & Parms() = 0;
};

/// Base interface for Application-specific Exception Events.
class AASLIB_API IApplicationExceptionEvent
{
public:
   // IApplicationExceptionEvent Destructor.
   virtual ~IApplicationExceptionEvent() {}
   /// Retrieve the Exception id.
   /// @returns The Exception ID.
   virtual btID ExceptionNumber() const = 0;
   /// Retrieve the Exception reason code.
   /// @returns The Exception ID.
   virtual btID          Reason() const = 0;
   /// Retrieve a string description of the Exception.
   /// @returns The description in a string.
   virtual btString Description() const = 0;
   /// Retrieve the parameters associated with this Application-specific Exception Event.
   /// @returns A reference to a NamedValueSet containing the parameters.
   virtual NamedValueSet & Parm()       = 0;
};


//=============================================================================
// Resource Release Request defined event interfaces
//=============================================================================

/// Base interface for Resource Release Request-specific Events.
class AASLIB_API IReleaseRequestEvent
{
public:
   // IReleaseRequestEvent Destructor.
   virtual ~IReleaseRequestEvent() {};

   /// AFU Release reason types
   enum  ReleaseReason_e {
       resource_revokeing
   };

   /// Retrieve AFU Release timeout in milliseconds .
   /// @returns The AFU resource release timeout.
   virtual btUnsigned64bitInt Timeout() const= 0;

   /// Retrieve Resource Release  Exception reason code.
   /// @returns The reason code.
   virtual ReleaseReason_e Reason()     const= 0;

   /// Retrieve a string description of the Resource Release Request event.
   /// @returns The event description.
   virtual btString Description()       const = 0;
};


/// Print information in Exception and ExceptionTransaction Events.
AASLIB_API void PrintExceptionDescription(IEvent const &theEvent);

#endif //__cplusplus

/// @}


END_NAMESPACE(AAL)

#endif // __AALSDK_AALEVENT_H__

