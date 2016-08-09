#ifndef _eventproperties_h_
#define  _eventproperties_h_

#include <windows.h>
#include <in6addr.h>
#include <evntprov.h>
#include <evntrace.h> 
#include <evntcons.h>
#include <tdh.h>
#include <node.h>
#include "asprintf.h"

DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO & pInfo);
DWORD GetEventProperties(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i, LPWSTR pStructureName, USHORT StructIndex, Handle<Value>* pLocalObject);
DWORD GetFormattedData(PEVENT_RECORD pEvent, USHORT InType, USHORT OutType, PBYTE pData, DWORD DataSize, PEVENT_MAP_INFO pMapInfo, Handle<Value>* pLocalObject);
void PrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData, Handle<Value>* pLocalObject);
DWORD GetArraySize(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i, PUSHORT ArraySize);
DWORD GetMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource, PEVENT_MAP_INFO & pMapInfo);
void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo);

#define MAX_NAME 256

typedef LPTSTR(NTAPI *PIPV6ADDRTOSTRING) (
    const IN6_ADDR *Addr,
    LPTSTR S
);

void PrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData, Handle<Value>* pLocalObject)
{
    BOOL MatchFound = FALSE;

    if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_MANIFEST_VALUEMAP) == EVENTMAP_INFO_FLAG_MANIFEST_VALUEMAP ||
        ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_VALUEMAP) == EVENTMAP_INFO_FLAG_WBEM_VALUEMAP &&
        (pMapInfo->Flag & (~EVENTMAP_INFO_FLAG_WBEM_VALUEMAP)) != EVENTMAP_INFO_FLAG_WBEM_FLAG))
    {
        if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_NO_MAP) == EVENTMAP_INFO_FLAG_WBEM_NO_MAP)
        {
            wprintf(L"%s\n", (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[*(PULONG)pData].OutputOffset));
        }
        else
        {
            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if (pMapInfo->MapEntryArray[i].Value == *(PULONG)pData)
                {
                    wprintf(L"%s\n", (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));
                    MatchFound = TRUE;
                    break;
                }
            }

            if (FALSE == MatchFound)
            {
                wprintf(L"%lu\n", *(PULONG)pData);
            }
        }
    }
    else if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_MANIFEST_BITMAP) == EVENTMAP_INFO_FLAG_MANIFEST_BITMAP ||
        (pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_BITMAP) == EVENTMAP_INFO_FLAG_WBEM_BITMAP ||
        ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_VALUEMAP) == EVENTMAP_INFO_FLAG_WBEM_VALUEMAP &&
        (pMapInfo->Flag & (~EVENTMAP_INFO_FLAG_WBEM_VALUEMAP)) == EVENTMAP_INFO_FLAG_WBEM_FLAG))
    {
        if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_NO_MAP) == EVENTMAP_INFO_FLAG_WBEM_NO_MAP)
        {
            DWORD BitPosition = 0;

            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if ((*(PULONG)pData & (BitPosition = (1 << i))) == BitPosition)
                {
                    wprintf(L"%s%s",
                        (MatchFound) ? L" | " : L"",
                        (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));

                    MatchFound = TRUE;
                }
            }

        }
        else
        {
            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if ((pMapInfo->MapEntryArray[i].Value & *(PULONG)pData) == pMapInfo->MapEntryArray[i].Value)
                {
                    wprintf(L"%s%s",
                        (MatchFound) ? L" | " : L"",
                        (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));

                    MatchFound = TRUE;
                }
            }
        }

        if (MatchFound)
        {
            wprintf(L"\n");
        }
        else
        {
            wprintf(L"%lu\n", *(PULONG)pData);
        }
    }
}

