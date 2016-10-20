#ifndef __VISITING_WORKER_H__
#define __VISITING_WORKER_H__

class GTCOMMON_API IVisitingWorker
{
public:
   virtual void doContractWork(ServiceBase* const pWorkContainer) = 0;
};

class GTCOMMON_API IAcceptsVisitors
{
public:
   virtual void acceptVisitor(IVisitingWorker* const pVisitingWorker) = 0;
};

#endif
