#pragma once

#include "cpcSubSystem.h"

namespace CPC {

    class CTape;

    /**
    ** Device that reads and writes data from a magnetic tape/cassette.
    */
    class CTapeDeck : public CSubSystem
    {
    public:

        CTapeDeck(CMachine* machine);
        virtual ~CTapeDeck() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void Reset();

        /** Inserts a new tape, or ejects the current one if nullptr is specified. */
        void SetTape(CTape* tape) { m_tape = tape; }
        /** Returns the tape currently inserted, or nullptr if the tape deck is empty. */
        CTape* GetTape() { return m_tape; }
        /** Returns the tape currently inserted, or nullptr if the tape deck is empty. */
        const CTape* GetTape() const { return m_tape; }

        void SetPlayButtonPressed(bool pressed) { m_playButtonPressed = pressed; }
        bool IsPlayButtonPressed() const { return m_playButtonPressed; }

        void SetMotorOn(bool on) { m_motorOn = on; }
        bool IsMotorOn() const { return m_motorOn; }

        bool IsPlaying() const;

        bool GetDataReadSignal() const;
        void SetDataWriteSignal(bool dataHigh) { /*** TODO ***/ }

        /** Runs for the given number of 1MHz clock cycles. */
        void Run(unsigned numCycles);

    private:

        using inherited = CSubSystem;

        void ResetVars();
        void FreeVars();

        CTape* m_tape;
        bool m_playButtonPressed;
        bool m_motorOn;
    };

} //namespace CPC
