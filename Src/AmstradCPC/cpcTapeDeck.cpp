#include "stdafx.h"
#include "cpcTapeDeck.h"
#include "cpcTape.h"

namespace CPC {

    CTapeDeck::CTapeDeck(CMachine* machine) : inherited(machine)
    {
        // Reset members
        ResetVars();

        //...
    }

    void CTapeDeck::ResetVars()
    {
        m_tape = nullptr;
        m_playButtonPressed = false;
        m_motorOn = false;
    }

    void CTapeDeck::FreeVars()
    {
        //...
    }

    /*virtual*/ void CTapeDeck::Reset()
    {
        ResetVars();
    }

    void CTapeDeck::Run(unsigned numCycles)
    {
        if (IsPlaying())
        {
            m_tape->AdvanceCycle();
        }
    }

    bool CPC::CTapeDeck::IsPlaying() const
    {
        return m_playButtonPressed && m_motorOn;
    }

    bool CPC::CTapeDeck::GetDataReadSignal() const
    {
        return IsPlaying() && (m_tape != nullptr) ? m_tape->GetOutputPulseLevel() : false;
    }

} //namespace CPC
