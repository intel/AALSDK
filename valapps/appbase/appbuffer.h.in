#ifndef APPBUFFER_H
#define APPBUFFER_H

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/CAALBase.h>

#include <iostream>
#include <string>
#include <list>

using namespace std;
using namespace AAL;

class appbuffer {
public:

    appbuffer(const string& bufferName,
              const btWSSize bufferSize);

    ~appbuffer();

private:
    string bufferName;
    btWSSize bufferSize;
    btVirtAddr bufferVirtAddress;
    btPhysAddr bufferPhysAddress;

    IALIBuffer *m_pALIBufferService; // Pointer to Buffer Service

};

#endif
