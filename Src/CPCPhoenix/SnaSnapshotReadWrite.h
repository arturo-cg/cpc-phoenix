#pragma once

class kmbInputStream;
//////class kmbOutputStream;

namespace CPC
{
    class Snapshot;
}

/** Loads and saves snapshot data in .SNA format.
    When loading, data is read from a stream and stored in a Snapshot object. When writing, data is taken from a Snapshot object and written to a stream.
    The .SNA format is the standard snapshot format in the CPC community. It was first developed for the CPCEMU emulator. */
class SnaSnapshotReadWrite
{
public:

    /** Reads a snapshot in .SNA format from the specified stream. */
    static bool LoadSnapshot(kmbInputStream& inputStream, CPC::Snapshot* outputSnapshot);

    ///////** Writes snapshot data contained in the specified Snapshot object to the specified stream. */
    //////static bool SaveSnapshotHeader(const Snapshot* inputSnapshot, kmbOutputStream* outputStream);
};
