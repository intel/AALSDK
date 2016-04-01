// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

////////////////////////////////////////////////////////////////////////////////

void Set(AAL::TransactionID  &tid,
         btApplicationContext ctx,
         btEventHandler       h,
         IBase               *pBase,
         btBool               filter,
         btID                 id)
{
   tid.Context(ctx);
   tid.Handler(h);
   tid.Ibase(pBase);
   tid.Filter(filter);
   tid.ID(id);
}

void Set(stTransactionID_t   &stid,
         btApplicationContext ctx,
         btEventHandler       h,
         IBase               *pBase,
         btBool               filter,
         btID                 id)
{
   stid.m_Context = ctx;
   stid.m_Handler = h;
   stid.m_IBase   = pBase;
   stid.m_Filter  = filter;
   stid.m_intID   = id;
}

void Check(const AAL::TransactionID &tid,
           btApplicationContext      ctx,
           btEventHandler            h,
           IBase const              *pBase,
           btBool                    filter)
{
   EXPECT_EQ(ctx,    tid.Context());
   EXPECT_EQ(h,      tid.Handler());
   EXPECT_EQ(pBase,  tid.Ibase());
   EXPECT_EQ(filter, tid.Filter());
}

void Check(const AAL::TransactionID &tid,
           btApplicationContext      ctx,
           btEventHandler            h,
           IBase const              *pBase,
           btBool                    filter,
           btID                      id)
{
   EXPECT_EQ(ctx,    tid.Context());
   EXPECT_EQ(h,      tid.Handler());
   EXPECT_EQ(pBase,  tid.Ibase());
   EXPECT_EQ(filter, tid.Filter());
   EXPECT_EQ(id,     tid.ID());
}

void Check(stTransactionID_t const &stid,
           btApplicationContext     ctx,
           btEventHandler           h,
           IBase const             *pBase,
           btBool                   filter,
           btID                     id)
{
   EXPECT_EQ(ctx,    stid.m_Context);
   EXPECT_EQ(h,      stid.m_Handler);
   EXPECT_EQ(pBase,  stid.m_IBase);
   EXPECT_EQ(filter, stid.m_Filter);
   EXPECT_EQ(id,     stid.m_intID);
}

////////////////////////////////////////////////////////////////////////////////

TEST(TransactionIDTest, aal0762)
{
   // TransactionID::TransactionID() creates a TransactionID with a NULL btApplicationContext,
   // a NULL btEventHandler, a NULL IBase *, and a set filter flag.

   TransactionID tid;
   Check(tid, NULL, NULL, NULL, true);
}

