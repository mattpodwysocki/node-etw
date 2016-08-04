#ifndef _etwtrace_h_
#define  _etwtrace_h_

bool g_ShouldRun = true;

#include <node.h>
#include <node_object_wrap.h>
#include <vector>

#include "tracesession.h"

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
    TraceSession* m_pTraceSession;
    std::vector<ITraceConsumer*> m_traceConsumers;
};

#endif
