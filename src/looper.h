#ifndef _looper_h
#define _looper_h

#include <circle/types.h>
#include <vector>
#include "mididevice.h"

// Maximum recording time in milliseconds (5 minutes)
#define MAX_LOOP_TIME 300000

struct MIDIEvent {
    unsigned timestamp;  // in milliseconds
    u8 status;
    u8 data1;
    u8 data2;
};

class CLooper {
public:
    enum State {
        STOPPED,
        RECORDING,
        PLAYING,
        OVERDUBBING
    };

    CLooper(void);
    ~CLooper(void);

    void Start(void);  // Start recording
    void Stop(void);   // Stop recording/playing
    void Play(void);   // Start playback
    void Clear(void);  // Clear the current loop
    void Overdub(void); // Start overdubbing

    void RecordEvent(u8 status, u8 data1, u8 data2);
    void Update(unsigned currentTime);
    bool IsActive(void) const { return m_state != STOPPED; }
    State GetState(void) const { return m_state; }

private:
    std::vector<MIDIEvent> m_events;
    unsigned m_startTime;
    unsigned m_loopLength;
    unsigned m_lastPlaybackTime;
    State m_state;
    size_t m_currentEventIndex;
};

#endif 