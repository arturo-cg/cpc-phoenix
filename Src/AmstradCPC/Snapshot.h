#pragma once

#include "cpcCpu.h"
#include "cpcCrtc.h"
#include "cpcGateArray.h"
#include "cpcPsg.h"

namespace CPC {

    /** A snapshot (a.k.a. save state) represents the state of the machine at a given point in time.
        Applying a snapshot to an emulated machine brings the machine back to the state it was in when the snapshot was taken. */
    class Snapshot
    {
    public:

        enum class CpcType
        {
            Cpc464 = 0,
            Cpc664,
            Cpc6128,
            Cpc464Plus,
            Cpc6128Plus,
            Gx4000,
            Unknown,
        };

        static const int MaxRamPageCount = 9;       // RAM page index can go from 0 to 8.
        static const int RamPageSize = 64 * 1024;   // 64 KB.

        Snapshot();
        ~Snapshot();

        void Reset();

        CpcType GetCpcType() const { return m_cpcType; }
        void SetCpcType(CpcType cpcType) { m_cpcType = cpcType; }

        CPC::CCpu::Registers& GetCpuRegisters() { return m_cpuRegisters; }
        const CPC::CCpu::Registers& GetCpuRegisters() const { return m_cpuRegisters; }

        CPC::CGateArray::Snapshot& GetGateArray() { return m_gateArray; }
        const CPC::CGateArray::Snapshot& GetGateArray() const { return m_gateArray; }

        CPC::CCrtc::Snapshot& GetCrtc() { return m_crtc; }
        const CPC::CCrtc::Snapshot& GetCrtc() const { return m_crtc; }

        CPC::CPsg::Snapshot& GetPsg() { return m_psg; }
        const CPC::CPsg::Snapshot& GetPsg() const { return m_psg; }

        /** Gets the RAM page (64 KB of memory) at the specified index, or nullptr if this RAM page does not exist.
            Base 64 KB is at index 0, CPC 6128's additional 64 KB is at index 1 (if it exists). */
        const cpcByte* GetRamPage(int index) const { return (index < MaxRamPageCount ? m_ramPages[index] : nullptr); }
        /** For Snapshot importers only. */
        cpcByte* CreateRamPageIfNecessary(int index);

    private:

        void ResetVars();
        void FreeVars();

        CpcType m_cpcType;
        CPC::CCpu::Registers m_cpuRegisters;
        CPC::CGateArray::Snapshot m_gateArray;
        CPC::CCrtc::Snapshot m_crtc;
        CPC::CPsg::Snapshot m_psg;
        cpcByte* m_ramPages[MaxRamPageCount];       // RAM pages 0 to 8. Each RAM page is 64KB. Page 0 is the base 64KB of memory, page 1 is the additional 64KB in the CPC 6128. Page n is nullptr if it doesn't exist.
    };

}
