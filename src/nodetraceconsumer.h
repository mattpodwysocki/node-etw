#ifndef _nodetraceconsumer_h_
#define  _nodetraceconsumer_h_

UCHAR g_PointerSize;

#include <windows.h>
#include <evntprov.h>
#include <evntrace.h> 
#include <evntcons.h>
#include <node.h>
#include "traceconsumer.h"

using namespace v8;

DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO & pInfo)
{
	DWORD status = ERROR_SUCCESS;
	DWORD BufferSize = 0;

	status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);

	if (ERROR_INSUFFICIENT_BUFFER == status)
	{
		pInfo = (TRACE_EVENT_INFO*)malloc(BufferSize);
		if (pInfo == NULL)
		{
			return ERROR_OUTOFMEMORY;
		}

		status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);
	}

	return status;
}

struct NodeTraceConsumer : ITraceConsumer
{
    NodeTraceConsumer(Persistent<Function, CopyablePersistentTraits<Function>> cb, Isolate* isolate) : mCb(cb), mIsolate(isolate) { }
    Persistent<Function, CopyablePersistentTraits<Function>> mCb;
    Isolate* mIsolate;

    virtual bool ContinueProcessing() { return g_ShouldRun; }
    virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord);
};

void NodeTraceConsumer::OnEventRecord(_In_ PEVENT_RECORD pEventRecord)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Object> obj = Object::New(isolate);

    wchar_t* szProviderGuidW = new wchar_t[40];
    wchar_t* szActivityGuidW = new wchar_t[40];
    StringFromGUID2(pEventRecord->EventHeader.ProviderId, szProviderGuidW, 40);
    StringFromGUID2(pEventRecord->EventHeader.ActivityId, szActivityGuidW, 40);

    obj->Set(String::NewFromUtf8(isolate, "providerId"), String::NewFromTwoByte(isolate, (uint16_t*)szProviderGuidW));
    delete[] szProviderGuidW;

    obj->Set(String::NewFromUtf8(isolate, "activityId"), String::NewFromTwoByte(isolate, (uint16_t*)szActivityGuidW));
    delete[] szActivityGuidW;

    obj->Set(String::NewFromUtf8(isolate, "processId"), Integer::NewFromUnsigned(isolate, pEventRecord->EventHeader.ProcessId));

    ULONGLONG TimeStamp = 0;
    ULONGLONG Nanoseconds = 0;
    SYSTEMTIME st;
    SYSTEMTIME stLocal;
    FILETIME ft;

    ft.dwHighDateTime = pEventRecord->EventHeader.TimeStamp.HighPart;
    ft.dwLowDateTime = pEventRecord->EventHeader.TimeStamp.LowPart;

	if (FileTimeToSystemTime(&ft, &st) && SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal))
	{
		TimeStamp = pEventRecord->EventHeader.TimeStamp.QuadPart;
		Nanoseconds = (TimeStamp % 10000000) * 100;

		Local<Object> timeObj = Object::New(isolate);
		timeObj->Set(String::NewFromUtf8(isolate, "year"), Integer::New(isolate, stLocal.wYear));
		timeObj->Set(String::NewFromUtf8(isolate, "month"), Integer::New(isolate, stLocal.wMonth));
		timeObj->Set(String::NewFromUtf8(isolate, "day"), Integer::New(isolate, stLocal.wDay));
		timeObj->Set(String::NewFromUtf8(isolate, "hour"), Integer::New(isolate, stLocal.wHour));
		timeObj->Set(String::NewFromUtf8(isolate, "minute"), Integer::New(isolate, stLocal.wMinute));
		timeObj->Set(String::NewFromUtf8(isolate, "second"), Integer::New(isolate, stLocal.wSecond));
		timeObj->Set(String::NewFromUtf8(isolate, "milliseconds"), Integer::New(isolate, stLocal.wMilliseconds));
		timeObj->Set(String::NewFromUtf8(isolate, "nanoseconds"), Integer::New(isolate, Nanoseconds));

		obj->Set(String::NewFromUtf8(isolate, "timestamp"), timeObj);
	}
	else
	{
		obj->Set(String::NewFromUtf8(isolate, "timestamp"), Undefined(isolate));
	}

	PTRACE_EVENT_INFO pInfo = NULL;
	DWORD status = ERROR_SUCCESS;

	status = GetEventInformation(pEventRecord, pInfo);
	if (status != ERROR_SUCCESS)
	{
		Nan::ErrnoException(status);
		return;
	}

	if (DecodingSourceWbem == pInfo->DecodingSource)
	{
		wchar_t* szEventId = new wchar_t[40];
		StringFromGUID2(pInfo->EventGuid, szEventId, 40);
		obj->Set(String::NewFromUtf8(isolate, "eventGuid"), String::NewFromTwoByte(isolate, (uint16_t*)szEventId));
		delete[] szEventId;

		obj->Set(String::NewFromUtf8(isolate, "eventVersion"), Integer::NewFromUnsigned(isolate, pEventRecord->EventHeader.EventDescriptor.Version));
		obj->Set(String::NewFromUtf8(isolate, "opCode"), Integer::NewFromUnsigned(isolate, pEventRecord->EventHeader.EventDescriptor.Opcode));
	}
	else if (DecodingSourceXMLFile == pInfo->DecodingSource)
	{
		obj->Set(String::NewFromUtf8(isolate, "eventId"), Integer::NewFromUnsigned(isolate, pInfo->EventDescriptor.Id));
	}

	if (EVENT_HEADER_FLAG_32_BIT_HEADER == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER))
	{
		g_PointerSize = 4;
	}
	else
	{
		g_PointerSize = 8;
	}

	if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
	{
		obj->Set(String::NewFromUtf8(isolate, "userData"), String::NewFromTwoByte(isolate, (uint16_t*)pEventRecord->UserData));
	}

	if (pInfo)
	{
		delete pInfo;
	}

    const unsigned argc = 1;
    Local<Value> argv[argc] = { obj };
    Local<Function> f = Local<Function>::New(isolate, this->mCb);
    f->Call(isolate->GetCurrentContext()->Global(), argc, argv);  
}

#endif
