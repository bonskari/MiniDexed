//
// dawcontroller.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
#include "dawcontroller.h"
#include <circle/logger.h>
#include <assert.h>

LOGMODULE ("dawcontroller");

CDAWController::CDAWController(CMiniDexed* pSynthesizer, CMIDIDevice* pMIDIDevice, CUserInterface* pUI)
    : m_pSynthesizer(pSynthesizer)
    , m_pMIDIDevice(pMIDIDevice)
    , m_pUI(pUI)
{
    assert(m_pSynthesizer != nullptr);
    assert(m_pMIDIDevice != nullptr);
    assert(m_pUI != nullptr);
}

CDAWController::~CDAWController()
{
    // Nothing to clean up
}

void CDAWController::HandleMIDIMessage(u8 ucStatus, u8 ucData1, u8 ucData2)
{
    u8 ucType = ucStatus >> 4;
    u8 ucChannel = ucStatus & 0x0F;

    // Check if this is a note message on channel 10 (9 in zero-based)
    if (ucChannel == 9)
    {
        if (ucType == 0x9 || ucType == 0x8)  // Note On or Note Off
        {
            // Convert note number to pad number (44-51 for Bank B)
            if (ucData1 >= 44 && ucData1 <= 51)
            {
                // For Note Off messages or Note On with velocity 0, use velocity 0
                u8 velocity = (ucType == 0x8 || ucData2 == 0) ? 0 : ucData2;
                HandlePadEvent(ucData1, velocity);
            }
        }
    }
}

void CDAWController::HandlePadEvent(u8 ucPadNumber, u8 ucVelocity)
{
    if (!m_pSynthesizer || !m_pSynthesizer->GetLooper())
        return;

    // Convert pad number to looper index (44-51 -> 0-7)
    u8 padIndex = ucPadNumber - 44;

    if (padIndex >= 0 && padIndex <= 3)
    {
        // Loop control pads (1-4)
        if (ucVelocity > 0)
        {
            if (!m_pSynthesizer->GetLooper()->HasContent(padIndex))
            {
                m_pSynthesizer->GetLooper()->StartRecording(padIndex);
            }
            else if (!m_pSynthesizer->GetLooper()->IsRecording(padIndex))
            {
                m_pSynthesizer->GetLooper()->StartPlayback(padIndex);
            }
        }
        else if (m_pSynthesizer->GetLooper()->IsRecording(padIndex))
        {
            m_pSynthesizer->GetLooper()->StopLoop(padIndex);
        }
    }
    else if (padIndex == 4 && ucVelocity > 0)
    {
        // Clear All Loops (pad 5)
        m_pSynthesizer->GetLooper()->ClearAllLoops();
    }
    else if (padIndex == 5 && ucVelocity > 0)
    {
        // Toggle Overdub (pad 6)
        m_pSynthesizer->GetLooper()->ToggleOverdub();
    }
    else if (padIndex == 6 && ucVelocity > 0)
    {
        // Toggle Mute (pad 7)
        m_pSynthesizer->GetLooper()->ToggleMute();
    }
    else if (padIndex == 7 && ucVelocity > 0)
    {
        // Toggle Sync (pad 8)
        m_pSynthesizer->GetLooper()->ToggleSync();
    }

    // Update pad colors to reflect new state
    UpdatePadColors();
}

void CDAWController::Process()
{
    // Update pad colors periodically to reflect current state
    UpdatePadColors();
}

void CDAWController::UpdatePadColors()
{
    if (!m_pSynthesizer || !m_pSynthesizer->GetLooper())
        return;

    CMIDILooper* looper = m_pSynthesizer->GetLooper();

    // Update loop pad colors (pads 1-4)
    for (u8 i = 0; i < 4; i++)
    {
        u8 color;
        if (looper->IsRecording(i))
        {
            color = 0x03;  // Red for recording
        }
        else if (looper->IsPlaying(i))
        {
            color = 0x01;  // Green for playing
        }
        else if (looper->HasContent(i))
        {
            color = 0x02;  // Blue for has content
        }
        else
        {
            color = 0x00;  // Off for empty
        }

        // Send color update via SysEx
        u8 sysex[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 44 + i, color, 0xF7};
        m_pMIDIDevice->Send(sysex, sizeof(sysex), 0);
    }

    // Update function pad colors (pads 5-8)
    u8 clearColor = 0x03;  // Red for clear
    u8 overdubColor = looper->IsOverdubEnabled() ? 0x07 : 0x00;  // Orange when enabled
    u8 muteColor = looper->IsMuted() ? 0x06 : 0x00;  // Yellow when enabled
    u8 syncColor = looper->IsSyncEnabled() ? 0x05 : 0x00;  // Purple when enabled

    // Send color updates
    u8 sysex5[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 48, clearColor, 0xF7};
    u8 sysex6[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 49, overdubColor, 0xF7};
    u8 sysex7[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 50, muteColor, 0xF7};
    u8 sysex8[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 51, syncColor, 0xF7};

    m_pMIDIDevice->Send(sysex5, sizeof(sysex5), 0);
    m_pMIDIDevice->Send(sysex6, sizeof(sysex6), 0);
    m_pMIDIDevice->Send(sysex7, sizeof(sysex7), 0);
    m_pMIDIDevice->Send(sysex8, sizeof(sysex8), 0);
} 