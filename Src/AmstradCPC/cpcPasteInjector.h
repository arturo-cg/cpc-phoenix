#pragma once

namespace CPC {

    class CMachine;

    class PasteInjector
    {
    public:
        PasteInjector(CMachine* owningMachine);
        ~PasteInjector();

        void Reset();

        // Pastes the given text, where each character is encoded in CPC ASCII.
        // Call ConvertHostToCpcAscii to convert a string encoded in host ASCII to CPC ASCII.
        // Characters will be injected into the emulated machine, one at a time, at the appropriate time.
        void PasteCharacters(const std::deque<cpcByte>& cpcCharacters);
        // Pastes an Enter character (CPC ASCII code 13).
        void PasteEnter();

        // Converts a string encoded in host ASCII to CPC ASCII.
        static std::deque<cpcByte> ConvertHostToCpcAscii(const std::string& hostText);

        // Called by the owning machine. It does all the work of injecting characters at the appropriate time.
        void Run();

    private:
        static constexpr cpcByte CpcAscii_Enter = '\r'/*ASCII 13*/;

        void ResetVars();
        void FreeVars();

        // It injects the given character.
        // it assumes that the CPU is just about to execute the firmware routine KM WAIT CHAR (&BB18).
        void InjectCharacter(cpcByte character);

        bool m_bOk;
        CPC::CMachine* m_machine;

        // Text that is pending injection. Characters are encoded in CPC ASCII.
        std::deque<cpcByte> m_pendingCharacters;
    };

} //namespace CPC
