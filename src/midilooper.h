#ifndef _midilooper_h
#define _midilooper_h

#include <circle/types.h>
#include <vector>
#include <deque>

class CMIDILooper
{
public:
    CMIDILooper();
    ~CMIDILooper();

    // Loop control
    void StartRecording(u8 loopIndex);
    void StopLoop(u8 loopIndex);
    void StartPlayback(u8 loopIndex);
    void ClearLoop(u8 loopIndex);
    void ClearAllLoops();

    // State control
    void ToggleOverdub();
    void ToggleMute();
    void ToggleSync();

    // State queries
    bool IsRecording(u8 loopIndex) const;
    bool IsPlaying(u8 loopIndex) const;
    bool HasContent(u8 loopIndex) const;
    bool IsOverdubEnabled() const { return m_bOverdubEnabled; }
    bool IsMuted() const { return m_bMuted; }
    bool IsSyncEnabled() const { return m_bSyncEnabled; }

    // MIDI event handling
    void HandleMIDIEvent(u8 ucStatus, u8 ucData1, u8 ucData2);
    void Process();

private:
    struct MIDIEvent {
        u32 timestamp;
        u8 status;
        u8 data1;
        u8 data2;
    };

    struct Loop {
        std::vector<MIDIEvent> events;
        bool isRecording;
        bool isPlaying;
        u32 playbackPosition;
        u32 length;
    };

    static const u8 MAX_LOOPS = 8;
    Loop m_loops[MAX_LOOPS];
    
    bool m_bOverdubEnabled;
    bool m_bMuted;
    bool m_bSyncEnabled;
    u32 m_nCurrentTime;

    void StopAllNotes();
};

#endif 