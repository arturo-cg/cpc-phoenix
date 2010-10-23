//-------------------------------------------------------------------------------------------
// File:        MemoryBlock.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMemoryBlock.h"
#include "Stream/kmbInputStream.h"


namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemoryBlock::CMemoryBlock(kmbInputStream* pContentStream/* = NULL*/)
  {
    // Reset members
    ResetVars();

    // Fill memory content, if any
    if (pContentStream != NULL)
    {
      FillContent( pContentStream );
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMemoryBlock::ResetVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CMemoryBlock::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemoryBlock::FillContent(kmbInputStream* pContentStream)
  {
    if ( (pContentStream != NULL) && pContentStream->IsOk() )
    {
      // Determine the conten length
      unsigned nLength;
      nLength = pContentStream->GetLength();
      if (nLength > MEMORY_BLOCK_LENGTH)
      {
        // We won't read more than 16Kb
        nLength = MEMORY_BLOCK_LENGTH;
      }

      // Copy the content to the memory block
      pContentStream->Read( m_anBytes, nLength );
    }
  }

} //namespace CPC
