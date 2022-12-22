#pragma once

namespace CPC {

    /**
    ** It holds the data of a magnetic tape.
    ** There is a derived class for each supported tape format, such as CDT.
    */
    class CTape
    {
    public:

        virtual ~CTape() { FreeVars(); }

        virtual unsigned GetMarkerCount() const = 0;
        virtual const string& GetMarkerName(unsigned markerIndex) const = 0;
        virtual void SeekToMarker(unsigned markerIndex) = 0;
        virtual unsigned GetCurrentMarker() const = 0;

        /** Rewinds the tape all the way back to the beginning. */
        virtual void Rewind() = 0;

        virtual bool GetOutputPulseLevel() const = 0;

        virtual void AdvanceCycle() = 0;

    protected:

        // Protected constructor
        CTape();

    private:

        void ResetVars();
        void FreeVars();

    };

} //namespace CPC
