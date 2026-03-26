#include "stdafx.h"
#include "cpcCdtTape.h"
#include "Stream/kmbInputStream.h"

namespace CPC {

#pragma pack (push)
#pragma pack (1)

    struct CdtFileHeader
    {
        uint8_t fileIdentifier[8];                 // Should contain "ZXTAPE!" + byte 0x1A.
        uint8_t majorVersionNumber;
        uint8_t minorVersionNumber;
        // The first block comes next, then the second block and so on.
    };

    struct CdtStandardSpeedDataBlock            // ID: 0x10
    {
        uint16_t endPauseDuration;              // Duration in milliseconds of the pause after this block.
        uint16_t dataLength;
        // The data comes next.
    };

    struct CdtTurboLoadingDataBlock             // ID: 0x11
    {
        uint16_t pilotPulseLength;              // Duration of a pulse in the pilot tone.
        uint16_t firstSyncPulseLength;
        uint16_t secondSyncPulseLength;
        uint16_t zeroBitPulseLength;
        uint16_t oneBitPulseLength;
        uint16_t pilotPulseCount;               // How many pulses there are in the pilot tone.
        uint8_t usedBitsInLastByte;             // Number of used bits in last byte (other bits should be 0), i.e. if this is 6 then the bits(x) used in last byte are : xxxxxx00
        uint16_t endPauseDuration;              // Duration in milliseconds of the pause after this block.
        uint8_t dataLength[3];
        // The data comes next.
    };

    struct CdtPureToneBlock                     // ID: 0x12
    {
        uint16_t pulseLength;
        uint16_t pulseCount;
    };

    struct CdtSequenceOfPulsesOfDifferentLengthsBlock   // ID: 0x13
    {
        uint8_t pulseCount;
        // The data comes next.
    };

    struct CdtPureDataBlock                     // ID: 0x14
    {
        uint16_t zeroBitPulseLength;
        uint16_t oneBitPulseLength;
        uint8_t usedBitsInLastByte;             // Number of used bits in last byte (other bits should be 0), i.e. if this is 6 then the bits(x) used in last byte are : xxxxxx00
        uint16_t endPauseDuration;              // Duration in milliseconds of the pause after this block.
        uint8_t dataLength[3];
        // The data comes next.
    };

    struct CdtPauseBlock                        // ID: 0x20
    {
        uint16_t duration;                      // Duration in milliseconds of the pause.
    };

    struct CdtTextBlock                         // IDs: 0x21 (Group Start), 0x30 (Text Description).
    {
        uint8_t textLength;
        // The text in ASCII comes next.
    };

#pragma pack (pop)

#define TIMING_ZX_TO_CPC(x) ((unsigned)(((x) * 4) / 3.5))

    CCdtTape::CCdtTape() : inherited()
    {
        // Reset members
        ResetVars();

        //...
    }

    void CCdtTape::ResetVars()
    {
        m_chunks.clear();
        m_markers.clear();
        m_nextChunk = 0;
        m_nextCycle = 0;
        m_nextPulse = 0;
        m_outputPulseLevel = false;
    }

    void CCdtTape::FreeVars()
    {
        for (ChunkList::iterator iter = m_chunks.begin(); iter != m_chunks.end(); ++iter)
        {
            DestroyChunk(*iter);
        }
    }

    bool CCdtTape::LoadFromStream(kmbInputStream* stream)
    {
        bool ret = false;
        if (stream->IsOk())
        {
            // File header.
            CdtFileHeader fileHeader;
            if (stream->Read(&fileHeader))
            {
                if (strncmp((char*)fileHeader.fileIdentifier, "ZXTape!\x1A", 8/*char_count*/) == 0)
                {
                    AddMarkerAtNextChunk("Beginning of tape");

                    // Blocks.
                    while (true)
                    {
                        if (!LoadBlockFromStream(stream))
                        {
                            ret = stream->IsAtEnd();
                            break;
                        }
                    }
                }
            }
        }

        if (ret)
        {
            // Tape image loaded successfully.
            AddMarkerAtNextChunk("End of tape");
            SeekToChunk(0);
            m_outputPulseLevel = false;
        }

        return ret;
    }