DWORD GetFormattedData(PEVENT_RECORD pEvent, USHORT InType, USHORT OutType, PBYTE pData, DWORD DataSize, PEVENT_MAP_INFO pMapInfo, Handle<Value>* pLocalObject)
{
    UNREFERENCED_PARAMETER(pEvent);
    DWORD status = ERROR_SUCCESS;
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    switch (InType)
    {
    case TDH_INTYPE_UNICODESTRING:
    case TDH_INTYPE_COUNTEDSTRING:
    case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
    case TDH_INTYPE_NONNULLTERMINATEDSTRING:
    {
        size_t StringLength = 0;

        if (TDH_INTYPE_COUNTEDSTRING == InType)
        {
            StringLength = *(PUSHORT)pData;
        }
        else if (TDH_INTYPE_REVERSEDCOUNTEDSTRING == InType)
        {
            StringLength = MAKEWORD(HIBYTE((PUSHORT)pData), LOBYTE((PUSHORT)pData));
        }
        else if (TDH_INTYPE_NONNULLTERMINATEDSTRING == InType)
        {
            StringLength = DataSize;
        }
        else
        {
            StringLength = wcslen((LPWSTR)pData);
        }

       // wprintf(L"%.*s\n", StringLength, (LPWSTR)pData);
		wchar_t* buff = NULL;
		aswprintf(&buff, L"%.*s\n", StringLength, (LPWSTR)pData);
		*pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buff);
		free(buff);
        break;
    }

    case TDH_INTYPE_ANSISTRING:
    case TDH_INTYPE_COUNTEDANSISTRING:
    case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:
    case TDH_INTYPE_NONNULLTERMINATEDANSISTRING:
    {
        size_t StringLength = 0;

        if (TDH_INTYPE_COUNTEDANSISTRING == InType)
        {
            StringLength = *(PUSHORT)pData;
        }
        else if (TDH_INTYPE_REVERSEDCOUNTEDANSISTRING == InType)
        {
            StringLength = MAKEWORD(HIBYTE((PUSHORT)pData), LOBYTE((PUSHORT)pData));
        }
        else if (TDH_INTYPE_NONNULLTERMINATEDANSISTRING == InType)
        {
            StringLength = DataSize;
        }
        else
        {
            StringLength = strlen((LPSTR)pData);
        }

        //wprintf(L"%.*S\n", StringLength, (LPSTR)pData);
        wchar_t* buf = new wchar_t[StringLength + 100];
        swprintf(buf, L"%.*S", StringLength, (LPSTR)pData);
        *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
        delete[] buf;
        break;
    }

    case TDH_INTYPE_INT8:
    {
        *pLocalObject = Integer::New(isolate, *(PCHAR)pData);
        //wprintf(L"%hd\n", *(PCHAR)pData);
        break;
    }

    case TDH_INTYPE_UINT8:
    {
        if (TDH_OUTTYPE_HEXINT8 == OutType)
        {
            wchar_t buf[100];
            wsprintf(buf, L"0x%x\n", *(PBYTE)pData);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            //wprintf(L"0x%x\n", *(PBYTE)pData);
        }
        else
        {
            *pLocalObject = Integer::New(isolate, *(PBYTE)pData);
            //wprintf(L"%hu\n", *(PBYTE)pData);
        }

        break;
    }

    case TDH_INTYPE_INT16:
    {
        *pLocalObject = Integer::New(isolate, *(PSHORT)pData);
        //wprintf(L"%hd\n", *(PSHORT)pData);
        break;
    }

    case TDH_INTYPE_UINT16:
    {
        if (TDH_OUTTYPE_HEXINT16 == OutType)
        {
            wchar_t buf[100];
            wsprintf(buf, L"0x%x\n", *(PUSHORT)pData);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            //wprintf(L"0x%x\n", *(PUSHORT)pData);
        }
        else if (TDH_OUTTYPE_PORT == OutType)
        {
            *pLocalObject = Integer::NewFromUnsigned(isolate, ntohs(*(PUSHORT)pData));
            //wprintf(L"%hu\n", ntohs(*(PUSHORT)pData));
        }
        else
        {
            *pLocalObject = Integer::NewFromUnsigned(isolate, *(PUSHORT)pData);
            //wprintf(L"%hu\n", *(PUSHORT)pData);
        }

        break;
    }

    case TDH_INTYPE_INT32:
    {
        if (TDH_OUTTYPE_HRESULT == OutType)
        {
            wchar_t buf[100];
            wsprintf(buf, L"0x%x\n", *(PLONG)pData);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            //wprintf(L"0x%x\n", *(PLONG)pData);
        }
        else
        {
            *pLocalObject = Integer::New(isolate, *(PLONG)pData);
            //wprintf(L"%d\n", *(PLONG)pData);
        }

        break;
    }

    case TDH_INTYPE_UINT32:
    {
        if (TDH_OUTTYPE_HRESULT == OutType ||
            TDH_OUTTYPE_WIN32ERROR == OutType ||
            TDH_OUTTYPE_NTSTATUS == OutType ||
            TDH_OUTTYPE_HEXINT32 == OutType)
        {
            wchar_t buf[100];
            wsprintf(buf, L"0x%x\n", *(PULONG)pData);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            //wprintf(L"0x%x\n", *(PULONG)pData);
        }
        else if (TDH_OUTTYPE_IPV4 == OutType)
        {
            wchar_t buf[100];
            wsprintf(buf, L"%d.%d.%d.%d\n", (*(PLONG)pData >> 0) & 0xff,
                (*(PLONG)pData >> 8) & 0xff,
                (*(PLONG)pData >> 16) & 0xff,
                (*(PLONG)pData >> 24) & 0xff);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            /*
            wprintf(L"%d.%d.%d.%d\n", (*(PLONG)pData >> 0) & 0xff,
                (*(PLONG)pData >> 8) & 0xff,
                (*(PLONG)pData >> 16) & 0xff,
                (*(PLONG)pData >> 24) & 0xff);
            */
        }
        else
        {
            if (pMapInfo)
            {
                PrintMapString(pMapInfo, pData, pLocalObject);
            }
            else
            {
                *pLocalObject = Integer::NewFromUnsigned(isolate, *(PULONG)pData);
                //wprintf(L"%lu\n", *(PULONG)pData);
            }
        }

        break;
    }

    case TDH_INTYPE_INT64:
    {
        *pLocalObject = Integer::New(isolate, *(PLONGLONG)pData);
        //wprintf(L"%I64d\n", *(PLONGLONG)pData);

        break;
    }

    case TDH_INTYPE_UINT64:
    {
        if (TDH_OUTTYPE_HEXINT64 == OutType)
        {
            wchar_t buf[256];
            wsprintf(buf, L"0x%x\n", *(PULONGLONG)pData);
            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
            //wprintf(L"0x%x\n", *(PULONGLONG)pData);
        }
        else
        {
            *pLocalObject = Integer::NewFromUnsigned(isolate, *(PULONGLONG)pData);
            //wprintf(L"%I64u\n", *(PULONGLONG)pData);
        }

        break;
    }

    case TDH_INTYPE_FLOAT:
    {
        *pLocalObject = Number::New(isolate, *(PFLOAT)pData);
        //wprintf(L"%f\n", *(PFLOAT)pData);

        break;
    }

    case TDH_INTYPE_DOUBLE:
    {
        *pLocalObject = Number::New(isolate, *(DOUBLE*)pData);
        //wprintf(L"%I64f\n", *(DOUBLE*)pData);

        break;
    }

    case TDH_INTYPE_BOOLEAN:
    {
        *pLocalObject = Boolean::New(isolate, (0 == (PBOOL)pData) ? false : true);
        //wprintf(L"%s\n", (0 == (PBOOL)pData) ? L"false" : L"true");

        break;
    }

    case TDH_INTYPE_BINARY:
    {
        if (TDH_OUTTYPE_IPV6 == OutType)
        {
            WCHAR IPv6AddressAsString[46];
            PIPV6ADDRTOSTRING fnRtlIpv6AddressToString;

            fnRtlIpv6AddressToString = (PIPV6ADDRTOSTRING)GetProcAddress(GetModuleHandle(L"ntdll"), "RtlIpv6AddressToStringW");

            if (NULL == fnRtlIpv6AddressToString)
            {
                status = GetLastError();
                goto cleanup;
            }

            fnRtlIpv6AddressToString((IN6_ADDR*)pData, IPv6AddressAsString);

            *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)IPv6AddressAsString);
            //wprintf(L"%s\n", IPv6AddressAsString);
        }
        else
        {
            for (DWORD i = 0; i < DataSize; i++)
            {
                
                wprintf(L"%.2x", pData[i]);
            }

            wprintf(L"\n");
        }

        break;
    }

    case TDH_INTYPE_GUID:
    {
        WCHAR szGuid[50];

        StringFromGUID2(*(GUID*)pData, szGuid, sizeof(szGuid) - 1);
        wprintf(L"%s\n", szGuid);

        break;
    }

    case TDH_INTYPE_POINTER:
    case TDH_INTYPE_SIZET:
    {
        if (4 == g_PointerSize)
        {
            wprintf(L"0x%x\n", *(PULONG)pData);
        }
        else
        {
            wprintf(L"0x%x\n", *(PULONGLONG)pData);
        }

        break;
    }

    case TDH_INTYPE_FILETIME:
    {
        break;
    }

    case TDH_INTYPE_SYSTEMTIME:
    {
        break;
    }

    case TDH_INTYPE_SID:
    {
        WCHAR UserName[MAX_NAME];
        WCHAR DomainName[MAX_NAME];
        DWORD cchUserSize = MAX_NAME;
        DWORD cchDomainSize = MAX_NAME;
        SID_NAME_USE eNameUse;

        if (!LookupAccountSid(NULL, (PSID)pData, UserName, &cchUserSize, DomainName, &cchDomainSize, &eNameUse))
        {
            if (ERROR_NONE_MAPPED == status)
            {
                status = ERROR_SUCCESS;
            }
            else
            {
                status = GetLastError();
            }

            goto cleanup;
        }
        else
        {
            Local<Object> userObj = Object::New(isolate);
            userObj->Set(String::NewFromUtf8(isolate, "domain"), String::NewFromTwoByte(isolate, (uint16_t*)DomainName));
            userObj->Set(String::NewFromUtf8(isolate, "user"), String::NewFromTwoByte(isolate, (uint16_t*)UserName));
            *pLocalObject = userObj;
            //wprintf(L"%s\\%s\n", DomainName, UserName);
        }

        break;
    }

    case TDH_INTYPE_HEXINT32:
    {
        wchar_t buf[512];
        wsprintf(buf, L"0x%x", (PULONG)pData);
        *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
        //wprintf(L"0x%x\n", (PULONG)pData);
        break;
    }

    case TDH_INTYPE_HEXINT64:
    {
        wchar_t buf[512];
        wsprintf(buf, L"0x%x", (PULONGLONG)pData);
        *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)buf);
        
        //wprintf(L"0x%x\n", (PULONGLONG)pData);
        break;
    }

    case TDH_INTYPE_UNICODECHAR:
    {;
        *pLocalObject = String::NewFromTwoByte(isolate, (uint16_t*)pData);
        //wprintf(L"%c\n", *(PWCHAR)pData);
        break;
    }

    case TDH_INTYPE_ANSICHAR:
    {
        *pLocalObject = String::NewFromUtf8(isolate, (PCHAR)pData);
        //wprintf(L"%C\n", *(PCHAR)pData);
        break;
    }

    case TDH_INTYPE_WBEMSID:
    {
        WCHAR UserName[MAX_NAME];
        WCHAR DomainName[MAX_NAME];
        DWORD cchUserSize = MAX_NAME;
        DWORD cchDomainSize = MAX_NAME;
        SID_NAME_USE eNameUse;

        if ((PULONG)pData > 0)
        {
            // A WBEM SID is actually a TOKEN_USER structure followed 
            // by the SID. The size of the TOKEN_USER structure differs 
            // depending on whether the events were generated on a 32-bit 
            // or 64-bit architecture. Also the structure is aligned
            // on an 8-byte boundary, so its size is 8 bytes on a
            // 32-bit computer and 16 bytes on a 64-bit computer.
            // Doubling the pointer size handles both cases.

            pData += g_PointerSize * 2;

            if (!LookupAccountSid(NULL, (PSID)pData, UserName, &cchUserSize, DomainName, &cchDomainSize, &eNameUse))
            {
                if (ERROR_NONE_MAPPED == status)
                {
                    status = ERROR_SUCCESS;
                }
                else
                {
                    status = GetLastError();
                }

                goto cleanup;
            }
            else
            {
                Handle<Object> userObj = Object::New(isolate);
                userObj->Set(String::NewFromUtf8(isolate, "domain"), String::NewFromTwoByte(isolate, (uint16_t*)DomainName));
                Handle<Object> localObj = Object::New(isolate);
                localObj->Set(String::NewFromUtf8(isolate, "account"), String::NewFromTwoByte(isolate, (uint16_t*)UserName));
                *pLocalObject = localObj;
                wprintf(L"%s\\%s\n", DomainName, UserName);
            }
        }

        break;
    }

    default:
        status = ERROR_NOT_FOUND;
    }

