#include "stdafx.h"
#include "SnaSnapshotReadWrite.h"
#include "Snapshot.h"
#include "Stream/kmbInputStream.h"

static const char* SnaHeaderIdentification = "MV - SNA";

struct Header
{
    // All versions.
    char identification[8];                 // "MV - SNA"
    uint8_t unused1[8];
    uint8_t version;                        // Supported versions: 1, 2 and 3.
    uint8_t cpuF;
    uint8_t cpuA;
    uint8_t cpuC;
    uint8_t cpuB;
    uint8_t cpuE;
    uint8_t cpuD;
    uint8_t cpuL;
    uint8_t cpuH;
    uint8_t cpuR;
    uint8_t cpuI;
    uint8_t cpuIFF0;
    uint8_t cpuIFF1;
    uint8_t cpuIXLow;
    uint8_t cpuIXHigh;
    uint8_t cpuIYLow;
    uint8_t cpuIYHigh;
    uint8_t cpuSPLow;
    uint8_t cpuSPHigh;
    uint8_t cpuPCLow;
    uint8_t cpuPCHigh;
    uint8_t cpuIM;
    uint8_t cpuAltF;
    uint8_t cpuAltA;
    uint8_t cpuAltC;
    uint8_t cpuAltB;
    uint8_t cpuAltE;
    uint8_t cpuAltD;
    uint8_t cpuAltL;
    uint8_t cpuAltH;
    uint8_t gateArraySelectedPen;
    uint8_t gateArrayPenColors[16];
    uint8_t gateArrayBorderColor;
    uint8_t gateArrayMultiConfiguration;
    uint8_t gateArrayRamConfiguration;
    uint8_t crtcSelectedRegister;
    uint8_t crtcRegisters[18];
    uint8_t gateArrayRomSelection;
    uint8_t ppiPortA;
    uint8_t ppiPortB;
    uint8_t ppiPortC;
    uint8_t ppiControlPort;
    uint8_t psgSelectedRegister;
    uint8_t psgRegisters[16];
    uint8_t memSizeInKilobytes;             // E.g. 64 for 64K, 128 for 128K, of uncompressed memory. In .SNA version 3, it can also be 0, in which case there will be MEM chunks of compressed memory later on in the snapshot.

    // Version 2 and higher.



    // Version 3.



    uint8_t unused2[93 + 55];
};

bool SnaSnapshotReadWrite::LoadSnapshot(kmbInputStream& inputStream, CPC::Snapshot* outputSnapshot)
{
    bool ret = false;

    KMASSERTM(sizeof(Header) == 256, (".SNA snapshot header definition is wrong. It should be exactly 256 bytes long."));
    KMASSERT(outputSnapshot != nullptr);

    // Reset all Snapshot values to zero.
    outputSnapshot->Reset();

    if (inputStream.IsOk())
    {
        // Read the header.
        Header header;
        if (inputStream.Read(&header))
        {
            if (strncmp(header.identification, SnaHeaderIdentification, sizeof(header.identification)) == 0)       // If header identification is "MV - SNA"...
            {
                CPC::CCpu::Registers* cpuRegisters = &outputSnapshot->GetCpuRegisters();
                CPC::CGateArray::Snapshot* gateArray = &outputSnapshot->GetGateArray();
                CPC::CCrtc::Snapshot* crtc = &outputSnapshot->GetCrtc();

                // All versions.
                outputSnapshot->SetCpcType(header.memSizeInKilobytes == 64 ? CPC::Snapshot::CpcType::Cpc464 : CPC::Snapshot::CpcType::Cpc6128);   // Overwritten later on if .SNA version is 2 or higher.
                cpuRegisters->F() = header.cpuF;
                cpuRegisters->A() = header.cpuA;
                cpuRegisters->C() = header.cpuC;
                cpuRegisters->B() = header.cpuB;
                cpuRegisters->E() = header.cpuE;
                cpuRegisters->D() = header.cpuD;
                cpuRegisters->L() = header.cpuL;
                cpuRegisters->H() = header.cpuH;
                cpuRegisters->R() = header.cpuR;
                cpuRegisters->I() = header.cpuI;
                cpuRegisters->IFF1 = header.cpuIFF0;
                cpuRegisters->IFF2 = header.cpuIFF1;
                cpuRegisters->IXL() = header.cpuIXLow;
                cpuRegisters->IXH() = header.cpuIXHigh;
                cpuRegisters->IYL() = header.cpuIYLow;
                cpuRegisters->IYH() = header.cpuIYHigh;
                cpuRegisters->SP.b.l = header.cpuSPLow;
                cpuRegisters->SP.b.h = header.cpuSPHigh;
                cpuRegisters->PC.b.l = header.cpuPCLow;
                cpuRegisters->PC.b.h = header.cpuPCHigh;
                cpuRegisters->IM = int(header.cpuIM);
                cpuRegisters->altF() = header.cpuAltF;
                cpuRegisters->altA() = header.cpuAltA;
                cpuRegisters->altC() = header.cpuAltC;
                cpuRegisters->altB() = header.cpuAltB;
                cpuRegisters->altE() = header.cpuAltE;
                cpuRegisters->altD() = header.cpuAltD;
                cpuRegisters->altL() = header.cpuAltL;
                cpuRegisters->altH() = header.cpuAltH;
                gateArray->selectedPen = header.gateArraySelectedPen;
                std::copy(std::begin(header.gateArrayPenColors), std::end(header.gateArrayPenColors), std::begin(gateArray->penColors));
                gateArray->borderColor = header.gateArrayBorderColor;
                gateArray->screenMode = CPC::CGateArray::EScreenMode(header.gateArrayMultiConfiguration & 0x03);
                gateArray->lowerRomVisible = (header.gateArrayMultiConfiguration & 0x04) == 0;
                gateArray->upperRomVisible = (header.gateArrayMultiConfiguration & 0x08) == 0;
                gateArray->interruptControlState = (header.gateArrayMultiConfiguration & 0x10) != 0;
                gateArray->ramConfig = CPC::CGateArray::ERamConfig(header.gateArrayRamConfiguration & 0x07);
                gateArray->secondaryRamPage = ((header.gateArrayRamConfiguration & 0x38) >> 3) + 1;
                gateArray->selectedUpperRom = header.gateArrayRomSelection;
                crtc->selectedRegister = CPC::CCrtc::ERegister(header.crtcSelectedRegister);
                std::copy(std::begin(header.crtcRegisters), std::end(header.crtcRegisters), std::begin(crtc->registers));

                // Version 2 or higher.
                if (header.version >= 2)
                {
//                    outputSnapshot->SetCpcType(Snapshot::CpcType(header.cpcType));
                }

                // Version 3 or higher.
                if (header.version >= 3)
                {
                }

                // Read uncompressed RAM, if it exists.
                // Note: The stream is now positioned right after the header, which is 256 bytes long, at the start of the RAM dump (if it exists).
                if (header.memSizeInKilobytes >= 64)    // Base 64 KB of memory.
                {
                    cpcByte* ramPage = outputSnapshot->CreateRamPageIfNecessary(0);
                    inputStream.Read(ramPage, CPC::Snapshot::RamPageSize);
                }

                if (header.memSizeInKilobytes >= 128)    // Additional 64 KB of memory in the CPC 6128.
                {
                    cpcByte* ramPage = outputSnapshot->CreateRamPageIfNecessary(1);
                    inputStream.Read(ramPage, CPC::Snapshot::RamPageSize);
                }

                // Read chunks (version 3 or higher).
                if (header.version >= 3)
                {
                }

                ret = true;
            }
        }
    }

    return ret;
}
