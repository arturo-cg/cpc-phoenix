//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

/** An archive is a file container.
*   The archive may contain folders too and allow the user to put an entire file system inside the archive.
*   The archive itself is usually stored as a single file.
*/
class kmbArchive
{
public:

    static const unsigned InvalidFileIndex = UINT32_MAX;

    virtual ~kmbArchive() { End(); }

    virtual void End();
    bool IsOk() const { return m_bOk; }

    /** Returns the total number of files in the archive.
*   File indexes go from 0 to GetNumFiles() - 1. */
    virtual unsigned GetNumFiles() const = 0;
    /** Returns the index of the file with the specified name, or InvalidFileIndex if it doesn't exist. */
    virtual unsigned FindFileByName(std::string fileName) = 0;
    /** Returns the name of a file specified by its index in the archive. */
    virtual std::string GetFileName(unsigned fileIndex) = 0;
    /** Returns the uncompressed size in bytes of the specified file. */
    virtual unsigned GetUncompressedFileSize(unsigned fileIndex) = 0;
    /** Extracts a file to a user-allocated memory buffer. Make sure that the memory buffer is at least the size returned by GetUncompressedFileSize. */
    virtual bool ExtractFileToMemory(unsigned fileIndex, void* destBuffer, unsigned destBufferSize) = 0;

protected:

    // Constructor and BaseInit() are protected to avoid instantiating this class.
    kmbArchive() { m_bOk = false; }
    bool BaseInit();

    bool m_bOk;

private:

    void ResetVars();
    void FreeVars();

};
