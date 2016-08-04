#include <Objbase.h>
#include <node.h>
#include <uv.h>
#include "etwtrace.h"
#include "nodetraceconsumer.h"

using namespace v8;

Persistent<Function> ETW::constructor;

ETW::ETW(const wchar_t* szSessionName)
{
    this->m_pTraceSession = new TraceSession(szSessionName);
}

ETW::~ETW()
{
    delete this->m_pTraceSession;
}

void ETW::Init(Local<Object> exports)
{
    Isolate* isolate = Isolate::GetCurrent();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "ETW"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "start", Start);
    NODE_SET_PROTOTYPE_METHOD(tpl, "stop", Stop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "status", Status);
    NODE_SET_PROTOTYPE_METHOD(tpl, "openTrace", OpenTrace);
    NODE_SET_PROTOTYPE_METHOD(tpl, "closeTrace", CloseTrace);
    NODE_SET_PROTOTYPE_METHOD(tpl, "enableProvider", EnableProvider);
    NODE_SET_PROTOTYPE_METHOD(tpl, "disableProvider", DisableProvider);
    NODE_SET_PROTOTYPE_METHOD(tpl, "process", Process);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "ETW"), tpl->GetFunction());
}

void ETW::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) 
    {
        if (args[0]->IsUndefined())
        {
            Nan::ThrowTypeError("Session name is required.");
            return;
        }

        int wchars_num =  MultiByteToWideChar(CP_UTF8 , 0 , *String::Utf8Value(args[0]), -1, NULL , 0 );
        wchar_t* szSessionName = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, *String::Utf8Value(args[0]), -1, szSessionName, wchars_num);

        // Invoked as constructor: `new ETW(...)`
        ETW* obj = new ETW(szSessionName);
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } 
    else 
    {
        // Invoked as plain function `ETW(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void ETW::Start(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->Start()));
}

void ETW::Stop(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->Stop()));
}

void ETW::Status(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Number::New(isolate, obj->m_pTraceSession->Status()));
}

void ETW::OpenTrace(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> cb = Local<Function>::Cast(args[0]);
    Persistent<Function, CopyablePersistentTraits<Function>> callback(isolate, cb);
    NodeTraceConsumer* consumer = new NodeTraceConsumer(callback);

    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    obj->m_traceConsumers.push_back(consumer);
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->OpenTrace(consumer)));
}

void ETW::CloseTrace(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    bool closeTrace = obj->m_pTraceSession->CloseTrace();

    int ii;
    for(ii=0; ii < obj->m_traceConsumers.size(); ii++)
    {
        delete obj->m_traceConsumers[ii];
    }
    obj->m_traceConsumers.clear();

    args.GetReturnValue().Set(Boolean::New(isolate, closeTrace));
}

void ETW::EnableProvider(const FunctionCallbackInfo<Value>& args)
{
    GUID nodeGuid;
    int wchars_num =  MultiByteToWideChar(CP_UTF8 , 0 , *String::Utf8Value(args[0]), -1, NULL , 0 );
    wchar_t* szGuid = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, *String::Utf8Value(args[0]), -1, szGuid, wchars_num);
    ZeroMemory(&nodeGuid, sizeof(GUID));        
    if (IIDFromString(szGuid, &nodeGuid) != S_OK)        
    {
        delete[] szGuid;
        Nan::ThrowTypeError("First argument must be a valid GUID in the form of \"{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}\"");
        return;
    }
    
    delete[] szGuid;

    UCHAR logLevel = args[1]->Uint32Value();

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->EnableProvider(nodeGuid, logLevel)));
}

void ETW::DisableProvider(const FunctionCallbackInfo<Value>& args)
{
    GUID nodeGuid;
    int wchars_num =  MultiByteToWideChar( CP_UTF8 , 0 , *String::Utf8Value(args[0]), -1, NULL , 0 );
    wchar_t* szGuid = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, *String::Utf8Value(args[0]), -1, szGuid, wchars_num);
    ZeroMemory(&nodeGuid, sizeof(GUID));        
    if (IIDFromString(szGuid, &nodeGuid) != S_OK)        
    {        
        delete[] szGuid;
        Nan::ThrowTypeError("First argument must be a valid GUID in the form of \"{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}\"");
        return;
    }

    delete[] szGuid;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->DisableProvider(nodeGuid)));
}

void ETW::Process(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());

    // TODO: Find out how to unblock main thread
    args.GetReturnValue().Set(Boolean::New(isolate, obj->m_pTraceSession->Process()));
}

extern "C" {
  void init(Handle<Object> target) {
    ETW::Init(target);
  }
}

NODE_MODULE(etwtrace, init);
