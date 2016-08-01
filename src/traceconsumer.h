#ifndef _traceconsumer_h_
#define  _traceconsumer_h_

#include <evntcons.h>

struct ITraceConsumer {
    virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord) = 0;
    virtual bool ContinueProcessing() = 0;
};

#endif
