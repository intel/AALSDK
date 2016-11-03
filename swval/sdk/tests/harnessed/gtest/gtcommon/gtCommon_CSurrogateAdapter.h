// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __SURROGATE_ADAPTER_H__
#define __SURROGATE_ADAPTER_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif   // HAVE_CONFIG_H
#include "gtCommon.h"

/// ===================================================================
/// @brief        The visitor producer / worker interface.
///
/// @details      Workers must implement doContractdWork

class GTCOMMON_API IVisitingWorker
{
public:
   virtual ~IVisitingWorker()
   {
   }
   virtual void doContractWork( ServiceBase* const pWorkContainer ) = 0;
};

/// ===================================================================
/// @brief        The visitor consumer / receiver interface.
///
class GTCOMMON_API IAcceptsVisitors
{
public:
   virtual ~IAcceptsVisitors()
   {
   }
   virtual void acceptVisitor( IVisitingWorker* const pVisitingWorker ) = 0;
};

// forward declaration
//
class GTCOMMON_API CAASBuilder;

/// ===================================================================
/// @brief        The builder consumer interface.
///
/// @details      The current implementation takes a CAASBuilder in
///               order to make use of the built-in interface map
///               inherited from CAASBase.
///
class GTCOMMON_API IAcceptsBuilders
{
public:
   virtual ~IAcceptsBuilders()
   {
   }

   virtual void acceptBuilder( CAASBuilder const* )
   {
   }
};

/// ===================================================================
/// @brief        The primary builder class.
///
/// @details      Ordinarily, an interface might be used with some sort
///               of getValues function, but in this case, the
///               SetInterface and Interface inherited functions are
///               used.
///
class GTCOMMON_API CAASBuilder : public CAASBase,
                                 public EmptyIAALTransport,
                                 public EmptyIAALMarshaller,
                                 public EmptyIAALUnMarshaller
{

public:
   CAASBuilder()
   {
   }

   /// ================================================================
   /// @brief        The helper function, exposing the CAASBase
   ///               interface map.
   ///
   /// @details      Use this function to set interfaces on self.
   ///               Avoids use of this pointer in the constructor.
   ///
protected:
   void SetInterfaces( CAASBuilder const* pObj )
   {
      SetInterface( iidAALTransport, (IAALTransport*)pObj );
      SetInterface( iidAALMarshaller, (IAALMarshaller*)pObj );
      SetInterface( iidAALUnMarshaller, (IAALUnMarshaller*)pObj );
   }

public:
   virtual ~CAASBuilder()
   {
   }
};

/// ===================================================================
/// @brief        The main surrogate adapter.
///
/// @details      Combines the builder and visitor into the same object.
///
class GTCOMMON_API CSurrogateAdapter : public IVisitingWorker,
                                       public IAcceptsBuilders,
                                       public CAASBuilder
{
public:
   virtual ~CSurrogateAdapter()
   {
   }

   virtual void acceptBuilder( CAASBuilder const* pBuilder )
   {
      SetInterfaces( pBuilder );
   }

   // This should be overridden in the derived classes that implement
   // test code.  These objects will be provided to the fixture as type
   // parameters.
   //
   // @param        pWorkContainer    The service object, provided to
   //                                 test functionality in the base
   //                                 class.
   //
   virtual void doContractWork( ServiceBase* const pWorkContainer )
   {
      MSG( "doing work from base class" );
   }
};

#endif   // __SURROGATE_ADAPTER_H__
