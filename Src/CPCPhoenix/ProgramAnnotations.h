#pragma once

/**
** AddressRange
*/
struct AddressRange
{
    cpcWord start;
    cpcWord end;

    bool Touches(const AddressRange& other) const;
    bool Contains(const AddressRange& other) const;
};

using AddressRangeList = std::vector<AddressRange>;

/**
** ProgramAnnotations
*/
class ProgramAnnotations
{
public:

    ProgramAnnotations() { m_bOk = false; }
    virtual ~ProgramAnnotations() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    const AddressRangeList& GetCodeSegments() const { return m_codeSegments; }
    // Adds a new code segment and, if possible, merges it with existing code segments.
    // E.g. There exist two segments #0000-#0100 and #0150-#0200 and then add the new segment #0101-#0149; the end result will be a single segment #0000-#0200.
    // The function does nothing if the passed segment is already contained in an existing segment.
    // Returns true if segments have changed, or false if the passed segment was already contained in an existing segment.
    bool AddCodeSegment(const AddressRange& newSegment);

private:

    void ResetVars();
    void FreeVars();

    bool m_bOk;
    AddressRangeList m_codeSegments;    // Each segment spans from the first byte of the first instruction to the last byte of the last instruction in the range. TODO: Separate code that lives in ROMs.
};
