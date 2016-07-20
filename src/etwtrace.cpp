#include "etwtrace.h"

using namespace v8;

Persistent<Function> ETW::constructor;

ETW::ETW(const wchar_t* szSessionName, const wchar_t* szFileName)
{
    this->pTraceSession = new TraceSession(szSessionName, szFileName);
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
    //NODE_SET_PROTOTYPE_METHOD(tpl, "plusOne", PlusOne);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "ETW"), tpl->GetFunction());
}

void ETW::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) 
    {
        // Invoked as constructor: `new MyObject(...)`
		LPCWSTR szSessionName = args[0]->IsUndefined() ? L"DefaultSession" : (LPCWSTR)*String::Utf8Value(args[0]);
		LPCWSTR szFileName = args[1]->IsUndefined() ? nullptr : (LPCWSTR)*String::Utf8Value(args[1]);
        ETW* obj = new ETW(szSessionName, szFileName);
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } 
    else 
    {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}


extern "C" {
  void init(Handle<Object> target) {
    ETW::Init(target);
  }
}

NODE_MODULE(etwtrace, init);
