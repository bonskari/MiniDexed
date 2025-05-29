#include "midilooper.h"
#include "minidexed.h"
#include <circle/timer.h>
#include <algorithm>

CMIDILooper::CMIDILooper(CMiniDexed* pSynthesizer)
    : m_overdubEnabled(false)
    , m_muted(false)
    , m_syncEnabled(false)
    , m_globalTime(0)
    , m_quantizeMs(0)
    , m_pSynthesizer(pSynthesizer)
{
    ClearAllLoops();
}

CMIDILooper::~CMIDILooper()
{
    ClearAllLoops();
}

void CMIDILooper::StartRecording(uint8_t loopIndex)
{
    if (loopIndex >= MAX_LOOPS)
        return;

    Loop& loop = m_loops[loopIndex];
    
    if (!loop.active) {
        // Start new recording
        loop.events.clear();
        loop.length = 0;
        loop.active = true;
        loop.recording = true;
        loop.startTime = GetCurrentTime();
    } else if (m_overdubEnabled && !loop.recording) {
        // Start overdubbing
        loop.recording = true;
        loop.startTime = GetCurrentTime();
    }
}

void CMIDILooper::StartPlayback(uint8_t loopIndex)
{
    if (loopIndex >= MAX_LOOPS || !m_loops[loopIndex].active)
        return;

    Loop& loop = m_loops[loopIndex];
    loop.recording = false;
    loop.startTime = GetCurrentTime();
}

void CMIDILooper::StopLoop(uint8_t loopIndex)
{
    if (loopIndex >= MAX_LOOPS)
        return;

    Loop& loop = m_loops[loopIndex];
    
    if (loop.recording) {
        uint32_t currentTime = GetCurrentTime();
        loop.length = currentTime - loop.startTime;
        loop.recording = false;
        
        // Quantize loop length if enabled
        if (m_quantizeMs > 0) {
            loop.length = ((loop.length + m_quantizeMs/2) / m_quantizeMs) * m_quantizeMs;
        }
        
        // Sort events by timestamp for efficient playback
        std::sort(loop.events.begin(), loop.events.end(), 
            [](const MIDIEvent& a, const MIDIEvent& b) {
                return a.timestamp < b.timestamp;
            });
    }
}

void CMIDILooper::ClearLoop(uint8_t loopIndex)
{
    if (loopIndex >= MAX_LOOPS)
        return;

    m_loops[loopIndex].events.clear();
    m_loops[loopIndex].length = 0;
    m_loops[loopIndex].active = false;
    m_loops[loopIndex].recording = false;
}

void CMIDILooper::ClearAllLoops()
{
    for (uint8_t i = 0; i < MAX_LOOPS; i++) {
        ClearLoop(i);
    }
}

void CMIDILooper::ToggleOverdub()
{
    m_overdubEnabled = !m_overdubEnabled;
}

void CMIDILooper::ToggleMute()
{
    m_muted = !m_muted;
}

void CMIDILooper::ToggleSync()
{
    m_syncEnabled = !m_syncEnabled;
}

bool CMIDILooper::IsRecording(uint8_t loopIndex) const
{
    return loopIndex < MAX_LOOPS && m_loops[loopIndex].recording;
}

bool CMIDILooper::IsPlaying(uint8_t loopIndex) const
{
    return loopIndex < MAX_LOOPS && m_loops[loopIndex].active && !m_loops[loopIndex].recording;
}

bool CMIDILooper::HasContent(uint8_t loopIndex) const
{
    return loopIndex < MAX_LOOPS && !m_loops[loopIndex].events.empty();
}

