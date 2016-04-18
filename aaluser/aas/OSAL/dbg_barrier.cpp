// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_BARRIER
# include "dbg_barrier.h"

# define AutoLock0(__x)                                                                                           \
AutoLock(__x);                                                                                                    \
if ( NULL != m_UserDefined ) {                                                                                    \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnCreate(UnlockCount, bAutoReset); \
}

# define AutoLock1(__x)                                                                     \
AutoLock(__x);                                                                              \
if ( NULL != m_UserDefined ) {                                                              \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnDestroy(); \
}

# define AutoLock2(__x)                                                                              \
AutoLock(__x);                                                                                       \
if ( NULL != m_UserDefined ) {                                                                       \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnReset(UnlockCount); \
}

# define AutoLock3(__x)                                                                                               \
AutoLock(__x);                                                                                                        \
if ( NULL != m_UserDefined ) {                                                                                        \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnCurrCounts(rCurCount, rUnlockCount); \
}

# define AutoLock4(__x)                                                                        \
AutoLock(__x);                                                                                 \
if ( NULL != m_UserDefined ) {                                                                 \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnPost(nCount); \
}

# define AutoLock5(__x)                                                                        \
AutoLock(__x);                                                                                 \
if ( NULL != m_UserDefined ) {                                                                 \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnUnblockAll(); \
}

# define AutoLock6(__x)                                                                  \
AutoLock(__x);                                                                           \
if ( NULL != m_UserDefined ) {                                                           \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnWait(); \
}

# define AutoLock7(__x)                                                                         \
AutoLock(__x);                                                                                  \
if ( NULL != m_UserDefined ) {                                                                  \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnWait(Timeout); \
}

# if   defined( __AAL_LINUX__ )

#    define _PThreadCondWait0                                                             \
if (NULL != m_UserDefined) {                                                              \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnSleep(); \
}                                                                                         \
_PThreadCondWait

#    define _PThreadCondTimedWait1                                                               \
if (NULL != m_UserDefined) {                                                                     \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnSleep(Timeout); \
}                                                                                                \
_PThreadCondTimedWait

# elif defined( __AAL_WINDOWS__ )

#    define _UnlockedWaitForSingleObject0                                                 \
if ( NULL != m_UserDefined ) {                                                            \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnSleep(); \
}                                                                                         \
_UnlockedWaitForSingleObject

#    define _UnlockedWaitForSingleObject1                                                        \
if ( NULL != m_UserDefined ) {                                                                   \
   reinterpret_cast< ::AAL::Testing::IAfterBarrierAutoLock * >(m_UserDefined)->OnSleep(Timeout); \
}                                                                                                \
_UnlockedWaitForSingleObject

# endif // OS

#endif // DBG_BARRIER
