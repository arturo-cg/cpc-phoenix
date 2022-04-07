#pragma once

#include "cpcCpu.h"
#include "cpcCrtc.h"
#include "cpcGateArray.h"

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
            Unknown,
            Cpc6128Plus,
            Cpc464Plus,
            Gx4000,
        };

        Snapshot();

        void Reset();

        CpcType GetCpcType() const { return m_cpcType; }
        void SetCpcType(CpcType cpcType) { m_cpcType = cpcType; }

        CPC::CCpu::Registers& GetCpuRegisters() { return m_cpuRegisters; }
        const CPC::CCpu::Registers& GetCpuRegisters() const { return m_cpuRegisters; }

        CPC::CGateArray::Snapshot& GetGateArray() { return m_gateArray; }
        const CPC::CGateArray::Snapshot& GetGateArray() const { return m_gateArray; }

        CPC::CCrtc::Snapshot& GetCrtc() { return m_crtc; }
        const CPC::CCrtc::Snapshot& GetCrtc() const { return m_crtc; }

    private:

        CpcType m_cpcType;
        CPC::CCpu::Registers m_cpuRegisters;
        CPC::CGateArray::Snapshot m_gateArray;
        CPC::CCrtc::Snapshot m_crtc;
    };

}
