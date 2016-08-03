#include <Objbase.h>
#include <node.h>
#include <uv.h>
#include "etwtrace.h"
#include "nodetraceconsumer.h"

using namespace v8;

struct Work
{
    uv_work_t request;
    TraceSession* session;
};

static void WorkAsync(uv_work_t* req)
{
    Work *work = static_cast<Work *>(req->data);
    work->session->Process();
}

static void WorkAsyncComplete(uv_work_t *req, int status)
{
    Work *work = static_cast<Work *>(req->data);
    delete work;
}

Persistent<Function> ETW::constructor;

ETW::ETW(const wchar_t* szSessionName)
{
    this->pTraceSession = new TraceSession(szSessionName);
}

ETW::~ETW()
{
    delete this->pTraceSession;
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
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->Start()));
}

void ETW::Stop(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->Stop()));
}

void ETW::Status(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Number::New(isolate, obj->pTraceSession->Status()));
}

void ETW::OpenTrace(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> cb = Local<Function>::Cast(args[0]);
    Persistent<Function, CopyablePersistentTraits<Function>> callback(isolate, cb);
    NodeTraceConsumer* consumer = new NodeTraceConsumer(callback, isolate);

    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->OpenTrace(consumer)));
}

void ETW::CloseTrace(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->CloseTrace()));
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
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->EnableProvider(nodeGuid, logLevel)));
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
    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->DisableProvider(nodeGuid)));
}

void ETW::Process(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    ETW* obj = ObjectWrap::Unwrap<ETW>(args.Holder());

    Work* work = new Work();
    work->request.data = work;
    work->session = obj->pTraceSession;

    //uv_queue_work(uv_default_loop(), &work->request, WorkAsync, WorkAsyncComplete);

    args.GetReturnValue().Set(Boolean::New(isolate, obj->pTraceSession->Process()));
    //args.GetReturnValue().Set(Boolean::New(isolate, true));
}

extern "C" {
  void init(Handle<Object> target) {
    ETW::Init(target);
  }
}

NODE_MODULE(etwtrace, init);
