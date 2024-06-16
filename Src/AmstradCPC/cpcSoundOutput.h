//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "cpcSubSystem.h"

namespace CPC {

    /**
    ** This class provides sound output functionality to the emulator. It is a pure virtual class. The front-end must implement
    ** a class derived from this one and register it via the method CMachine::SetSoundOutput.
    */
    class CSoundOutput
    {
    public:
        enum class OutputChannelCount : uint8_t
        {
            Mono,
            Stereo,
        };

        CSoundOutput();
        virtual ~CSoundOutput() { FreeVars(); }

        /** Called by the emulator when the emulated machine is reset. */
        virtual void Reset() = 0;

        /** Returns the machine this subsystem is part of. */
        CMachine* GetMachine() { return m_machine; }
        const CMachine* GetMachine() const { return m_machine; }
        void SetMachine(CMachine* machine) { m_machine = machine; }

        /** Enables or disables (aka mutes) the specified channel. */
        void SetChannelEnabled(int channelIndex, bool enabled);
        bool IsChannelEnabled(int channelIndex) const;

        /** Updates the sound output for the given number of cycles. */
        virtual void Run(unsigned numCycles) = 0;

    private:

        void ResetVars();
        void FreeVars();

        CMachine* m_machine;
        bool m_channelEnabled[3];      // 0 = channel A, 1 = channel B, 2 = channel C.
    };

} //namespace CPC