cleanup:

    return status;
}

void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo)
{
    SIZE_T ByteLength = 0;

    for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
    {
        ByteLength = (wcslen((LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset)) - 1) * 2;
        *((LPWSTR)((PBYTE)pMapInfo + (pMapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
    }
}

DWORD GetMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource, PEVENT_MAP_INFO & pMapInfo)
{
    DWORD status = ERROR_SUCCESS;
    DWORD mapSize = 0;

    status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &mapSize);

    if (ERROR_INSUFFICIENT_BUFFER == status)
    {
        pMapInfo = (PEVENT_MAP_INFO)malloc(mapSize);
        if (pMapInfo == NULL)
        {
            return ERROR_OUTOFMEMORY;
        }

        status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &mapSize);
    }

    if (ERROR_SUCCESS == status)
    {
        if (DecodingSourceXMLFile == DecodingSource)
        {
            RemoveTrailingSpace(pMapInfo);
        }
    }
    else if (ERROR_NOT_FOUND == status)
    {
        status = ERROR_SUCCESS;
    }

    return status;
}

DWORD GetArraySize(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i, PUSHORT ArraySize)
{
    DWORD status = ERROR_SUCCESS;
    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    DWORD PropertySize = 0;

    if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyParamCount) == PropertyParamCount)
    {
        DWORD Count = 0;
        DWORD j = pInfo->EventPropertyInfoArray[i].countPropertyIndex;
        ZeroMemory(&DataDescriptor, sizeof(PROPERTY_DATA_DESCRIPTOR));
        DataDescriptor.PropertyName = (ULONGLONG)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[j].NameOffset);
        DataDescriptor.ArrayIndex = ULONG_MAX;
        status = TdhGetPropertySize(pEvent, 0, NULL, 1, &DataDescriptor, &PropertySize);
        status = TdhGetProperty(pEvent, 0, NULL, 1, &DataDescriptor, PropertySize, (PBYTE)&Count);
        *ArraySize = (USHORT)Count;
    }
    else
    {
        *ArraySize = pInfo->EventPropertyInfoArray[i].count;
    }

    return status;
}

