#include "stdafx.h"
#include "Snapshot.h"

namespace CPC {

    Snapshot::Snapshot()
    {
        ResetVars();
    }

    Snapshot::~Snapshot()
    {
        FreeVars();
    }

    void Snapshot::Reset()
    {
        FreeVars();
        ResetVars();
    }

    void Snapshot::ResetVars()
    {
        memset(&m_cpuRegisters, 0, sizeof(m_cpuRegisters));
        memset(&m_gateArray, 0, sizeof(m_gateArray));
        memset(&m_crtc, 0, sizeof(m_crtc));
        memset(&m_psg, 0, sizeof(m_psg));

        for (int i = 0; i < MaxRamPageCount; i++)
        {
            m_ramPages[i] = nullptr;
        }
    }

    void Snapshot::FreeVars()
    {
        for (int i = 0; i < MaxRamPageCount; i++)
        {
            if (m_ramPages[i] != nullptr)
            {
                delete[] m_ramPages[i];
                m_ramPages[i] = nullptr;
            }
        }
    }

    cpcByte* Snapshot::CreateRamPageIfNecessary(int index)
    {
        cpcByte* ret = nullptr;

        if (index < MaxRamPageCount)
        {
            // Create it if it doesn't exist yet.
            if (m_ramPages[index] == nullptr)
            {
                m_ramPages[index] = new cpcByte[RamPageSize];
            }

            ret = m_ramPages[index];
        }

        return ret;
    }
}
