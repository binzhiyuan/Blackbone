#include "MExcept.h"
#include "../Process/Process.h"
#include "../Include/Macro.h"
#include "../Misc/Trace.hpp"
#include "../Asm/LDasm.h"

namespace blackbone
{

// taken from CRT include <Ehdata.h>
#define EH_MAGIC_NUMBER1        0x19930520    
#define EH_PURE_MAGIC_NUMBER1   0x01994000
#define EH_EXCEPTION_NUMBER     ('msc' | 0xE0000000)

/*
LONG __declspec(naked) CALLBACK VectoredHandler( PEXCEPTION_POINTERS / *ExceptionInfo* / )
{
    PEXCEPTION_REGISTRATION_RECORD pFs;
    PRTL_INVERTED_FUNCTION_TABLE7 pTable;// = (PRTL_INVERTED_FUNCTION_TABLE7)0x777dc2b0;
    PRTL_INVERTED_FUNCTION_TABLE_ENTRY pEntries;
    void( __stdcall* LdrProtectMrdata )(BOOL a);
    PDWORD pDec, pStart;
    DWORD verMajor, verMinor, tmp;
    PEB_T* peb;
    bool newHandler, useNewTable;

    __asm
    {
        push ebp
        mov ebp, esp
        sub esp, __LOCAL_SIZE
        pushad
        mov eax, 0xDEADDA7A
        mov pTable, eax
        mov eax, 0xDEADC0D2
        mov LdrProtectMrdata, eax
    }

    pFs = reinterpret_cast<decltype(pFs)>(__readfsdword( 0 ));

    // Get OS version
    peb = reinterpret_cast<PEB_T*>(__readfsdword( 0x30 ));
    verMajor = peb->OSMajorVersion;
    verMinor = peb->OSMinorVersion;

    useNewTable = (verMajor >= 6 && verMinor >= 2) || verMajor >= 10;

    if (useNewTable)
        pEntries = reinterpret_cast<decltype(pEntries)>(GET_FIELD_PTR( reinterpret_cast<PRTL_INVERTED_FUNCTION_TABLE8>(pTable), Entries ));
    else
        pEntries = reinterpret_cast<decltype(pEntries)>(GET_FIELD_PTR( pTable, Entries ));


    //LdrProtectMrdata = (decltype(LdrProtectMrdata))0x777043aa;
    if (LdrProtectMrdata)
        LdrProtectMrdata( FALSE );

    //
    // Add each handler to LdrpInvertedFunctionTable
    //
    for (; pFs && pFs != reinterpret_cast<EXCEPTION_REGISTRATION_RECORD*>(0xffffffff)
        && pFs->Next != reinterpret_cast<EXCEPTION_REGISTRATION_RECORD*>(0xffffffff);
        pFs = pFs->Next)
    {
        // Find image for handler
        for (ULONG imageIndex = 0; imageIndex < pTable->Count; imageIndex++)
        {
            if (reinterpret_cast<uintptr_t>(pFs->Handler) >= reinterpret_cast<uintptr_t>(pEntries[imageIndex].ImageBase) &&
                reinterpret_cast<uintptr_t>(pFs->Handler) <= reinterpret_cast<uintptr_t>(pEntries[imageIndex].ImageBase)
                + pEntries[imageIndex].ImageSize)
            {
                newHandler = false;

                // Win8+ always has ntdll.dll as first image, so we can safely skip its handlers.
                // Also ntdll.dll ExceptionDirectory isn't Encoded via RtlEncodeSystemPointer (it's plain address)
                if (useNewTable && imageIndex == 0)
                    break;

                //pStart = (DWORD*)DecodeSystemPointer( pEntries[imageIndex].ExceptionDirectory );
                pStart = (DWORD*)((int( __stdcall* )(PVOID))0xDEADC0DE)(pEntries[imageIndex].ExceptionDirectory);

                //
                // Add handler as fake SAFESEH record
                //
                for (pDec = pStart; pDec != nullptr && pDec < pStart + 0x100; pDec++)
                {
                    if (*pDec == 0)
                    {
                        *pDec = reinterpret_cast<uintptr_t>(pFs->Handler) - reinterpret_cast<uintptr_t>(pEntries[imageIndex].ImageBase);
                        pEntries[imageIndex].SizeOfTable++;
                        newHandler = true;

                        break;
                    }
                    // Already in table
                    else if ((*pDec + reinterpret_cast<DWORD>(pEntries[imageIndex].ImageBase)) == reinterpret_cast<DWORD>(pFs->Handler))
                        break;
                }

                // Sort handler addresses
                if (newHandler)
                    for (ULONG i = 0; i < pEntries[imageIndex].SizeOfTable; i++)
                        for (ULONG j = pEntries[imageIndex].SizeOfTable - 1; j > i; j--)
                            if (pStart[j - 1] > pStart[j])
                            {
                                tmp = pStart[j - 1];
                                pStart[j - 1] = pStart[j];
                                pStart[j] = tmp;
                            }
            }
        }
    }

    if (LdrProtectMrdata)
        LdrProtectMrdata( TRUE );

    // Return control to SEH
    //return EXCEPTION_CONTINUE_SEARCH;
    __asm
    {
        popad
        mov esp, ebp
        pop ebp

        mov eax, EXCEPTION_CONTINUE_SEARCH
        ret 4
    }
}
*/
uint8_t MExcept::_handler32[]=
{
    0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x18, 0x60, 0xB8, 0x7A, 0xDA, 0xAD, 0xDE, 0x89, 0x45, 0xF4, 0xB8, 
    0xD2, 0xC0, 0xAD, 0xDE, 0x89, 0x45, 0xEC, 0x64, 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x64, 0x8B,
    0x0D, 0x30, 0x00, 0x00, 0x00, 0x89, 0x5D, 0xF8, 0x8B, 0x81, 0xA4, 0x00, 0x00, 0x00, 0x83, 0xF8, 
    0x06, 0x72, 0x09, 0x83, 0xB9, 0xA8, 0x00, 0x00, 0x00, 0x02, 0x73, 0x11, 0x83, 0xF8, 0x0A, 0x73, 
    0x0C, 0x8B, 0x7D, 0xF4, 0xC6, 0x45, 0xFF, 0x00, 0x83, 0xC7, 0x0C, 0xEB, 0x0A, 0x8B, 0x7D, 0xF4, 
    0xC6, 0x45, 0xFF, 0x01, 0x83, 0xC7, 0x10, 0x8B, 0x45, 0xEC, 0x89, 0x7D, 0xE8, 0x85, 0xC0, 0x74, 
    0x07, 0x6A, 0x00, 0xFF, 0xD0, 0x8B, 0x45, 0xEC, 0x85, 0xDB, 0x0F, 0x84, 0xEA, 0x00, 0x00, 0x00, 
    0x83, 0xFB, 0xFF, 0x0F, 0x84, 0xDE, 0x00, 0x00, 0x00, 0x83, 0x3B, 0xFF, 0x0F, 0x84, 0xD5, 0x00, 
    0x00, 0x00, 0x8B, 0x45, 0xF4, 0x33, 0xF6, 0x89, 0x75, 0xF0, 0x39, 0x30, 0x0F, 0x86, 0xB5, 0x00, 
    0x00, 0x00, 0x83, 0xC7, 0x04, 0x8B, 0x4B, 0x04, 0x8B, 0x17, 0x3B, 0xCA, 0x0F, 0x82, 0x8E, 0x00, 
    0x00, 0x00, 0x8B, 0x47, 0x04, 0x03, 0xC2, 0x3B, 0xC8, 0x0F, 0x87, 0x81, 0x00, 0x00, 0x00, 0x80, 
    0x7D, 0xFF, 0x00, 0x74, 0x08, 0x85, 0xF6, 0x0F, 0x84, 0x8A, 0x00, 0x00, 0x00, 0xFF, 0x77, 0xFC, 
    0xB8, 0xDE, 0xC0, 0xAD, 0xDE, 0xFF, 0xD0, 0x8B, 0xD8, 0x8B, 0xCB, 0x85, 0xDB, 0x74, 0x5E, 0x8D, 
    0xB3, 0x00, 0x04, 0x00, 0x00, 0x3B, 0xCE, 0x73, 0x54, 0x8B, 0x11, 0x85, 0xD2, 0x74, 0x13, 0x8B, 
    0x07, 0x03, 0xC2, 0x8B, 0x55, 0xF8, 0x3B, 0x42, 0x04, 0x74, 0x42, 0x83, 0xC1, 0x04, 0x75, 0xE5, 
    0xEB, 0x3B, 0x8B, 0x45, 0xF8, 0x8B, 0x40, 0x04, 0x2B, 0x07, 0x89, 0x01, 0xB9, 0x00, 0x00, 0x00, 
    0x00, 0x83, 0x47, 0x08, 0x01, 0x74, 0x26, 0x8B, 0x47, 0x08, 0x48, 0x3B, 0xC1, 0x76, 0x18, 0x90, 
    0x8B, 0x74, 0x83, 0xFC, 0x8B, 0x14, 0x83, 0x3B, 0xF2, 0x76, 0x07, 0x89, 0x54, 0x83, 0xFC, 0x89, 
    0x34, 0x83, 0x48, 0x3B, 0xC1, 0x77, 0xE9, 0x41, 0x3B, 0x4F, 0x08, 0x72, 0xDA, 0x8B, 0x5D, 0xF8, 
    0x8B, 0x4D, 0xF0, 0x83, 0xC7, 0x10, 0x8B, 0x45, 0xF4, 0x41, 0x89, 0x4D, 0xF0, 0x8B, 0xF1, 0x3B, 
    0x08, 0x0F, 0x82, 0x4E, 0xFF, 0xFF, 0xFF, 0x8B, 0x1B, 0x8B, 0x7D, 0xE8, 0x89, 0x5D, 0xF8, 0x85, 
    0xDB, 0x0F, 0x85, 0x19, 0xFF, 0xFF, 0xFF, 0x8B, 0x45, 0xEC, 0x85, 0xC0, 0x74, 0x04, 0x6A, 0x01, 
    0xFF, 0xD0, 0x61, 0x8B, 0xE5, 0x5D, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xC2, 0x04, 0x00
};

/*
LONG CALLBACK VectoredHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
    // Check if it's a VC++ exception
    // for SEH RtlAddFunctionTable is enough
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EH_EXCEPTION_NUMBER)
    {
        ModuleTable* pTable = reinterpret_cast<ModuleTable*>(0xDEADBEEFDEADBEEF);
        for (ptr_t i = 0; i < pTable->count; i++)
        {
            // Check exception site image boundaries
            if (ExceptionInfo->ExceptionRecord->ExceptionInformation[2] >= pTable->entry[i].base &&
                ExceptionInfo->ExceptionRecord->ExceptionInformation[2] <= pTable->entry[i].base + pTable->entry[i].size)
            {
                // Assume that's our exception because ImageBase = 0 and not suitable magic number
                if (ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == EH_PURE_MAGIC_NUMBER1
                    && ExceptionInfo->ExceptionRecord->ExceptionInformation[3] == 0)
                {
                    // CRT magic number
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[0] = (ULONG_PTR)EH_MAGIC_NUMBER1;

                    // fix exception image base
                    ExceptionInfo->ExceptionRecord->ExceptionInformation[3] = (ULONG_PTR)pTable->entry[i].base;
                }
            }
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}*/
uint8_t MExcept::_handler64[] =
{
    0x48, 0x83, 0xEC, 0x08, 0x48, 0x8B, 0x01, 0x4C, 0x8B, 0xD9, 0x81, 0x38, 0x63, 0x73, 0x6D, 0xE0, 
    0x0F, 0x85, 0x7C, 0x00, 0x00, 0x00, 0x48, 0x89, 0x1C, 0x24, 0x45, 0x33, 0xC9, 0x48, 0xBB, 0xEF, 
    0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0x4C, 0x39, 0x0B, 0x76, 0x5B, 0x48, 0xB8, 0xF7, 0xBE, 
    0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4D, 0x8B, 0x03, 0x48, 0x8B, 0x10, 0x4D, 0x8B, 0x50, 0x30, 0x4C, 0x3B, 0xD2, 0x72, 0x2C, 0x48, 
    0x03, 0x50, 0x08, 0x4C, 0x3B, 0xD2, 0x77, 0x23, 0x49, 0x81, 0x78, 0x20, 0x00, 0x40, 0x99, 0x01,
    0x75, 0x19, 0x49, 0x83, 0x78, 0x38, 0x00, 0x75, 0x12, 0x49, 0xC7, 0x40, 0x20, 0x20, 0x05, 0x93, 
    0x19, 0x49, 0x8B, 0x13, 0x48, 0x8B, 0x08, 0x48, 0x89, 0x4A, 0x38, 0x49, 0xFF, 0xC1, 0x48, 0x83, 
    0xC0, 0x10, 0x4C, 0x3B, 0x0B, 0x72, 0xB9, 0x33, 0xC0, 0x48, 0x8B, 0x1C, 0x24, 0x48, 0x83, 0xC4, 
    0x08, 0xC3, 0x33, 0xC0, 0x48, 0x83, 0xC4, 0x08, 0xC3
};

MExcept::MExcept( class Process& proc )
    : _proc( proc )
{
    //AddVectoredExceptionHandler( 0, &VectoredHandler );
}

MExcept::~MExcept()
{
}

/// <summary>
/// Inject VEH wrapper into process
/// Used to enable execution of SEH handlers out of image
/// </summary>
/// <param name="pTargetBase">Target image base address</param>
/// <param name="imageSize">Size of the image</param>
/// <param name="mt">Mosule type</param>
/// <param name="partial">Partial exception support</param>
/// <returns>Error code</returns>
NTSTATUS MExcept::CreateVEH( uintptr_t pTargetBase, size_t imageSize, eModType mt, bool partial )
{    
    auto a = AsmFactory::GetAssembler( mt );
    uint64_t result = 0;
    auto& mods = _proc.modules();

    if(mt == mt_mod64)
    {
        // Add module to module table
        if (!_pModTable.valid())
        {
            auto mem = _proc.memory().Allocate( 0x1000, PAGE_READWRITE, 0, false );
            if (!mem)
                return mem.status;

            _pModTable = std::move( mem.result() );
        }

        ModuleTable table;
        _pModTable.Read ( 0, table );

        // Add new entry to the table
        table.entry[table.count].base = pTargetBase;
        table.entry[table.count].size = imageSize;
        table.count++;

        _pModTable.Write( 0, table );

        // No handler required
        if (partial)
            return STATUS_SUCCESS;

        // VEH codecave
        auto mem = _proc.memory().Allocate( 0x2000, PAGE_EXECUTE_READWRITE, 0, false );
        if (!mem)
            return mem.status;

        _pVEHCode = std::move( mem.result() );

        BLACKBONE_TRACE( "ManualMap: Vectored hander: 0x%p\n", _pVEHCode.ptr() );

        for (uint8_t *pData = _handler64; pData < _handler64 + sizeof( _handler64 ) - sizeof( uint8_t* ); pData++)
        {
            // Module table pointer
            if (*(ptr_t*)pData == 0xDEADBEEFDEADBEEF)
            {
                *(ptr_t*)pData = _pModTable.ptr();
                continue;
            }

            // ModuleTable::count
            if (*(ptr_t*)pData == 0xDEADBEEFDEADBEF7)
            {
                *(ptr_t*)pData = _pModTable.ptr() + sizeof( ptr_t );
                continue;
            }
        }

        if (_pVEHCode.Write( 0, sizeof( _handler64 ), _handler64 ) != STATUS_SUCCESS)
        {
            _pVEHCode.Free();
            return LastNtStatus();
        }       
    }
    else
    {
        // No handler required
        if (partial)
            return STATUS_SUCCESS;

        // VEH codecave
        auto mem = _proc.memory().Allocate( 0x2000, PAGE_EXECUTE_READWRITE, 0, false );
        if (!mem)
            return mem.status;

        _pVEHCode = std::move( mem.result() );

        auto pDecode = mods.GetNtdllExport( "RtlDecodeSystemPointer", mt, Sections );
        if (!pDecode)
            return pDecode.status;

        // Find and replace magic values
        for (uint8_t *pData = _handler32; pData < _handler32 + sizeof( _handler32 ) - sizeof( uint8_t* ); pData++)
        {
            // LdrpInvertedFunctionTable
            if (*(uint32_t*)pData == 0xDEADDA7A)
            {
                *(uint32_t*)pData = static_cast<uint32_t>(_proc.nativeLdr().LdrpInvertedFunctionTable());
                continue;
            }

            // DecodeSystemPointer address
            if (*(uint32_t*)pData == 0xDEADC0DE)
            {
                *(uint32_t*)pData = static_cast<uint32_t>(pDecode->procAddress);
                break;
            }

            // LdrProtectMrdata address
            if (*(uint32_t*)pData == 0xDEADC0D2)
            {
                *(uint32_t*)pData = static_cast<uint32_t>(_proc.nativeLdr().LdrProtectMrdata());
                continue;
            }
        }

        // Write handler data into target process
        if (!NT_SUCCESS( _pVEHCode.Write( 0, sizeof( _handler32 ), _handler32 ) ))
        {
            _pVEHCode.Free();
            return LastNtStatus();
        }
    }

    // AddVectoredExceptionHandler(0, pHandler);
    auto pAddHandler = mods.GetNtdllExport( "RtlAddVectoredExceptionHandler", mt, Sections );
    if (!pAddHandler)
        return pAddHandler.status;

    (*a)->reset();
    a->GenPrologue();

    a->GenCall( pAddHandler->procAddress, { 0, _pVEHCode.ptr() } );

    _proc.remote().AddReturnWithEvent( *a, mt );
    a->GenEpilogue();

    _proc.remote().ExecInWorkerThread( (*a)->make(), (*a)->getCodeSize(), result );
    _hVEH = result;

    return (_hVEH == 0 ? STATUS_NOT_FOUND : STATUS_SUCCESS);
}

/// <summary>
/// Removes VEH from target process
/// </summary>
/// <param name="partial">Partial exception support</param>
/// <param name="mt">Mosule type</param>
/// <returns>Status code</returns>
NTSTATUS MExcept::RemoveVEH( bool partial, eModType mt )
{  
    auto a = AsmFactory::GetAssembler( mt );
    uint64_t result = 0;

    auto& mods = _proc.modules();
    auto pRemoveHandler = mods.GetNtdllExport( "RtlRemoveVectoredExceptionHandler", mt );
    if (!pRemoveHandler)
        return pRemoveHandler.status;

    a->GenPrologue();

    // RemoveVectoredExceptionHandler(pHandler);
    a->GenCall( pRemoveHandler->procAddress, { _hVEH } );

    _proc.remote().AddReturnWithEvent( *a );
    a->GenEpilogue();

    // Destroy table and handler
    if (!partial)
    {
        _proc.remote().ExecInWorkerThread( (*a)->make(), (*a)->getCodeSize(), result );
        _pVEHCode.Free();
        _hVEH = 0;

        _pModTable.Free();
    }        

    return STATUS_SUCCESS;
}

}
