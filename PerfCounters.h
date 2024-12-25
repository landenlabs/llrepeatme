///
// Author: Dennis Lang 2010
// https://lanenlabs.com
//
    
///  Class to Collect Performance counter stats.
//   Usage:
//      1. Call Initialize() once program run.
//      2. Create one or more PerfCounters(counterName)
//      3. Call Collect() 
//      4. Call GetValue() as often as you want.
//      5. Call Close() once per program run.

#pragma once

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <tchar.h>

class PerfCounters
{
public:

    // Global management. Call Initialize once before use and close when donee.
    // Call collect to prime collection.

    static bool Initialize();
    static bool Collect();
    static void Close();

public:
    // Example Performance Counters
    //
    // \Process(chrome)\\% Processor Time
    // \Process(chrome)\Elapsed Time
    // \Process(chrome)\% User Time
    // \System\Context Switches/sec
    // \System\File Control Bytes/sec
    // \System\File Read Bytes/sec
    // \System\File Write Bytes/sec
    // \System\System Calls/sec

    PerfCounters(TCHAR* counterName);  

    // Return status
    PDH_STATUS GetStatus() const
    {
        return m_status;
    }
    
    // Return TRUE if counter is measured per second
    bool IsPerSec() const { return m_isPerSec; }

    // Get value and return true if valid.
    bool GetValue(double& doubleValue);

    // If any error status, display error message.
    void DisplayErrorStatus(FILE* log);

    static const double m_sNoDouble;

private:
    HCOUNTER    m_counter;
    PDH_STATUS  m_status;
    TCHAR       m_cmd[100];
    bool        m_isPerSec;

    static HQUERY m_sQuery;
};
