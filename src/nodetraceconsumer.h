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
    NodeTraceConsumer(Local<Function> cb) : mCb(cb) { }
    Local<Function> mCb;

    virtual bool ContinueProcessing() { return true; }
    virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord);
};

void NodeTraceConsumer::OnEventRecord(_In_ PEVENT_RECORD pEventRecord)
{
    wprintf(L"ProcessID %d\n", pEventRecord->EventHeader.ProcessId);
    
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    LPWSTR szProviderGuidW = NULL;
    StringFromCLSID(pEventRecord->EventHeader.ProviderId, &szProviderGuidW);

    Local<Object> obj = Object::New(isolate);
    char cbuff[80];
    wcstombs(cbuff, szProviderGuidW, wcslen(szProviderGuidW));
    obj->Set(String::NewFromUtf8(isolate, "providerId"), String::NewFromUtf8(isolate, cbuff));
    CoTaskMemFree(szProviderGuidW);

    const unsigned argc = 1;
    Local<Value> argv[argc] = { obj };
    mCb->Call(isolate->GetCurrentContext()->Global(), argc, argv);  
}

#endif
