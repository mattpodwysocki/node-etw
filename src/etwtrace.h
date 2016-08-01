#ifndef _etwtrace_h_
#define  _etwtrace_h_

#include <node.h>
#include <node_object_wrap.h>
#include <exception>
#include <string>
#include "tracesession.h"
#include "traceeventinfo.h"

using namespace v8;

class ETW : public node::ObjectWrap 
{
  public:
    static void Init(Local<Object> exports);

  private:
    explicit ETW(const wchar_t* szSession);
    ~ETW();

    static void New(const FunctionCallbackInfo<Value>& args);
    static void Start(const FunctionCallbackInfo<Value>& args);
    static void Stop(const FunctionCallbackInfo<Value>& args);
    static void OpenTrace(const FunctionCallbackInfo<Value>& args);
    static void CloseTrace(const FunctionCallbackInfo<Value>& args);
    static void EnableProvider(const FunctionCallbackInfo<Value>& args);
    static void DisableProvider(const FunctionCallbackInfo<Value>& args);
    static void Process(const FunctionCallbackInfo<Value>& args);
    static void Status(const FunctionCallbackInfo<Value>& args); 
  
    static v8::Persistent<Function> constructor;
    TraceSession* pTraceSession;
};

/*
ZeroMemory(&nodeGuid, sizeof(GUID));        
if (IIDFromString(L"{77754E9B-264B-4D8D-B981-E4135C1ECB0C}", &nodeGuid) != S_OK)        
{        
  wprintf(L"Failed to get GUID from string");        
  return 1;        
}
*/

#endif