    bool CCdtTape::LoadBlockFromStream(kmbInputStream* stream)
    {
        bool ret = false;

        // Block ID.
        uint8_t blockId;
        if (stream->Read(&blockId))
        {
            switch (blockId)
            {
                //case 0x10:
                //{
                //    CdtStandardSpeedDataBlock block;
                //    if (stream->Read(&block))
                //    {
                //
                //    }
                //    break;
                //}

                case 0x11:
                {
                    CdtTurboLoadingDataBlock block;
                    if (stream->Read(&block))
                    {
                        // Marker.
                        AddMarkerAtNextChunk("Turbo loading data block");
                        // Pilot tone and sync pulses.
                        CreateToneChunk(TIMING_ZX_TO_CPC(block.pilotPulseLength), block.pilotPulseCount & 0xFFFE);    // Hack - Force pulse count to be even so that the pulse level at the end is high.
                        CreateVariableLengthPulsesChunkForSync(TIMING_ZX_TO_CPC(block.firstSyncPulseLength), TIMING_ZX_TO_CPC(block.secondSyncPulseLength));
                        // Data.
                        unsigned byteCount = unsigned(block.dataLength[0]) | (unsigned(block.dataLength[1]) << 8) | (unsigned(block.dataLength[2]) << 16);
                        unsigned bitCount = (byteCount * 8) - (8 - block.usedBitsInLastByte);
                        ret = CreateConstantLengthPulsesChunk(TIMING_ZX_TO_CPC(block.zeroBitPulseLength), TIMING_ZX_TO_CPC(block.oneBitPulseLength), bitCount, stream);
                        // Silence.
                        if (block.endPauseDuration > 0)
                        {
                            CreateSilenceChunk(block.endPauseDuration);
                        }
                    }
                    break;
                }

                case 0x12:
                {
                    CdtPureToneBlock block;
                    if (stream->Read(&block))
                    {
                        AddMarkerAtNextChunk("Pure tone");
                        CreateToneChunk(TIMING_ZX_TO_CPC(block.pulseLength), block.pulseCount);
                        ret = true;
                    }
                    break;
                }

                case 0x13:
                {
                    CdtSequenceOfPulsesOfDifferentLengthsBlock block;
                    if (stream->Read(&block))
                    {
                        AddMarkerAtNextChunk("Sequence of pulses of different lengths");
                        ret = CreateVariableLengthPulsesChunk(block.pulseCount, stream);
                    }
                    break;
                }

                case 0x14:
                {
                    CdtPureDataBlock block;
                    if (stream->Read(&block))
                    {
                        // Data.
                        unsigned byteCount = unsigned(block.dataLength[0]) | (unsigned(block.dataLength[1]) << 8) | (unsigned(block.dataLength[2]) << 16);
                        unsigned bitCount = (byteCount * 8) - (8 - block.usedBitsInLastByte);
                        ret = CreateConstantLengthPulsesChunk(TIMING_ZX_TO_CPC(block.zeroBitPulseLength), TIMING_ZX_TO_CPC(block.oneBitPulseLength), bitCount, stream);
                        // Silence.
                        if (block.endPauseDuration > 0)
                        {
                            CreateSilenceChunk(block.endPauseDuration);
                        }
                    }
                    break;
                }

                case 0x20:
                {
                    CdtPauseBlock block;
                    if (stream->Read(&block))
                    {
                        if (block.duration > 0)
                        {
                            CreateSilenceChunk(block.duration);
                        }
                        ret = true;
                    }
                    break;
                }

                case 0x21:
                {
                    CdtTextBlock block;
                    if (stream->Read(&block))
                    {
                        // For now, just skip the block.
                        // We might want to store these as markers in the future.
                        char buffer[256];
                        ret = stream->Read(buffer, block.textLength);
                        buffer[block.textLength] = '\0';
                    }
                    break;
                }

                case 0x22:
                {
                    // This block has no body.
                    ret = true;
                    break;
                }

                case 0x30:
                {
                    CdtTextBlock block;
                    if (stream->Read(&block))
                    {
                        // For now, just skip the block.
                        // We might want to store these as markers in the future.
                        char buffer[256];
                        ret = stream->Read(buffer, block.textLength);
                        buffer[block.textLength] = '\0';
                    }
                    break;
                }

                default:
                    KMASSERTM(false, ("Unknown block ID 0x%x in CDT stream.", blockId));
                    break;
            }
        }

        return ret;
    }

    bool CPC::CCdtTape::CreateConstantLengthPulsesChunk(unsigned zeroPulseLength, unsigned onePulseLength, unsigned bitCount, kmbInputStream* stream)
    {
        Chunk chunk;
        chunk.type = ChunkType::ConstantLengthPulses;
        chunk.constantLengthPulses.zeroPulseLength = zeroPulseLength;
        chunk.constantLengthPulses.onePulseLength = onePulseLength;
        chunk.constantLengthPulses.bitCount = bitCount;
        unsigned byteCount = (bitCount / 8) + ((bitCount % 8) > 0 ? 1 : 0);
        chunk.constantLengthPulses.bytes = new uint8_t[byteCount];
        m_chunks.push_back(chunk);
        return stream->Read(chunk.constantLengthPulses.bytes, byteCount);
    }