void CMIDILooper::ProcessMIDIEvent(uint8_t status, uint8_t data1, uint8_t data2)
{
    // Skip if muted
    if (m_muted)
        return;

    // Record event to any active recording loops
    uint32_t currentTime = GetCurrentTime();
    
    for (uint8_t i = 0; i < MAX_LOOPS; i++) {
        Loop& loop = m_loops[i];
        
        if (loop.recording && loop.events.size() < MAX_EVENTS_PER_LOOP) {
            MIDIEvent event;
            event.timestamp = currentTime - loop.startTime;
            event.status = status;
            event.data1 = data1;
            event.data2 = data2;
            
            // Quantize timestamp if enabled
            if (m_quantizeMs > 0) {
                event.timestamp = (event.timestamp / m_quantizeMs) * m_quantizeMs;
            }
            
            loop.events.push_back(event);
        }
    }

    // Always pass through live events
    SendMIDIEvent({currentTime, status, data1, data2});
}

void CMIDILooper::ProcessTick()
{
    // Process each active loop
    for (uint8_t i = 0; i < MAX_LOOPS; i++) {
        if (IsPlaying(i)) {
            ProcessLoop(m_loops[i]);
        }
    }
    
    // Update global time
    m_globalTime++;
}

void CMIDILooper::HandlePadPress(uint8_t padNumber, uint8_t velocity)
{
    if (padNumber >= MAX_LOOPS)
        return;

    if (velocity > 0) {
        if (!m_loops[padNumber].active) {
            StartRecording(padNumber);
        } else if (!m_loops[padNumber].recording) {
            StartPlayback(padNumber);
        }
    }
}

void CMIDILooper::HandlePadRelease(uint8_t padNumber)
{
    if (padNumber >= MAX_LOOPS)
        return;

    if (m_loops[padNumber].recording) {
        StopLoop(padNumber);
    }
}

void CMIDILooper::ProcessLoop(Loop& loop)
{
    if (!loop.active || loop.events.empty())
        return;

    uint32_t loopTime = (GetCurrentTime() - loop.startTime) % loop.length;
    
    for (const auto& event : loop.events) {
        if (event.timestamp == loopTime) {
            SendMIDIEvent(event);
        }
    }
}

void CMIDILooper::SendMIDIEvent(const MIDIEvent& event)
{
    if (!m_pSynthesizer)
        return;

    uint8_t status = event.status & 0xF0;  // Get message type
    uint8_t channel = event.status & 0x0F; // Get channel
    
    switch (status)
    {
        case 0x90:  // Note On
            if (event.data2 > 0)
            {
                m_pSynthesizer->keydown(event.data1, event.data2, channel);
            }
            else
            {
                m_pSynthesizer->keyup(event.data1, channel);
            }
            break;
            
        case 0x80:  // Note Off
            m_pSynthesizer->keyup(event.data1, channel);
            break;
            
        case 0xB0:  // Control Change
            // Handle various CC messages
            switch (event.data1)
            {
                case 0x01:  // Modulation Wheel
                    m_pSynthesizer->setModWheel(event.data2, channel);
                    break;
                case 0x04:  // Foot Controller
                    m_pSynthesizer->setFootController(event.data2, channel);
                    break;
                case 0x02:  // Breath Controller
                    m_pSynthesizer->setBreathController(event.data2, channel);
                    break;
                case 0x07:  // Volume
                    m_pSynthesizer->SetVolume(event.data2, channel);
                    break;
                case 0x0A:  // Pan
                    m_pSynthesizer->SetPan(event.data2, channel);
                    break;
                case 0x40:  // Sustain
                    m_pSynthesizer->setSustain(event.data2 >= 64, channel);
                    break;
                case 0x42:  // Sostenuto
                    m_pSynthesizer->setSostenuto(event.data2 >= 64, channel);
                    break;
            }
            break;
            
        case 0xD0:  // Channel Aftertouch
            m_pSynthesizer->setAftertouch(event.data1, channel);
            break;
    }
}

uint32_t CMIDILooper::GetCurrentTime() const
{
    return CTimer::GetClockTicks() / (CTimer::CLOCKHZ / 1000); // Convert to milliseconds
} 