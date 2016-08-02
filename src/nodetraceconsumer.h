#ifndef _nodetraceconsumer_h_
#define  _nodetraceconsumer_h_

#include <windows.h>
#include <evntprov.h>
#include <evntrace.h> 
#include <evntcons.h>
#include <node.h>
#include "traceconsumer.h"

using namespace v8;

struct NodeTraceConsumer : ITraceConsumer
{
    NodeTraceConsumer(Persistent<Function, CopyablePersistentTraits<Function>> cb) : mCb(cb) { }
    Persistent<Function, CopyablePersistentTraits<Function>> mCb;

    virtual bool ContinueProcessing() { return true; }
    virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord);
};

void NodeTraceConsumer::OnEventRecord(_In_ PEVENT_RECORD pEventRecord)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> f = Local<Function>::New(isolate, this->mCb);

    wchar_t* szProviderGuidW = new wchar_t[40];
    wchar_t* szActivityGuidW = new wchar_t[40];
    StringFromGUID2(pEventRecord->EventHeader.ProviderId, szProviderGuidW, 40);
    StringFromGUID2(pEventRecord->EventHeader.ActivityId, szActivityGuidW, 40);

    Local<Object> obj = Object::New(isolate);

    obj->Set(String::NewFromUtf8(isolate, "providerId"), String::NewFromTwoByte(isolate, (uint16_t*)szProviderGuidW));
    delete[] szProviderGuidW;

    obj->Set(String::NewFromUtf8(isolate, "activityId"), String::NewFromTwoByte(isolate, (uint16_t*)szActivityGuidW));
    delete[] szActivityGuidW;

    const unsigned argc = 1;
    Local<Value> argv[argc] = { obj };
    f->Call(isolate->GetCurrentContext()->Global(), argc, argv);  
}

#endif