TEST(TransactionIDTest, aal0763)
{
   // TransactionID::TransactionID(const stTransactionID_t & ) creates a TransactionID whose fields
   // match those contained in the struct.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   stTransactionID_t stid;
   stid.m_Context = ctx;
   stid.m_Handler = h;
   stid.m_IBase   = const_cast<IBase *>(pBase);
   stid.m_Filter  = filt;
   stid.m_intID   = id;

   TransactionID tid(stid);
   Check(tid, ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0764)
{
   // The TransactionID copy c'tor creates an exact copy of the given object.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   TransactionID tid0;
   Set(tid0, ctx, h, const_cast<IBase *>(pBase), filt, id);

   TransactionID tid1(tid0);
   Check(tid1, ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0765)
{
   // TransactionID::TransactionID(btApplicationContext ) creates a TransactionID with the given
   // btApplicationContext, a NULL btEventHandler, a NULL IBase *, and a set filter flag.

   const btApplicationContext ctx = (btApplicationContext)3;

   TransactionID tid(ctx);
   Check(tid, ctx, NULL, NULL, true);
}

TEST(TransactionIDTest, aal0766)
{
   // TransactionID::TransactionID(btID ) creates a TransactionID with a NULL
   // btApplicationContext, a NULL btEventHandler, a NULL IBase *, a set filter flag, and the
   // given integer ID.

   const btID id = 6;

   TransactionID tid(id);
   Check(tid, NULL, NULL, NULL, true, id);
}

TEST(TransactionIDTest, aal0767)
{
   // TransactionID::TransactionID(btID , btApplicationContext ) creates a TransactionID
   // with the given btApplicationContext, a NULL btEventHandler, a NULL IBase *, a set filter flag,
   // and the given integer ID.

   const btApplicationContext ctx = (btApplicationContext)3;
   const btID                 id  = 6;

   TransactionID tid(id, ctx);
   Check(tid, ctx, NULL, NULL, true, id);
}

TEST(TransactionIDTest, aal0768)
{
   // TransactionID::TransactionID(btApplicationContext , btEventHandler, btBool ) creates a
   // TransactionID with the given btApplicationContext, the given btEventHandler, a NULL IBase *,
   // and the given filter flag.

   const btApplicationContext ctx  = (btApplicationContext)3;
   const btEventHandler       h    = (btEventHandler)4;
   const btBool               filt = true;

   TransactionID tid(ctx, h, filt);
   Check(tid, ctx, h, NULL, filt);
}

TEST(TransactionIDTest, aal0769)
{
   // TransactionID::TransactionID(btID , btEventHandler, btBool ) creates a
   // TransactionID with a NULL btApplicationContext, the given btEventHandler, a NULL IBase *,
   // the given filter flag, and the given integer ID.

   const btEventHandler h    = (btEventHandler)4;
   const btBool         filt = true;
   const btID           id   = 6;

   TransactionID tid(id, h, filt);
   Check(tid, NULL, h, NULL, filt, id);
}

TEST(TransactionIDTest, aal0770)
{
   // TransactionID::TransactionID(btID , btApplicationContext , btEventHandler, btBool )
   // creates a TransactionID with the given btApplicationContext, the given btEventHandler, a NULL IBase *,
   // the given filter flag, and the given integer ID.

   const btApplicationContext ctx  = (btApplicationContext)3;
   const btEventHandler       h    = (btEventHandler)4;
   const btBool               filt = true;
   const btID                 id   = 6;

   TransactionID tid(id, ctx, h, filt);
   Check(tid, ctx, h, NULL, filt, id);
}

TEST(TransactionIDTest, aal0771)
{
   // TransactionID::TransactionID(btEventHandler, btBool )
   // creates a TransactionID with a NULL btApplicationContext, the given btEventHandler, a NULL IBase *,
   // and the given filter flag.

   const btEventHandler h    = (btEventHandler)4;
   const btBool         filt = true;

   TransactionID tid(h, filt);
   Check(tid, NULL, h, NULL, filt);
}

TEST(TransactionIDTest, aal0772)
{
   // TransactionID::TransactionID(btApplicationContext , IBase * , btBool )
   // creates a TransactionID with the given btApplicationContext, a NULL btEventHandler,
   // the given IBase *, and the given filter flag.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;

   TransactionID tid(ctx, const_cast<IBase *>(pBase), filt);
   Check(tid, ctx, NULL, pBase, filt);
}

TEST(TransactionIDTest, aal0773)
{
   // TransactionID::TransactionID(btID , IBase * , btBool )
   // creates a TransactionID with a NULL btApplicationContext, a NULL btEventHandler,
   // the given IBase *, the given filter flag, and the given integer ID.

   const IBase *pBase = (IBase *)5;
   const btID   id    = 6;
   const btBool filt  = true;

   TransactionID tid(id, const_cast<IBase *>(pBase), filt);
   Check(tid, NULL, NULL, pBase, filt, id);
}

TEST(TransactionIDTest, aal0774)
{
   // TransactionID::TransactionID(btID , btApplicationContext , IBase * , btBool )
   // creates a TransactionID with the given btApplicationContext, a NULL btEventHandler,
   // the given IBase *, the given filter flag, and the given integer ID.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const IBase               *pBase = (IBase *)5;
   const btID                 id    = 6;
   const btBool               filt  = true;

   TransactionID tid(id, ctx, const_cast<IBase *>(pBase), filt);
   Check(tid, ctx, NULL, pBase, filt, id);
}

TEST(TransactionIDTest, aal0775)
{
   // TransactionID::TransactionID(IBase * , btBool )
   // creates a TransactionID with a NULL btApplicationContext, a NULL btEventHandler,
   // the given IBase *, and the given filter flag.

   const IBase *pBase = (IBase *)5;
   const btBool filt  = true;

   TransactionID tid(const_cast<IBase *>(pBase), filt);
   Check(tid, NULL, NULL, pBase, filt);
}

TEST(TransactionIDTest, aal0776)
{
   // TransactionID::operator = (TransactionID const & ) assigns this TransactionID object to
   // the values found in the argument TransactionID.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   TransactionID tid0;
   Set(tid0, ctx, h, const_cast<IBase *>(pBase), filt, id);

   TransactionID tid1;

   tid1 = tid0;

   Check(tid1, ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0777)
{
   // TransactionID::operator = (stTransactionID_t const & ) assigns this TransactionID object to
   // the values found in the argument struct.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   stTransactionID_t stid;
   stid.m_Context = ctx;
   stid.m_Handler = h;
   stid.m_IBase   = const_cast<IBase *>(pBase);
   stid.m_Filter  = filt;
   stid.m_intID   = id;

   TransactionID tid1;

   tid1 = stid;

   Check(tid1, ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0778)
{
   // TransactionID::operator stTransactionID_t const & converts the TransactionID object to
   // a const struct reference.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   TransactionID tid;
   Set(tid, ctx, h, const_cast<IBase *>(pBase), filt, id);

   Check(tid.operator const AAL::stTransactionID_t &(), ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0779)
{
   // TransactionID::operator stTransactionID_t & converts the TransactionID object to
   // a struct reference.

   const btApplicationContext ctx   = (btApplicationContext)3;
   const btEventHandler       h     = (btEventHandler)4;
   const IBase               *pBase = (IBase *)5;
   const btBool               filt  = true;
   const btID                 id    = 6;

   TransactionID tid;
   Set(tid.operator AAL::stTransactionID_t &(), ctx, h, const_cast<IBase *>(pBase), filt, id);

   Check(tid, ctx, h, pBase, filt, id);
}

TEST(TransactionIDTest, aal0780)
{
   // When the IBase * in one of the two TransactionID objects being considered in
   // TransactionID::operator == () is NULL, the function returns false.

   CAASBase base;

   TransactionID lhs0;
   Set(lhs0, NULL, NULL, NULL, true, 3);

   TransactionID rhs0;
   Set(rhs0, NULL, NULL, &base, true, 4);

   EXPECT_FALSE(lhs0 == rhs0);


   TransactionID lhs1;
   Set(lhs1, NULL, NULL, &base, true, 3);

   TransactionID rhs1;
   Set(rhs1, NULL, NULL, NULL, true, 4);

   EXPECT_FALSE(lhs1 == rhs1);


   TransactionID lhs2;
   TransactionID rhs2;

   EXPECT_FALSE(lhs2 == rhs2);
   rhs2.ID(lhs2.ID());
   EXPECT_TRUE(lhs2 == rhs2);


   TransactionID lhs3;
   Set(lhs3, NULL, NULL, &base, true, 3);

   TransactionID rhs3;
   Set(rhs3, NULL, NULL, &base, true, 3);

   EXPECT_TRUE(lhs3 == rhs3);
}

TEST(TransactionIDTest, aal0781)
{
   // When comparing TransactionID objects with TransactionID::operator == (), if both
   // TransactionID's have non-NULL IBase *'s, then IBase::operator != () is used to compare the
   // two IBase *'s. (When IBase::operator != () returns true, TransactionID::operator == () returns
   // false.)

   class aal0781Base : public CAASBase
   {
   public:
      aal0781Base() {}
      void Add(btIID id, btGenericInterface ifc) { SetInterface(id, ifc); }
   };

   aal0781Base lhsbase;
   CAASBase    rhsbase;

   TransactionID lhs;
   Set(lhs, NULL, NULL, &lhsbase, true, 3);

   TransactionID rhs;
   Set(rhs, NULL, NULL, &rhsbase, true, 3);

   EXPECT_TRUE(lhs == rhs);


   lhsbase.Add((btIID)789, (btGenericInterface)29);
   EXPECT_FALSE(lhs == rhs);
}

TEST(TransactionIDTest, aal0782)
{
   // When comparing TransactionID objects with TransactionID::operator == (), if both
   // TransactionID's have non-NULL IBase *'s, then IBase::operator != () is used to compare the
   // two IBase *'s. When IBase::operator != () returns false, the btApplicationContext,
   // btEventHandler, btID, and btBool fields in the two objects are bitwise compared to
   // make the final determination of equality.

   CAASBase lhsbase;
   CAASBase rhsbase;

   TransactionID lhs;
   Set(lhs, NULL, NULL, &lhsbase, true, 3);

   TransactionID rhs;
   Set(rhs, NULL, NULL, &rhsbase, true, 3);

   EXPECT_TRUE(lhs == rhs);


   Set(lhs, (btApplicationContext)3, NULL, &lhsbase, true, 3);
   EXPECT_FALSE(lhs == rhs);
   Set(rhs, (btApplicationContext)3, NULL, &rhsbase, true, 3);
   EXPECT_TRUE(lhs == rhs);

   Set(lhs, (btApplicationContext)3, (btEventHandler)4, &lhsbase, true, 3);
   EXPECT_FALSE(lhs == rhs);
   Set(rhs, (btApplicationContext)3, (btEventHandler)4, &rhsbase, true, 3);
   EXPECT_TRUE(lhs == rhs);

   Set(lhs, (btApplicationContext)3, (btEventHandler)4, &lhsbase, false, 3);
   EXPECT_FALSE(lhs == rhs);
   Set(rhs, (btApplicationContext)3, (btEventHandler)4, &rhsbase, false, 3);
   EXPECT_TRUE(lhs == rhs);

   Set(lhs, (btApplicationContext)3, (btEventHandler)4, &lhsbase, false, 4);
   EXPECT_FALSE(lhs == rhs);
   Set(rhs, (btApplicationContext)3, (btEventHandler)4, &rhsbase, false, 4);
   EXPECT_TRUE(lhs == rhs);
}

