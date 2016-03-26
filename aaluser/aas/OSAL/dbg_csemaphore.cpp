// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_CSEMAPHORE
# include "dbg_csemaphore.h"

# define AutoLock0(__x)                                                                                               \
AutoLock(__x);                                                                                                        \
if ( NULL != m_UserDefined ) {                                                                                        \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnCreate(nInitialCount, nMaxCount); \
}

# define AutoLock1(__x)                                                                        \
AutoLock(__x);                                                                                 \
if ( NULL != m_UserDefined ) {                                                                 \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnDestroy(); \
}

# define AutoLock2(__x)                                                                            \
AutoLock(__x);                                                                                     \
if ( NULL != m_UserDefined ) {                                                                     \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnReset(nCount); \
}

# define AutoLock3(__x)                                                                                                \
AutoLock(__x);                                                                                                         \
if ( NULL != m_UserDefined ) {                                                                                         \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnCurrCounts(rcurrCount, rmaxCount); \
}

# define AutoLock4(__x)                                                                           \
AutoLock(__x);                                                                                    \
if ( NULL != m_UserDefined ) {                                                                    \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnPost(nCount); \
}

# define AutoLock5(__x)                                                                           \
AutoLock(__x);                                                                                    \
if ( NULL != m_UserDefined ) {                                                                    \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnUnblockAll(); \
}

# define AutoLock6(__x)                                                                     \
AutoLock(__x);                                                                              \
if ( NULL != m_UserDefined ) {                                                              \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnWait(); \
}

# define AutoLock7(__x)                                                                            \
AutoLock(__x);                                                                                     \
if ( NULL != m_UserDefined ) {                                                                     \
   reinterpret_cast< ::AAL::Testing::IAfterCSemaphoreAutoLock * >(m_UserDefined)->OnWait(Timeout); \
}

#endif // DBG_CSEMAPHORE
