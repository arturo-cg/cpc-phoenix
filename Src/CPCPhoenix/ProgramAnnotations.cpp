#include "stdafx.h"
#include "ProgramAnnotations.h"

/**
** AddressRange
*/

bool AddressRange::Touches(const AddressRange& other) const
{
    if (start == other.start)
    {
        return true;
    }
    else
    {
        if (start < other.start)
        {
            return (end == 0xFFFF) || ((end + 1) >= other.start);
        }
        else
        {
            return (other.end == 0xFFFF) || ((other.end + 1) >= start);
        }
    }
}

bool AddressRange::Contains(const AddressRange& other) const
{
    return (start <= other.start) && (end >= other.end);
}

/**
** ProgramAnnotations
*/

bool ProgramAnnotations::Init()
{
    bool bRet = true;

    End();
    ResetVars();

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Initialize class members
    if (bRet)
    {
        //...
    }


    if (bRet)
    {
        m_bOk = true;
    }
    else
    {
        FreeVars();
    }

    return bRet;
}

/*virtual*/ void ProgramAnnotations::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void ProgramAnnotations::ResetVars()
{
    m_codeSegments.clear();
}

void ProgramAnnotations::FreeVars()
{
    //...
}

bool ProgramAnnotations::AddCodeSegment(const AddressRange& newSegment)
{
    bool ret = false;

    // Find where the new segment should be inserted.
    AddressRangeList::iterator iter = std::lower_bound(m_codeSegments.begin(), m_codeSegments.end(), newSegment, [](auto a, auto b) {
        return a.start < b.start;
        });

    if ((iter != m_codeSegments.end()) && iter->Contains(newSegment))
    {
        // Do nothing, an existing segment already contains the new one.
    }
    else
    {
        bool mergeWithPrevious = (iter != m_codeSegments.begin()) && newSegment.Touches(*(iter - 1));
        bool mergeWithNext = (iter != m_codeSegments.end()) && newSegment.Touches(*iter);

        if (mergeWithPrevious && mergeWithNext)
        {
            // Merge previous, new and next segments into a single one.
            AddressRange& previousSegment = *(iter - 1);
            const AddressRange& nextSegment = *iter;
            previousSegment.end = max(newSegment.end, nextSegment.end);
            m_codeSegments.erase(iter);
            ret = true;
        }
        else if (mergeWithPrevious)
        {
            // Expand the previous segment to contain the new one.
            AddressRange& previousSegment = *(iter - 1);
            previousSegment.end = max(previousSegment.end, newSegment.end);
            ret = true;
        }
        else if (mergeWithNext)
        {
            // Expand the next segment to contain the new one.
            AddressRange& nextSegment = *iter;
            nextSegment.start = newSegment.start;
            ret = true;
        }
        else
        {
            // Insert the new, isolated segment.
            m_codeSegments.insert(iter, newSegment);
            ret = true;
        }
    }

    return ret;
}
