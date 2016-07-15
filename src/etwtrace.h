#pragma once
#ifndef _etwtrace_h_
#define  _etwtrace_h_


#include <nan.h>
#include <tchar.h>
#include <stdio.h>
#include <Windows.h>
#include <SDKDDKVer.h>
#include <Evntcons.h>
#include <Evntrace.h>


struct ITraceConsumer {
    virtual void OnEventRecord(PEVENT_RECORD eventPointer) = 0;
};

class TraceSession
{

public:
    TraceSession(LPCTSTR szSessionName);
    ~TraceSession();

public:
    bool Start();
    bool EnableProvider(const GUID& providerId, UCHAR level, ULONGLONG anyKeyword = 0, ULONGLONG allKeyword = 0);
    bool OpenTrace(ITraceConsumer *pConsumer);
    bool Process();
    bool CloseTrace();
    bool DisableProvider(const GUID& providerId);
    bool Stop();

    ULONG Status() const;
    LONGLONG PerfFreq() const;

private:
    LPTSTR _szSessionName;
    ULONG _status;
    EVENT_TRACE_PROPERTIES* _pSessionProperties;
    TRACEHANDLE hSession;
    EVENT_TRACE_LOGFILE _logFile;
    TRACEHANDLE _hTrace;
};

#endif
