#ifndef __BUILDER_H__
#define __BUILDER_H__

class GTCOMMON_API IServiceBuilder
{
public:
   virtual INamedValueSet const* getValues() = 0;
};

class GTCOMMON_API IAcceptsBuilders
{
public:
   virtual void acceptBuilder(IServiceBuilder* pBuilder) = 0;
};

#endif // __BUILDER_H__