DWORD GetEventProperties(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i, LPWSTR pStructureName, USHORT StructIndex, Handle<Value>* pLocalObject)
{
    DWORD status = ERROR_SUCCESS;
    DWORD LastMember = 0;  // Last member of a structure
    USHORT ArraySize = 0;
    PEVENT_MAP_INFO pMapInfo = NULL;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    status = GetArraySize(pEvent, pInfo, i, &ArraySize);

    for (USHORT k = 0; k < ArraySize; k++)
    {
        LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].NameOffset);
        Handle<Value> objName = String::NewFromTwoByte(isolate, (uint16_t*)propertyName);
        wprintf(L"%*s%s: ", (pStructureName) ? 4 : 0, L"", (LPWSTR)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].NameOffset));

        // If the property is a structure, print the members of the structure
        if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyStruct) == PropertyStruct)
        {
            wprintf(L"\n");

            LastMember = pInfo->EventPropertyInfoArray[i].structType.StructStartIndex +
                pInfo->EventPropertyInfoArray[i].structType.NumOfStructMembers;

            for (USHORT j = pInfo->EventPropertyInfoArray[i].structType.StructStartIndex; j < LastMember; j++)
            {
                Handle<Value> innerObj;
                status = GetEventProperties(pEvent, pInfo, j, (LPWSTR)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].NameOffset), k, &innerObj);
                if (ERROR_SUCCESS != status)
                {
                    goto cleanup;
                }
            }
        }
        else
        {
            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));
            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            if (TDH_INTYPE_BINARY == pInfo->EventPropertyInfoArray[i].nonStructType.InType &&
                TDH_OUTTYPE_IPV6 == pInfo->EventPropertyInfoArray[i].nonStructType.OutType)
            {
                status = ERROR_EVT_INVALID_EVENT_DATA;
                break;
            }
            else
            {
                status = TdhGetPropertySize(pEvent, 0, NULL, DescriptorsCount, &DataDescriptors[0], &PropertySize);

                if (ERROR_SUCCESS != status)
                {
                    goto cleanup;
                }

                pData = (PBYTE)malloc(PropertySize);

                if (NULL == pData)
                {
                    status = ERROR_OUTOFMEMORY;
                    goto cleanup;
                }

                status = TdhGetProperty(pEvent, 0, NULL, DescriptorsCount, &DataDescriptors[0], PropertySize, pData);

                // Get the name/value mapping if the property specifies a value map.
                status = GetMapInfo(pEvent,
                    (PWCHAR)((PBYTE)(pInfo)+pInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
                    pInfo->DecodingSource,
                    pMapInfo);

                if (ERROR_SUCCESS != status)
                {
                    goto cleanup;
                }

                Handle<Value> localFormatted;
                status = GetFormattedData(pEvent,
                    pInfo->EventPropertyInfoArray[i].nonStructType.InType,
                    pInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                    pData,
                    PropertySize,
                    pMapInfo,
                    &localFormatted
                );

                if (ERROR_SUCCESS != status)
                {
                    goto cleanup;
                }

                if (pData)
                {
                    free(pData);
                    pData = NULL;
                }

                if (pMapInfo)
                {
                    free(pMapInfo);
                    pMapInfo = NULL;
                }
            }
        }
    }

cleanup:

    if (pData)
    {
        free(pData);
        pData = NULL;
    }

    if (pMapInfo)
    {
        free(pMapInfo);
        pMapInfo = NULL;
    }

    return status;
}

DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO& pInfo)
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

#endif
