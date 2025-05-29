//
// minidexed.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "minidexed.h"
#include <circle/logger.h>
#include <circle/memory.h>
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/gpiopin.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "arm_float_to_q23.h"

LOGMODULE ("minidexed");

CMiniDexed::CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
			CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, CSPIMaster *pSPIMaster, FATFS *pFileSystem)
:
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (pInterrupt),
#endif
	m_pConfig (pConfig),
	m_nToneGenerators (0),
	m_nPolyphony (0),
	m_pLooper (nullptr),
	m_pDAWController (nullptr),
	m_pMIDIDevice (nullptr),
	m_pUI (nullptr)
{
	// ... existing initialization code ...
}

bool CMiniDexed::Initialize (void)
{
	// ... existing initialization code ...

	// Initialize user interface first
	m_pUI = new CUserInterface(this, m_pConfig);
	if (!m_pUI)
	{
		return false;
	}

	// Initialize MIDI device
	CMIDIKeyboard* pKeyboard = new CMIDIKeyboard(this, m_pConfig, m_pUI, 0);
	if (!pKeyboard)
	{
		delete m_pUI;
		return false;
	}
	m_pMIDIDevice = pKeyboard;  // Store in base class pointer

	// Initialize MIDI looper
	m_pLooper = new CMIDILooper(this);
	if (!m_pLooper)
	{
		delete m_pMIDIDevice;
		delete m_pUI;
		return false;
	}

	// Initialize DAW controller
	m_pDAWController = new CDAWController(this, m_pMIDIDevice, m_pUI);
	if (!m_pDAWController)
	{
		delete m_pLooper;
		delete m_pMIDIDevice;
		delete m_pUI;
		return false;
	}

	return true;
}

CMiniDexed::~CMiniDexed (void)
{
	// ... existing cleanup code ...
	delete m_pLooper;
	delete m_pDAWController;
	delete m_pMIDIDevice;
	delete m_pUI;
}

void CMiniDexed::ProcessLooper()
{
	if (m_pLooper)
	{
		m_pLooper->ProcessTick();
	}
}

void CMiniDexed::ProcessDAWController()
{
	if (m_pDAWController)
	{
		m_pDAWController->Process();
	}
}

void CMiniDexed::Process (bool bPlugAndPlayUpdated)
{
	// ... existing process code ...

	// Process MIDI looper
	ProcessLooper();

	// Process DAW controller
	ProcessDAWController();

	// ... rest of existing process code ...
}

void CMiniDexed::HandlePadEvent(uint8_t padNumber, uint8_t velocity)
{
	if (m_pDAWController)
	{
		m_pDAWController->HandleMIDIMessage(0x90 | 9, padNumber, velocity);  // Channel 10 (9 in zero-based)
	}
}

void CMiniDexed::keydown(int16_t pitch, uint8_t velocity, unsigned nTG)
{
	// ... existing keydown code ...

	// Record the note event in the looper if it's not a pad event
	if (m_pLooper && (pitch < 44 || pitch > 51))
	{
		m_pLooper->ProcessMIDIEvent(0x90 | (nTG & 0x0F), pitch, velocity);
	}
}

void CMiniDexed::keyup(int16_t pitch, unsigned nTG)
{
	// ... existing keyup code ...

	// Record the note event in the looper if it's not a pad event
	if (m_pLooper && (pitch < 44 || pitch > 51))
	{
		m_pLooper->ProcessMIDIEvent(0x80 | (nTG & 0x0F), pitch, 0);
	}
}

// ... rest of existing code ... 