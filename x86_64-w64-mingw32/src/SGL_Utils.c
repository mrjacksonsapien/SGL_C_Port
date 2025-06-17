#include "SGL_Utils.h"

void PrintMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    HANDLE hProcess = GetCurrentProcess();

    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        //printf("Working Set Size (RAM in use): %zu bytes\n", (size_t)pmc.WorkingSetSize);
        printf("Private Bytes (Memory allocated): %zu bytes\n", (size_t)pmc.PrivateUsage);
    } else {
        printf("GetProcessMemoryInfo failed.\n");
    }
}