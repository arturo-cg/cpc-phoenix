#pragma once

#include "cpcTape.h"

class kmbInputStream;

namespace CPC {

    /**
    ** A CTape-derived class that can read tape images in CDT/TZX format.
    ** CDT and TZX formats are exactly the same, only the file extension changes.
    ** CDT stands for "CPC Digital Tape".
    */
    class CCdtTape : public CTape
    {
    public:

        CCdtTape();
        virtual ~CCdtTape() { FreeVars(); }

        /** Loads data from the given tape image. */
        bool LoadFromStream(kmbInputStream* stream);

        virtual unsigned GetMarkerCount() const override;
        virtual const std::string& GetMarkerName(unsigned markerIndex) const override;
        virtual void SeekToMarker(unsigned markerIndex) override;
        virtual unsigned GetCurrentMarker() const override { return m_nextMarker; }

        virtual void Rewind() override;

        virtual bool GetOutputPulseLevel() const override { return m_outputPulseLevel; }

        virtual void AdvanceCycle() override;

    private:

        using inherited = CTape;

        enum class ChunkType
        {
            ConstantLengthPulses,           // Pulse data in which bits 0 and 1 always have the same length (each bit pulse its own length).
            VariableLengthPulses,           // Pulse data in which each bit pulse has its own length. Usually used by protection schemes.
            Tone,                           // Tone with a given constant pulse length.
            Silence,                        // Low pulse level for the entire chunk.
        };

        struct Chunk
        {
            ChunkType type;

            union
            {
                struct
                {
                    unsigned zeroPulseLength;   // Length of bit 0 pulse, in 4-MHz clock ticks.
                    unsigned onePulseLength;    // Length of bit 1 pulse, in 4-MHz clock ticks.
                    unsigned bitCount;
                    uint8_t* bytes;
                } constantLengthPulses;

                struct
                {
                    unsigned pulseCount;
                    uint16_t* pulseLengths;
                } variableLengthPulses;

                struct
                {
                    unsigned pulseLength;       // In 4-MHz clock ticks.
                    unsigned pulseCount;
                } tone;

                struct
                {
                    unsigned cycleCount;
                } silence;
            };
        };

        struct Marker
        {
            std::string name;
            unsigned chunkIndex;                // The chunk that is referenced by this marker.
        };

        using ChunkList = std::vector<Chunk>;
        using MarkerList = std::vector<Marker>;

        void ResetVars();
        void FreeVars();

        bool LoadBlockFromStream(kmbInputStream* stream);

        bool CreateConstantLengthPulsesChunk(unsigned zeroPulseLength, unsigned onePulseLength, unsigned bitCount, kmbInputStream* stream);
        bool CreateVariableLengthPulsesChunk(unsigned pulseCount, kmbInputStream* stream);
        bool CreateVariableLengthPulsesChunkForSync(uint16_t firstPulseLength, uint16_t secondPulseLength);
        void CreateToneChunk(unsigned pulseLength, unsigned pulseCount);
        void CreateSilenceChunk(unsigned duration);
        bool DestroyChunk(Chunk& chunk);

        void AddMarkerAtNextChunk(std::string name);
        unsigned FindMarkerByChunk(unsigned chunkIndex) const;
        unsigned FindMarkerByChunk_Rec(unsigned chunkIndex, unsigned fromMarker, unsigned count) const;

        void SeekToChunk(unsigned chunkIndex);
        void SeekToNextChunk();
        bool GetPulseLevel(unsigned chunkIndex, unsigned cycle) const;

        ChunkList m_chunks;
        MarkerList m_markers;

        unsigned m_nextChunk;
        unsigned m_nextCycle;         // Relative to the current chunk or current pulse, depending on the chunk type.
        unsigned m_nextPulse;         // Relative to the current chunk.
        unsigned m_nextMarker;
        bool m_outputPulseLevel;
    };

} //namespace CPC
