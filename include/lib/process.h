#ifndef LIB_PROCESS_H_
#define LIB_PROCESS_H_

#include <lib/app.h>

namespace nfs
{

class Process : public GenericResource
{
public:
    Process();
    ~Process();
};

}

#endif