    bool CPC::CCdtTape::CreateVariableLengthPulsesChunk(unsigned pulseCount, kmbInputStream* stream)
    {
        Chunk chunk;
        chunk.type = ChunkType::VariableLengthPulses;
        chunk.variableLengthPulses.pulseCount = pulseCount;
        chunk.variableLengthPulses.pulseLengths = new uint16_t[pulseCount];
        m_chunks.push_back(chunk);
        bool ret = stream->Read(chunk.variableLengthPulses.pulseLengths, pulseCount * 2/*bytes per pulse length*/);
        // Convert pulse timings from the ZX (3.5 MHz) to the CPC (4 MHz).
        uint16_t* pulseLength = chunk.variableLengthPulses.pulseLengths;
        for (unsigned i = 0; i < chunk.variableLengthPulses.pulseCount; i++)
        {
            *pulseLength = TIMING_ZX_TO_CPC(*pulseLength);
            pulseLength++;
        }

        return ret;
    }

    bool CPC::CCdtTape::CreateVariableLengthPulsesChunkForSync(uint16_t firstPulseLength, uint16_t secondPulseLength)
    {
        Chunk chunk;
        chunk.type = ChunkType::VariableLengthPulses;
        chunk.variableLengthPulses.pulseCount = 2;
        chunk.variableLengthPulses.pulseLengths = new uint16_t[2];
        chunk.variableLengthPulses.pulseLengths[0] = firstPulseLength;
        chunk.variableLengthPulses.pulseLengths[1] = secondPulseLength;
        m_chunks.push_back(chunk);
        return false;
    }

    void CPC::CCdtTape::CreateToneChunk(unsigned pulseLength, unsigned pulseCount)
    {
        Chunk chunk;
        chunk.type = ChunkType::Tone;
        chunk.tone.pulseLength = pulseLength;
        chunk.tone.pulseCount = pulseCount;
        m_chunks.push_back(chunk);
    }

    void CPC::CCdtTape::CreateSilenceChunk(unsigned duration)
    {
        Chunk chunk;
        chunk.type = ChunkType::Silence;
        chunk.silence.cycleCount = duration/*milliseconds*/ * 4000/*cycles/milliseconds*/;
        m_chunks.push_back(chunk);
    }

    bool CPC::CCdtTape::DestroyChunk(Chunk& chunk)
    {
        switch (chunk.type)
        {
            case ChunkType::ConstantLengthPulses:
            {
                delete[] chunk.constantLengthPulses.bytes;
                chunk.constantLengthPulses.bytes = nullptr;
                break;
            }

            case ChunkType::VariableLengthPulses:
            {
                delete[] chunk.variableLengthPulses.pulseLengths;
                chunk.variableLengthPulses.pulseLengths = nullptr;
                break;
            }

            case ChunkType::Tone:
            {
                break;
            }

            case ChunkType::Silence:
            {
                break;
            }

            default:
                KMASSERTM(false, ("Unhandled case."));
                break;
        }
        return false;
    }

    void CCdtTape::AddMarkerAtNextChunk(std::string name)
    {
        char numberAndName[100];
        snprintf(numberAndName, sizeof(numberAndName), "%zu - %s", m_markers.size(), name.c_str());

        Marker marker;
        marker.name = numberAndName;
        marker.chunkIndex = static_cast<unsigned>(m_chunks.size());
        m_markers.push_back(marker);
    }

    unsigned CCdtTape::GetMarkerCount() const
    {
        return static_cast<unsigned>(m_markers.size());
    }

    const std::string& CCdtTape::GetMarkerName(unsigned markerIndex) const
    {
        KMASSERT(markerIndex < m_markers.size());
        return m_markers[markerIndex].name;
    }

    void CCdtTape::SeekToMarker(unsigned markerIndex)
    {
        KMASSERT(markerIndex < m_markers.size());
        SeekToChunk(m_markers[markerIndex].chunkIndex);
    }

    unsigned CPC::CCdtTape::FindMarkerByChunk(unsigned chunkIndex) const
    {
        return FindMarkerByChunk_Rec(chunkIndex, 0, static_cast<unsigned>(m_markers.size()));
    }

