// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_THREADGROUP
# include "dbg_threadgroup.h"

# define AutoLock0(__x)                                                                          \
AutoLock(__x);                                                                                   \
if ( NULL != m_UserDefined ) {                                                                   \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnAdd(pDisp); \
}

# define AutoLock1(__x)                                                                       \
AutoLock(__x);                                                                                \
if ( NULL != m_UserDefined ) {                                                                \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnStart(); \
}

# define AutoLock2(__x)                                                                      \
AutoLock(__x);                                                                               \
if ( NULL != m_UserDefined ) {                                                               \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnStop(); \
}

# define AutoLock3(__x)                                                                       \
AutoLock(__x);                                                                                \
if ( NULL != m_UserDefined ) {                                                                \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnDrain(); \
}

# define AutoLock4(__x)                                                                             \
AutoLock(__x);                                                                                      \
if ( NULL != m_UserDefined ) {                                                                      \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnJoin(Timeout); \
}

# define AutoLock5(__x)                                                                                \
AutoLock(__x);                                                                                         \
if ( NULL != m_UserDefined ) {                                                                         \
   reinterpret_cast< ::AAL::Testing::IAfterThreadGroupAutoLock * >(m_UserDefined)->OnDestroy(Timeout); \
}

#endif // DBG_THREADGROUP
