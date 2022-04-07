#include "stdafx.h"
#include "Snapshot.h"

namespace CPC {

    Snapshot::Snapshot()
    {
        Reset();
    }

    void Snapshot::Reset()
    {
        memset(&m_cpuRegisters, 0, sizeof(m_cpuRegisters));
        memset(&m_gateArray, 0, sizeof(m_gateArray));
        memset(&m_crtc, 0, sizeof(m_crtc));
    }

}
