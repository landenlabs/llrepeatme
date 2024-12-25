///
// llRepeatMe v1.1  March 2010
//
// Author: Dennis Lang 2010
// https://lanenlabs.com
//
// llRepeatMe program repeats execution of target program.
//
//


#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>

#include <psapi.h>
#pragma comment( lib, "psapi.lib" )

extern void EnterDebugLoop(const LPDEBUG_EVENT DebugEv);

//-----------------------------------------------------------------------------
static TCHAR* ProcName(TCHAR* pCmd, TCHAR* outProcName, int outSize)
{
    static TCHAR sAuxChar[] = _T("\"*:<>?\\/|");   // not valid in filename

    TCHAR* pDst = outProcName;
    TCHAR* pLastDot = _tcsrchr(pCmd, _T('.')); 
    TCHAR* pLastSlash = _tcsrchr(pCmd, _T('\\')); 

    if (pLastSlash != NULL)
        pCmd = ++pLastSlash;
    
    while (outSize-- != 0 && *pCmd >= _T(' '))
    {
        if (pCmd == pLastDot)
            break;
        *pDst++ = *pCmd++;
    }

    *pDst++  = _T('\0');
    return outProcName;
}

//-----------------------------------------------------------------------------
DWORD RunProgram(TCHAR* cmdBuf)
{
    // Start the child process.
    // Start suspended so counters can be attached and use debug to detected program exit.
    // DWORD crFlags = CREATE_SUSPENDED | DEBUG_ONLY_THIS_PROCESS;
    DWORD crFlags = 0; //  DEBUG_PROCESS;
    // DWORD crFlags = CREATE_SUSPENDED;
    
    
    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory( &pi, sizeof(pi) );

    if( !CreateProcess( NULL,   // No module name (use command line)
        cmdBuf,         // Command line
        NULL,           // Process handle not inheritable    
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        crFlags,        // creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
            ) 
    {
        wprintf(L"[Error %d] - Failed to start process '%s'\n", GetLastError(), cmdBuf );
        return GetLastError();
    }

    // ResumeThread(pi.hThread);

    if ((crFlags & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS)) != 0)
    {
        // Run program in debug mode to detect program exit
        // Note: debug mode may slow program's execution.
        DEBUG_EVENT debugEv;
        debugEv.dwProcessId = pi.dwProcessId;
        debugEv.dwThreadId = pi.dwThreadId;
        EnterDebugLoop(&debugEv);
    }
    else
    {
        // Wait until child process exits.
        while (WaitForSingleObject( pi.hProcess, 10) == WAIT_TIMEOUT)
        {
        }
    }

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return ERROR_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
// Convert special characters from text to binary.
// [static]
const TCHAR* convertSpecialChar( TCHAR* inPtr) {
    unsigned int len = 0;
    int x, n, scnt;
    const TCHAR* begPtr = inPtr;
    TCHAR* outPtr = inPtr;
    while (*inPtr) {
        if (*inPtr == '\\') {
            inPtr++;
            switch (*inPtr) {
            // case 'n': *outPtr++ = '\n'; break;
            // case 't': *outPtr++ = '\t'; break;
            // case 'v': *outPtr++ = '\v'; break;
            // case 'b': *outPtr++ = '\b'; break;
            // case 'r': *outPtr++ = '\r'; break;
            // case 'f': *outPtr++ = '\f'; break;
            // case 'a': *outPtr++ = '\a'; break;
            case '<':
            case '%':
            // case '?':
            // case '\\':
            // case '\'':
            // case '"':
                *outPtr++ = *inPtr;
                break;
            }

            inPtr++;
        } else
            *outPtr++ = *inPtr++;
        len++;
    }

    *outPtr = '\0';
    return begPtr;
}

//-----------------------------------------------------------------------------
int _tmain( int argc, TCHAR *argv[] )
{
    if (argc < 3)
    {
        wprintf(L"llRepeatMe v1.2 Dec 2024 - Repeat execution of target program\n\n"
                L"Author: Dennis Lang\n"
                L"https://lanenlabs.com\n\n"
                L"Usage: \n"
                L"    <seconds> <programToRun> [programArgs]...\n"
                L"   [-wait=<seconds>] [-repeat=<repeatCnt>] <programToRun> [programArgs]...\n"
                L"   seconds to sleep between runs\n"
                L"   repeatCnt defaults to 10\n"
                L"   IF command string contains %%d it is replaced with iteration count\n"
                L"Example:\n"
                L"   llrepeatme -repeat=100 netstat -rn \n"
                L"   llrepeatme -repeat=5 cmd /c \"netstat -rn > net_\\%%02d.txt\" \n"
             );
        return -1;
    }

    long maxRuns = 10;
    int argI = 1;
    long seconds = _ttoi(argv[1]);
    while (argv[argI][0] == '-') {
        if (_tcsncmp(argv[argI], L"-wait=", 6) == 0)
            seconds = _ttoi(argv[argI]+6);
        else if (_tcsncmp(argv[argI], L"-repeat=", 8) == 0)
            maxRuns = _ttoi(argv[argI] + 8);
        else {
            fwprintf(stderr, L"Unknown option: %s\n", argv[argI]);
            exit(-1);
        }
        argI++;
    }

    // Merge argv's into one command line, add appropriate quotes.
    TCHAR cmdBuf[4096];
    TCHAR quote[] = _T("\"");
    TCHAR space = ' ';
    _tcscpy_s(cmdBuf, sizeof(cmdBuf)/sizeof(cmdBuf[0]), _T(""));
    for (int argIdx = argI; argIdx < argc; argIdx++)
    {
        TCHAR* arg = argv[argIdx];
        convertSpecialChar(arg);
        if (argIdx != argI)
            _tcscat_s(cmdBuf, sizeof(cmdBuf)/sizeof(cmdBuf[0]), L" ");

        bool addQuote = (_tcschr(arg, space) != NULL) && ( arg[0] != quote[0]);
        if (addQuote)
            _tcscat_s(cmdBuf, sizeof(cmdBuf)/sizeof(cmdBuf[0]), quote);
        _tcscat_s(cmdBuf, sizeof(cmdBuf)/sizeof(cmdBuf[0]), arg);
         if (addQuote)
            _tcscat_s(cmdBuf, sizeof(cmdBuf)/sizeof(cmdBuf[0]), quote);
    }

    // TCHAR procName[4096];
    // ProcName(argv[1], procName, sizeof(procName)/sizeof(procName[0]));
    TCHAR cmdBufWithCnt[4096];

    unsigned idx = 0;
    while (maxRuns-- > 0)
    {
        swprintf_s(cmdBufWithCnt, sizeof(cmdBufWithCnt)/sizeof(cmdBufWithCnt[0]), cmdBuf, idx, idx, idx);
        fwprintf(stderr, L"%s\n", cmdBufWithCnt);
        RunProgram(cmdBufWithCnt);
        idx++;
        if (maxRuns > 0)
            Sleep(seconds * 1000);
    }

    return 0;
}

