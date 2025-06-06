//
// midikeyboard.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// Original author of this class:
//	R. Stange <rsta2@o2online.de>
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
#ifndef _midikeyboard_h
#define _midikeyboard_h

#include "mididevice.h"
#include "config.h"
#include <circle/usb/usbmidi.h>
#include <circle/device.h>
#include <circle/types.h>
#include "../circle-stdlib/libs/circle/include/circle/string.h"
#include "../circle-stdlib/libs/fatfs/ff.h"
#include <queue>

// Forward declarations
class CDAWController;
class CUIMenu;

#define USB_SYSEX_BUFFER_SIZE (MAX_DX7_SYSEX_LENGTH+128) // Allow a bit spare to handle unexpected SysEx messages

class CMiniDexed;

class CMIDIKeyboard : public CMIDIDevice
{
public:
	CMIDIKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI, unsigned nInstance = 0);
	~CMIDIKeyboard (void);

	void Process (boolean bPlugAndPlayUpdated);

	void Send (const u8 *pMessage, size_t nLength, unsigned nCable = 0) override;
	void SendDisplay (const u8 *pMessage, size_t nLength, unsigned nCable = 0);

	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
			   bool bArrowDown, bool bArrowUp);
	
	void UpdateDAWState (void);
	void UpdateDAWMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG);

	void SetDAWController(CDAWController *pDAWController) { m_pDAWController = pDAWController; }

private:
	static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength, unsigned nDevice, void *pParam);
	static void DeviceRemovedHandler (CDevice *pDevice, void *pContext);
	
	void USBMIDIMessageHandler (u8 *pPacket, unsigned nLength, unsigned nCable, unsigned nDevice);

	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;

private:
	struct TSendQueueEntry
	{
		u8	*pMessage;
		size_t	 nLength;
		unsigned nCable;
	};
	uint8_t m_SysEx[USB_SYSEX_BUFFER_SIZE];
	unsigned m_nSysExIdx;

private:
	unsigned m_nInstance;
	CString m_DeviceName;

	CUSBMIDIDevice * volatile m_pMIDIDevice;

	std::queue<TSendQueueEntry> m_SendQueue;
	u8 m_LastMessage[256];
	TSendQueueEntry m_LastDisplayEntry = {m_LastMessage, 0, 0};
	unsigned m_LastDisplayRefreshTime = 0;

	CDAWController *m_pDAWController;
};

#endif
