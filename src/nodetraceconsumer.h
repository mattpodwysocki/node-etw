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

    LPWSTR szProviderGuidW = NULL;
    LPWSTR szActivityGuidW = NULL;
    StringFromCLSID(pEventRecord->EventHeader.ProviderId, &szProviderGuidW);
    StringFromCLSID(pEventRecord->EventHeader.ActivityId, &szActivityGuidW);

    Local<Object> obj = Object::New(isolate);

    char providerBuff[80];
    wcstombs(providerBuff, szProviderGuidW, wcslen(szProviderGuidW));
    obj->Set(String::NewFromUtf8(isolate, "providerId"), String::NewFromUtf8(isolate, providerBuff));
    CoTaskMemFree(szProviderGuidW);

    char activityBuff[80];
    wcstombs(activityBuff, szActivityGuidW, wcslen(szActivityGuidW));
    obj->Set(String::NewFromUtf8(isolate, "activityId"), String::NewFromUtf8(isolate, activityBuff));
    CoTaskMemFree(szActivityGuidW);

    const unsigned argc = 1;
    Local<Value> argv[argc] = { obj };
    f->Call(isolate->GetCurrentContext()->Global(), argc, argv);  
}

#endif
