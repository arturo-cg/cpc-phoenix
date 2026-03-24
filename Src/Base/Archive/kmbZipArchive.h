//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "kmbArchive.h"
#include "miniz/miniz.h"

class kmbInputStream;

/**
**
*/
class kmbZipArchive : public kmbArchive
{
public:

    kmbZipArchive() { }
    virtual ~kmbZipArchive() { End(false); }

    /* IMPORTANT: 'inputStream' must remain valid during the lifetime of this kmbZipArchive. */
    bool Init(kmbInputStream& inputStream);
    virtual void End(bool bIncludedSuper = true);

    /** Returns the total number of files in the archive.
    *   File indexes go from 0 to GetNumFiles() - 1. */
    virtual unsigned GetNumFiles() const;
    /** Returns the index of the file with the specified name, or InvalidFileIndex if it doesn't exist. */
    virtual unsigned FindFileByName(std::string fileName);
    /** Returns the name of a file specified by its index in the archive. */
    virtual std::string GetFileName(unsigned fileIndex);
    /** Returns the uncompressed size in bytes of the specified file. */
    virtual unsigned GetUncompressedFileSize(unsigned fileIndex);
    /** Extracts a file to a user-allocated memory buffer. Make sure that the memory buffer is at least the size returned by GetUncompressedFileSize. */
    virtual bool ExtractFileToMemory(unsigned fileIndex, void* destBuffer, unsigned destBufferSize);

private:

    typedef kmbArchive Super;

    void ResetVars();
    void FreeVars();

    mz_zip_archive m_mzZipArchive;
};
