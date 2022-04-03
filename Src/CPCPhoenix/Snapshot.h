#pragma once

/** A snapshot represents the state of the machine at a given point in time.
    Applying a snapshot to an emulated machine brings the machine back to the state it was in when the snapshot was taken. */
class Snapshot
{
public:

    Snapshot() { m_bOk = false; }
    virtual ~Snapshot() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

private:

    void ResetVars();
    void FreeVars();

    bool m_bOk;
};
