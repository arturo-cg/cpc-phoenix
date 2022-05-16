//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbZipArchive.h"
#include "Stream/kmbInputStream.h"

size_t ReadFromInputStream(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n);

bool kmbZipArchive::Init(kmbInputStream& inputStream)
{
    bool bRet = true;

    End(false);
    ResetVars();

    // Initialize base class
    if (bRet)
    {
        bRet = Super::BaseInit();
    }

    // Check parameters
    if (bRet)
    {
        bRet = inputStream.IsOk();
    }

    // Initialize class members
    if (bRet)
    {
        m_mzZipArchive.m_pIO_opaque = &inputStream;
        m_mzZipArchive.m_pRead = ReadFromInputStream;
        bRet = mz_zip_reader_init(&m_mzZipArchive, inputStream.GetLength(), 0/*flags*/);
    }

    if (!bRet)
    {
        FreeVars();
        m_bOk = false;
    }

    return bRet;
}

/*virtual*/ void kmbZipArchive::End(bool bIncludedSuper/* = true*/)
{
    if (IsOk())
    {
        FreeVars();

        if (bIncludedSuper)
        {
            Super::End();
        }
    }
}

void kmbZipArchive::ResetVars()
{
    mz_zip_zero_struct(&m_mzZipArchive);
}

void kmbZipArchive::FreeVars()
{
    mz_zip_reader_end(&m_mzZipArchive);
}

unsigned kmbZipArchive::GetNumFiles() const
{
    // Don't use mz_zip_reader_get_num_files function as it is not const.
    return m_mzZipArchive.m_total_files;
}

unsigned kmbZipArchive::FindFileByName(string fileName)
{
    unsigned ret;
    if (!mz_zip_reader_locate_file_v2(&m_mzZipArchive, fileName.c_str(), nullptr, 0/*flags*/, &ret))
    {
        ret = InvalidFileIndex;
    }

    return ret;
}

string kmbZipArchive::GetFileName(unsigned fileIndex)
{
    char buffer[1000];
    mz_zip_reader_get_filename(&m_mzZipArchive, fileIndex, buffer, sizeof(buffer));

    return string(buffer);
}

unsigned kmbZipArchive::GetUncompressedFileSize(unsigned fileIndex)
{
    mz_zip_archive_file_stat fileStat;
    if (mz_zip_reader_file_stat(&m_mzZipArchive, fileIndex, &fileStat))
    {
        return unsigned(fileStat.m_uncomp_size);        // Warning, file size truncated to a 32-bit number...
    }
    else
    {
        return 0;
    }
}

bool kmbZipArchive::ExtractFileToMemory(unsigned fileIndex, void* destBuffer, unsigned destBufferSize)
{
    return mz_zip_reader_extract_to_mem(&m_mzZipArchive, fileIndex, destBuffer, destBufferSize, 0/*flags*/);
}

size_t ReadFromInputStream(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n)
{
    kmbInputStream* inputStream = (kmbInputStream*)pOpaque;
    inputStream->Seek(unsigned(file_ofs));      // Warning: Converting from 64-bit uint to 32-bit uint...
    unsigned bytesReadBefore = inputStream->GetReadBytesCount();
    inputStream->Read(pBuf, n);

    return size_t(inputStream->GetReadBytesCount() - bytesReadBefore);
}
