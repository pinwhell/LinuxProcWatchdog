#include <iostream>
#include "clipp.h"
#include "crc.h"
#include "LinuxProcess/LinuxProcess.h"
#include "LinuxProcess/ElfUtils.h"
#include <unordered_map>
#include <functional>
#include <unordered_set>
#include <chrono>
#include <thread>

struct LibInfoCache {
    uintptr_t mDotTextOffset;
    size_t mDotTextSize;
    uint32_t mDotTextCrc;
};

std::unordered_map< std::string, LibInfoCache > libInfoCaches;

bool ElfLibRegisterCache(LinuxProcess* pProcess, const std::string& libName)
{
    std::string fullLibPath = "";

    if(pProcess->GetFullModulePath(libName.c_str(), fullLibPath) == false)
        return false;

    // At this point, we have extracted the path of the library

    bool libMappedProperly = ElfOpen(fullLibPath, [&](ElfPack libMap){
        // at this point, we sucessfully mapped the lib

        Elf32_Shdr* pDotTextSection = ElfLookupSectionByName(libMap, ".text");

        if(pDotTextSection == nullptr)
            return;

        // At this point we had found the .text .text section
        // Lets simply cache this info, for optimization

        libInfoCaches[libName].mDotTextOffset = pDotTextSection->sh_offset;
        libInfoCaches[libName].mDotTextSize = pDotTextSection->sh_size;
        libInfoCaches[libName].mDotTextCrc = calculate_crc32((const void*)(libMap.base + pDotTextSection->sh_offset), pDotTextSection->sh_size);
    });

    if(libMappedProperly == false)
        return false;

    return libInfoCaches.find(libName) != libInfoCaches.end();
}

uint32_t ElfDotTextSectionHash(LinuxProcess* pProcess, uint64_t libEntry, const std::string& libName, uint32_t* originalDotTextHash = nullptr)
{     
    if(
        libInfoCaches.find(libName) == libInfoCaches.end() &&
        ElfLibRegisterCache(pProcess, libName) == false
    )
        return 0x0;

    if(originalDotTextHash)
        *originalDotTextHash = libInfoCaches[libName].mDotTextCrc;


    // At this point, we alredy got a lib cache
    // Ready to deal with hashing the .text section

    void* tmpDotText = malloc(libInfoCaches[libName].mDotTextSize + 1);

    if(tmpDotText == nullptr)
        return 0x0;

    // at this point we have a buffer with enougth space to put the .text

    pProcess->ReadMemory(libEntry + libInfoCaches[libName].mDotTextOffset, tmpDotText, libInfoCaches[libName].mDotTextSize);

    // At  this point we should have the .text section at the tmpDotText buffer
    // Lets hash it & return it

    uint32_t resultHash = calculate_crc32(tmpDotText, libInfoCaches[libName].mDotTextSize);

    free(tmpDotText);

    return resultHash;
}

int ProcInstrussionWatcherRun(
    LinuxProcess* pTargetProcess,
    const std::vector<std::string>& libs, 
    uint32_t interval,
    std::function<void(
        const std::string& libName,
        uint32_t prevCrc,
        uint32_t newCrc
    )> onDetectionChangedCallback
    )
{
    std::unordered_map<std::string, uint32_t> dotCodeSectionHashs;

    while(true)
    {
        // Must Implement
        // if(pTargetProcess->isOpen() == false)
        //     break;

        // App is running

        for(const std::string& libName : libs)
        {
            uintptr_t currLibEntry = pTargetProcess->GetModBaseAddr(libName.c_str());

            if(currLibEntry == 0x0)
                continue;

            // Good Lib Found

            uint32_t outOrigDotTextHash = 0x0;

            uint32_t libCodeSectionHash = ElfDotTextSectionHash(pTargetProcess, currLibEntry, libName, &outOrigDotTextHash);
            
            if(libCodeSectionHash == 0x0)
                continue;

            // Proper hash found for curr Lib .text section

            if(dotCodeSectionHashs.find(libName) == dotCodeSectionHashs.end())
            {
                // Looks like its the first time obtaining a hash for this lib
                // lets simply put the original hash

                dotCodeSectionHashs[libName] = outOrigDotTextHash;
            }

            // Seems we alredy got a hash for the .text section of the lib
            // Lets compare previeous & new one to see if any has changed

            if(dotCodeSectionHashs[libName] == libCodeSectionHash)
                continue;    

            // Seems we catch a change, lets report it

            onDetectionChangedCallback(libName, dotCodeSectionHashs[libName], libCodeSectionHash);

            // Lets now update the actual hash

            dotCodeSectionHashs[libName] = libCodeSectionHash;
        }
        
        if(interval < 1)
            continue;

        // Seems we have a time to sleep
        // Lets perform te sleeping

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }

    return 0;
}

void PrintJsonResult(const std::string& libName, uint32_t oldCrc, uint32_t newCrc)
{
    std::cout << "{\n";
    std::cout << "  \"DetectedChange\": {\n";
    std::cout << "    \"LibraryName\": \"" << libName << "\",\n";
    std::cout << "    \"OldCRC\": " << std::dec << oldCrc << ",\n";
    std::cout << "    \"NewCRC\": " << std::dec << newCrc << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";

    // Lets signal end of the JSON
    // With Empty new line

    std::cout << "\n"; 
}

int main(int argc, char** argv)
{
    init_crc32_table();

    std::string targetName;
    std::vector<std::string> libs;
    uint32_t interval = 500;

    auto cli = (
        clipp::option("--target-name") & clipp::value("process name to watch on", targetName).required(true),
        clipp::option("--libs") & clipp::values("libs to watch on", libs).required(true),
        clipp::option("--interval") & clipp::value("Interval in milliseconds at which rate to run the checks", interval)
        );
        
    auto results = clipp::parse(argc, argv, cli);

    if (!results || targetName.empty() == true || libs.empty() == true)
    {
        std::cout << clipp::make_man_page(cli) << std::endl;
        return 1;
    }

    std::unique_ptr<LinuxProcess> targetProc;

    try {
        targetProc = std::make_unique<LinuxProcess>(targetName.c_str());
    } catch (const char* what)
    {
        printf("Error: %s\n", what);

        return 1;
    }

    return ProcInstrussionWatcherRun(targetProc.get(), libs, interval, [](const std::string& libName, uint32_t prevCrc, uint32_t newCrc){
        PrintJsonResult(libName.c_str(), prevCrc, newCrc);
    });
}