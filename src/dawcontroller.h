// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2024 The MiniDexed Team
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
#ifndef _dawcontroller_h
#define _dawcontroller_h

#include <circle/types.h>
#include <circle/timer.h>
#include "minidexed.h"
#include "config.h"
#include "userinterface.h"
#include "looper.h"

// Forward declarations
class CMIDIKeyboard;

struct CColor
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class CDAWConnection
{
public:
	virtual void DisplayWrite(const char *pMenu, const char *pParam,
							const char *pValue, bool bArrowDown, bool bArrowUp) = 0;
	virtual void UpdateState() = 0;
	virtual void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) = 0;
	virtual void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) = 0;
	virtual void SetPadColor(unsigned nPad, CColor color) = 0;
	virtual ~CDAWConnection() = default;

	static const unsigned nDefaultDisplayUpdateDelay = 2000;
};

class CDAWController
{
public:
	CDAWController(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	~CDAWController();

	void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue,
					 bool bArrowDown, bool bArrowUp);
	void UpdateState();
	void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG);
	void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void MIDISysexHandler(u8 *pPacket, unsigned nLength, unsigned nCable);

	// Looper functions
	void StartLooper(unsigned nPad);
	void StopLooper(unsigned nPad);
	void ClearLooper(unsigned nPad);
	void ToggleOverdub(unsigned nPad);
	void HandlePadPress(unsigned nPad, bool bLongPress);
	void UpdateLooper(void);

private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;
	CDAWConnection *m_pDAWConnection;
	CLooper m_looper[8];  // 8 independent loopers
};

// DAW Connection implementations
class CMiniLab3DawConnection : public CDAWConnection
{
public:
	CMiniLab3DawConnection(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState() override;
	void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
	void SetPadColor(unsigned nPad, CColor color) override;

private:
	enum TPadID {
		MonoPad = 0,
		PortamentoPad = 1,
		SostenutoPad = 2,
		SustainPad = 3,
		SoundOffPad = 4,
		HoldPad = 5,
		TBDPad7 = 6,
		ATPad = 7,
	};

	enum TBankID {
		BankA = 0x34,
		BankB = 0x44,
	};

	void SetPadColor(TBankID BankID, TPadID PadID, CColor color);
	void SetPadColor(TBankID BankID, TPadID PadID, u8 state);
	void SetPadColor(TBankID BankID, TPadID PadID, u8 state, u8 state2);
	void SetPadColor(TBankID BankID, TPadID PadID, CColor color, u8 state);
};

class CKeyLabEs3DawConnection : public CDAWConnection
{
public:
	CKeyLabEs3DawConnection(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState() override;
	void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
	void SetPadColor(unsigned nPad, CColor color) override;
};

class CKeyLab2DawConnection : public CDAWConnection
{
public:
	CKeyLab2DawConnection(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState() override;
	void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
	void SetPadColor(unsigned nPad, CColor color) override;
};

class CKeyLabEsDawConnection : public CDAWConnection
{
public:
	CKeyLabEsDawConnection(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState() override;
	void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
	void SetPadColor(unsigned nPad, CColor color) override;
};

#endif