    unsigned CPC::CCdtTape::FindMarkerByChunk_Rec(unsigned chunkIndex, unsigned fromMarker, unsigned count) const
    {
        unsigned ret = -1;
        if (count <= 1)
        {
            ret = fromMarker;
        }
        else
        {
            // Binary search.
            unsigned halfCount = count / 2;
            unsigned center = fromMarker + halfCount;
            if (chunkIndex < m_markers[center].chunkIndex)
            {
                ret = FindMarkerByChunk_Rec(chunkIndex, fromMarker, halfCount);
            }
            else
            {
                ret = FindMarkerByChunk_Rec(chunkIndex, center, halfCount);
            }
        }

        return ret;
    }

    void CPC::CCdtTape::Rewind()
    {
        SeekToChunk(0);
    }

    void CPC::CCdtTape::AdvanceCycle()
    {
        if (m_nextChunk < m_chunks.size())
        {
            const Chunk& chunk = m_chunks[m_nextChunk];
            switch (chunk.type)
            {
                case ChunkType::ConstantLengthPulses:
                {
                    // m_nextPulse - Current bit.
                    // m_nextCycle - Cycle in current bit.
                    uint8_t byte = chunk.constantLengthPulses.bytes[m_nextPulse / 8];
                    bool bit = (byte & (0x80 >> (m_nextPulse % 8))) != 0;
                    unsigned pulseLength = (bit ? chunk.constantLengthPulses.onePulseLength : chunk.constantLengthPulses.zeroPulseLength);
                    m_outputPulseLevel = (m_nextCycle < pulseLength ? false : true);    // First pulse low, second pulse high: ___---
                    // Next cycle.
                    m_nextCycle++;
                    if (m_nextCycle >= (pulseLength * 2))
                    {
                        // Next bit.
                        m_nextCycle = 0;
                        m_nextPulse++;
                        if (m_nextPulse >= chunk.constantLengthPulses.bitCount)
                        {
                            // Next chunk.
                            SeekToNextChunk();
                        }
                    }

                    break;
                }

                case ChunkType::VariableLengthPulses:
                {
                    // m_nextPulse - Current pulse.
                    // m_nextCycle - Cycle in current pulse.
                    m_outputPulseLevel = ((m_nextPulse % 2) == 0) ? false : true;    // Low on even pulses, high on odd pulses: ___---
                    m_nextCycle++;
                    if (m_nextCycle == chunk.variableLengthPulses.pulseLengths[m_nextPulse])
                    {
                        // Next pulse.
                        m_nextCycle = 0;
                        m_nextPulse++;
                        if (m_nextPulse == chunk.variableLengthPulses.pulseCount)
                        {
                            // Next chunk.
                            SeekToNextChunk();
                        }
                    }
                    break;
                }

                case ChunkType::Tone:
                {
                    m_outputPulseLevel = ((m_nextPulse % 2) == 0) ? false : true;    // Low on even pulses, high on odd pulses: ___---
                    m_nextCycle++;
                    if (m_nextCycle >= chunk.tone.pulseLength)
                    {
                        // Next pulse.
                        m_nextCycle = 0;
                        m_nextPulse++;
                        if (m_nextPulse == chunk.tone.pulseCount)
                        {
                            // Next chunk.
                            SeekToNextChunk();
                        }
                    }
                    break;
                }

                case ChunkType::Silence:
                {
                    m_outputPulseLevel = false;
                    m_nextCycle++;
                    if (m_nextCycle >= chunk.silence.cycleCount)
                    {
                        // Next chunk.
                        SeekToNextChunk();
                    }
                    break;
                }

                default:
                {
                    m_outputPulseLevel = false;
                    SeekToNextChunk();
                    KMASSERTM(false, ("Unhandled case."));
                    break;
                }
            }
        }
        else
        {
            // The tape is at the end. Just output silence.
            m_outputPulseLevel = false;
        }
    }

    void CPC::CCdtTape::SeekToChunk(unsigned chunkIndex)
    {
        m_nextChunk = (chunkIndex < m_chunks.size() ? chunkIndex : static_cast<unsigned>(m_chunks.size()));    // Note that it allows m_nextChunk to go past the last chunk index, which means it is at the end of the tape.
        m_nextCycle = 0;
        m_nextPulse = 0;
        m_nextMarker = FindMarkerByChunk(m_nextChunk);
    }

    void CPC::CCdtTape::SeekToNextChunk()
    {
        SeekToChunk(m_nextChunk + 1);
    }

} //namespace CPC
