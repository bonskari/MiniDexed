#ifndef _midilooper_h
#define _midilooper_h

#include <stdint.h>
#include <vector>
#include <queue>

#define MAX_LOOPS 8
#define MAX_EVENTS_PER_LOOP 2048

class CMiniDexed;  // Forward declaration

struct MIDIEvent {
    uint32_t timestamp;  // in milliseconds since loop start
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
};

struct Loop {
    std::vector<MIDIEvent> events;
    uint32_t length;     // loop length in milliseconds
    bool active;         // whether the loop is currently playing
    bool recording;      // whether the loop is currently recording
    uint32_t startTime;  // when the loop started playing/recording
};

class CMIDILooper {
public:
    CMIDILooper(CMiniDexed* pSynthesizer = nullptr);
    ~CMIDILooper();

    // Loop control
    void StartRecording(uint8_t loopIndex);
    void StartPlayback(uint8_t loopIndex);
    void StopLoop(uint8_t loopIndex);
    void ClearLoop(uint8_t loopIndex);
    void ClearAllLoops();

    // Global controls
    void ToggleOverdub();
    void ToggleMute();
    void ToggleSync();

    // Status queries
    bool IsRecording(uint8_t loopIndex) const;
    bool IsPlaying(uint8_t loopIndex) const;
    bool HasContent(uint8_t loopIndex) const;
    bool IsOverdubEnabled() const { return m_overdubEnabled; }
    bool IsMuted() const { return m_muted; }
    bool IsSyncEnabled() const { return m_syncEnabled; }

    // MIDI event handling
    void ProcessMIDIEvent(uint8_t status, uint8_t data1, uint8_t data2);
    void ProcessTick(); // Called regularly to handle playback

    // Pad control mapping
    void HandlePadPress(uint8_t padNumber, uint8_t velocity);
    void HandlePadRelease(uint8_t padNumber);

    // Configuration
    void SetQuantization(uint32_t quantizeMs) { m_quantizeMs = quantizeMs; }
    void SetSynthesizer(CMiniDexed* pSynthesizer) { m_pSynthesizer = pSynthesizer; }

private:
    void ProcessLoop(Loop& loop);
    void SendMIDIEvent(const MIDIEvent& event);
    uint32_t GetCurrentTime() const;

private:
    Loop m_loops[MAX_LOOPS];
    uint32_t m_quantizeMs;     // Quantization in milliseconds (0 = off)
    bool m_overdubEnabled;     // Whether overdubbing is enabled
    bool m_muted;              // Whether the looper is muted
    bool m_syncEnabled;        // Whether sync is enabled
    uint32_t m_globalTime;     // Global time counter in milliseconds
    CMiniDexed* m_pSynthesizer; // Pointer to the MiniDexed synthesizer
};

#endif 