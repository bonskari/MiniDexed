#include "looper.h"
#include <circle/timer.h>
#include <circle/logger.h>

CLooper::CLooper(void)
    : m_startTime(0)
    , m_loopLength(0)
    , m_lastPlaybackTime(0)
    , m_state(STOPPED)
    , m_currentEventIndex(0)
{
}

CLooper::~CLooper(void)
{
}

void CLooper::Start(void)
{
    if (m_state == STOPPED) {
        m_events.clear();
        m_startTime = CTimer::Get()->GetTicks();
        m_state = RECORDING;
        m_currentEventIndex = 0;
        CLogger::Get()->Write("Looper", LogNotice, "Recording started");
    }
}

void CLooper::Stop(void)
{
    if (m_state == RECORDING) {
        m_loopLength = CTimer::Get()->GetTicks() - m_startTime;
        if (m_loopLength > 0 && !m_events.empty()) {
            m_state = STOPPED;
            CLogger::Get()->Write("Looper", LogNotice, "Recording stopped, loop length: %u ms", m_loopLength);
        } else {
            Clear();
        }
    } else if (m_state == PLAYING || m_state == OVERDUBBING) {
        m_state = STOPPED;
        CLogger::Get()->Write("Looper", LogNotice, "Playback stopped");
    }
}

void CLooper::Play(void)
{
    if (m_state == STOPPED && !m_events.empty()) {
        m_startTime = CTimer::Get()->GetTicks();
        m_lastPlaybackTime = m_startTime;
        m_currentEventIndex = 0;
        m_state = PLAYING;
        CLogger::Get()->Write("Looper", LogNotice, "Playback started");
    }
}

void CLooper::Clear(void)
{
    m_events.clear();
    m_loopLength = 0;
    m_state = STOPPED;
    CLogger::Get()->Write("Looper", LogNotice, "Loop cleared");
}

void CLooper::Overdub(void)
{
    if (m_state == PLAYING) {
        m_state = OVERDUBBING;
        CLogger::Get()->Write("Looper", LogNotice, "Overdubbing started");
    } else if (m_state == OVERDUBBING) {
        m_state = PLAYING;
        CLogger::Get()->Write("Looper", LogNotice, "Overdubbing stopped");
    }
}

void CLooper::RecordEvent(u8 status, u8 data1, u8 data2)
{
    if (m_state == RECORDING || m_state == OVERDUBBING) {
        unsigned currentTime = CTimer::Get()->GetTicks();
        if (currentTime - m_startTime < MAX_LOOP_TIME) {
            MIDIEvent event;
            event.timestamp = currentTime - m_startTime;
            event.status = status;
            event.data1 = data1;
            event.data2 = data2;
            m_events.push_back(event);
        }
    }
}

void CLooper::Update(unsigned currentTime)
{
    if (m_state == PLAYING || m_state == OVERDUBBING) {
        unsigned elapsedTime = currentTime - m_startTime;
        
        // Handle loop wrapping
        if (elapsedTime >= m_loopLength) {
            m_startTime = currentTime - (elapsedTime % m_loopLength);
            m_currentEventIndex = 0;
            CLogger::Get()->Write("Looper", LogDebug, "Loop wrapped");
        }

        // Play events that occurred between last update and current time
        while (m_currentEventIndex < m_events.size()) {
            const MIDIEvent& event = m_events[m_currentEventIndex];
            unsigned eventTime = m_startTime + event.timestamp;
            
            if (eventTime <= currentTime) {
                // TODO: Send MIDI event to synthesizer
                m_currentEventIndex++;
            } else {
                break;
            }
        }
        
        m_lastPlaybackTime = currentTime;
    }
} 