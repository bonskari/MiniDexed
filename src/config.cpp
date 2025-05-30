//
// config.cpp
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
#include "config.h"
#include "../Synth_Dexed/src/dexed.h"

CConfig::CConfig (FATFS *pFileSystem)
:	m_Properties ("minidexed.ini", pFileSystem)
{
}

CConfig::~CConfig (void)
{
}

void CConfig::Load (void)
{
	m_Properties.Load ();
	
	// Number of Tone Generators and Polyphony
	m_nToneGenerators = m_Properties.GetNumber ("ToneGenerators", DefToneGenerators);
	m_nPolyphony = m_Properties.GetNumber ("Polyphony", DefaultNotes);
	// At present there are only two options for tone generators: min or max
	// and for the Pi 1,2,3 these are the same anyway.
	if ((m_nToneGenerators != MinToneGenerators) && (m_nToneGenerators != AllToneGenerators))
	{
		m_nToneGenerators = DefToneGenerators;
	}
	if (m_nPolyphony > MaxNotes)
	{
		m_nPolyphony = DefaultNotes;
	}
	
	m_bUSBGadget = m_Properties.GetNumber ("USBGadget", 0) != 0;
	m_nUSBGadgetPin = m_Properties.GetNumber ("USBGadgetPin", 0); // Default OFF
	SetUSBGadgetMode(m_bUSBGadget); // Might get overriden later by USBGadgetPin state

	m_SoundDevice = m_Properties.GetString ("SoundDevice", "pwm");

	m_nSampleRate = m_Properties.GetNumber ("SampleRate", 48000);
	m_bQuadDAC8Chan = m_Properties.GetNumber ("QuadDAC8Chan", 0) != 0;
	if (m_SoundDevice == "hdmi") {
		m_nChunkSize = m_Properties.GetNumber ("ChunkSize", 384*6);
	}
	else
	{
#ifdef ARM_ALLOW_MULTI_CORE
		m_nChunkSize = m_Properties.GetNumber ("ChunkSize", m_bQuadDAC8Chan ? 1024 : 256);  // 128 per channel
#else
		m_nChunkSize = m_Properties.GetNumber ("ChunkSize", 1024);
#endif
	}
	m_nDACI2CAddress = m_Properties.GetNumber ("DACI2CAddress", 0);
	m_bChannelsSwapped = m_Properties.GetNumber ("ChannelsSwapped", 0) != 0;

	unsigned newEngineType = m_Properties.GetNumber ("EngineType", 1);
	if (newEngineType == 2) {
  		m_EngineType = MKI;
	} else if (newEngineType == 3) {
  		m_EngineType = OPL;
	} else {
  		m_EngineType = MSFA;
	}

	m_nMIDIBaudRate = m_Properties.GetNumber ("MIDIBaudRate", 31250);

	const char *pMIDIThru = m_Properties.GetString ("MIDIThru");
	if (pMIDIThru)
	{
		std::string Arg (pMIDIThru);

		size_t nPos = Arg.find (',');
		if (nPos != std::string::npos)
		{
			m_MIDIThruIn = Arg.substr (0, nPos);
			m_MIDIThruOut = Arg.substr (nPos+1);

			if (   m_MIDIThruIn.empty ()
			    || m_MIDIThruOut.empty ())
			{
				m_MIDIThruIn.clear ();
				m_MIDIThruOut.clear ();
			}
		}
	}
	
	m_bMIDIRXProgramChange = m_Properties.GetNumber ("MIDIRXProgramChange", 1) != 0;
	m_bIgnoreAllNotesOff = m_Properties.GetNumber ("IgnoreAllNotesOff", 0) != 0;
	m_bMIDIAutoVoiceDumpOnPC = m_Properties.GetNumber ("MIDIAutoVoiceDumpOnPC", 0) != 0;
	m_bHeaderlessSysExVoices = m_Properties.GetNumber ("HeaderlessSysExVoices", 0) != 0;
	m_bExpandPCAcrossBanks = m_Properties.GetNumber ("ExpandPCAcrossBanks", 1) != 0;
	
	m_nMIDISystemCCVol = m_Properties.GetNumber ("MIDISystemCCVol", 0);
	m_nMIDISystemCCPan = m_Properties.GetNumber ("MIDISystemCCPan", 0);
	m_nMIDISystemCCDetune = m_Properties.GetNumber ("MIDISystemCCDetune", 0);
	m_nMIDIGlobalExpression = m_Properties.GetNumber ("MIDIGlobalExpression", 0);

	m_bLCDEnabled = m_Properties.GetNumber ("LCDEnabled", 0) != 0;
	m_nLCDPinEnable = m_Properties.GetNumber ("LCDPinEnable", 4);
	m_nLCDPinRegisterSelect = m_Properties.GetNumber ("LCDPinRegisterSelect", 27);
	m_nLCDPinReadWrite = m_Properties.GetNumber ("LCDPinReadWrite", 0);
	m_nLCDPinData4 = m_Properties.GetNumber ("LCDPinData4", 22);
	m_nLCDPinData5 = m_Properties.GetNumber ("LCDPinData5", 23);
	m_nLCDPinData6 = m_Properties.GetNumber ("LCDPinData6", 24);
	m_nLCDPinData7 = m_Properties.GetNumber ("LCDPinData7", 25);
	m_nLCDI2CAddress = m_Properties.GetNumber ("LCDI2CAddress", 0);

	m_nSSD1306LCDI2CAddress = m_Properties.GetNumber ("SSD1306LCDI2CAddress", 0);
	m_nSSD1306LCDWidth = m_Properties.GetNumber ("SSD1306LCDWidth", 128);
	m_nSSD1306LCDHeight = m_Properties.GetNumber ("SSD1306LCDHeight", 32);
	m_bSSD1306LCDRotate = m_Properties.GetNumber ("SSD1306LCDRotate", 0) != 0;
	m_bSSD1306LCDMirror = m_Properties.GetNumber ("SSD1306LCDMirror", 0) != 0;

	m_nSPIBus = m_Properties.GetNumber ("SPIBus", SPI_INACTIVE);  // Disabled by default
	m_nSPIMode = m_Properties.GetNumber ("SPIMode", SPI_DEF_MODE);
	m_nSPIClockKHz = m_Properties.GetNumber ("SPIClockKHz", SPI_DEF_CLOCK);

	m_bST7789Enabled = m_Properties.GetNumber ("ST7789Enabled", 0) != 0;
	m_nST7789Data = m_Properties.GetNumber ("ST7789Data", 0);
	m_nST7789Select = m_Properties.GetNumber ("ST7789Select", 0);
	m_nST7789Reset = m_Properties.GetNumber ("ST7789Reset", 0);  // optional
	m_nST7789Backlight = m_Properties.GetNumber ("ST7789Backlight", 0);  // optional
	m_nST7789Width = m_Properties.GetNumber ("ST7789Width", 240);
	m_nST7789Height = m_Properties.GetNumber ("ST7789Height", 240);
	m_nST7789Rotation = m_Properties.GetNumber ("ST7789Rotation", 0);
	m_bST7789SmallFont = m_Properties.GetNumber ("ST7789SmallFont", 0) != 0;

	m_nLCDColumns = m_Properties.GetNumber ("LCDColumns", 16);
	m_nLCDRows = m_Properties.GetNumber ("LCDRows", 2);

	m_nButtonPinPrev = m_Properties.GetNumber ("ButtonPinPrev", 0);
	m_nButtonPinNext = m_Properties.GetNumber ("ButtonPinNext", 0);
	m_nButtonPinBack = m_Properties.GetNumber ("ButtonPinBack", 11);
	m_nButtonPinSelect = m_Properties.GetNumber ("ButtonPinSelect", 11);
	m_nButtonPinHome = m_Properties.GetNumber ("ButtonPinHome", 11);
	m_nButtonPinShortcut = m_Properties.GetNumber ("ButtonPinShortcut", 11);

	m_ButtonActionPrev = m_Properties.GetString ("ButtonActionPrev", "");
	m_ButtonActionNext = m_Properties.GetString ("ButtonActionNext", "");
	m_ButtonActionBack = m_Properties.GetString ("ButtonActionBack", "doubleclick");
	m_ButtonActionSelect = m_Properties.GetString ("ButtonActionSelect", "click");
	m_ButtonActionHome = m_Properties.GetString ("ButtonActionHome", "longpress");

	m_nDoubleClickTimeout = m_Properties.GetNumber ("DoubleClickTimeout", 400);
	m_nLongPressTimeout = m_Properties.GetNumber ("LongPressTimeout", 600);

	m_nButtonPinPgmUp = m_Properties.GetNumber ("ButtonPinPgmUp", 0);
	m_nButtonPinPgmDown = m_Properties.GetNumber ("ButtonPinPgmDown", 0);
	m_nButtonPinBankUp = m_Properties.GetNumber ("ButtonPinBankUp", 0);
	m_nButtonPinBankDown = m_Properties.GetNumber ("ButtonPinBankDown", 0);
	m_nButtonPinTGUp = m_Properties.GetNumber ("ButtonPinTGUp", 0);
	m_nButtonPinTGDown = m_Properties.GetNumber ("ButtonPinTGDown", 0);

	m_ButtonActionPgmUp = m_Properties.GetString ("ButtonActionPgmUp", "");
	m_ButtonActionPgmDown = m_Properties.GetString ("ButtonActionPgmDown", "");
	m_ButtonActionBankUp = m_Properties.GetString ("ButtonActionBankUp", "");
	m_ButtonActionBankDown = m_Properties.GetString ("ButtonActionBankDown", "");
	m_ButtonActionTGUp = m_Properties.GetString ("ButtonActionTGUp", "");
	m_ButtonActionTGDown = m_Properties.GetString ("ButtonActionTGDown", "");

	m_nMIDIButtonCh = m_Properties.GetNumber ("MIDIButtonCh", 0);
	m_nMIDIButtonNotes = m_Properties.GetNumber ("MIDIButtonNotes", 0);

	m_nMIDIButtonPrev = m_Properties.GetNumber ("MIDIButtonPrev", 0);
	m_nMIDIButtonNext = m_Properties.GetNumber ("MIDIButtonNext", 0);
	m_nMIDIButtonBack = m_Properties.GetNumber ("MIDIButtonBack", 0);
	m_nMIDIButtonSelect = m_Properties.GetNumber ("MIDIButtonSelect", 0);
	m_nMIDIButtonHome = m_Properties.GetNumber ("MIDIButtonHome", 0);

	m_MIDIButtonActionPrev = m_Properties.GetString ("MIDIButtonActionPrev", "");
	m_MIDIButtonActionNext = m_Properties.GetString ("MIDIButtonActionNext", "");
	m_MIDIButtonActionBack = m_Properties.GetString ("MIDIButtonActionBack", "");
	m_MIDIButtonActionSelect = m_Properties.GetString ("MIDIButtonActionSelect", "");
	m_MIDIButtonActionHome = m_Properties.GetString ("MIDIButtonActionHome", "");

	m_nMIDIButtonPgmUp = m_Properties.GetNumber ("MIDIButtonPgmUp", 0);
	m_nMIDIButtonPgmDown = m_Properties.GetNumber ("MIDIButtonPgmDown", 0);
	m_nMIDIButtonBankUp = m_Properties.GetNumber ("MIDIButtonBankUp", 0);
	m_nMIDIButtonBankDown = m_Properties.GetNumber ("MIDIButtonBankDown", 0);
	m_nMIDIButtonTGUp = m_Properties.GetNumber ("MIDIButtonTGUp", 0);
	m_nMIDIButtonTGDown = m_Properties.GetNumber ("MIDIButtonTGDown", 0);

	m_MIDIButtonActionPgmUp = m_Properties.GetString ("MIDIButtonActionPgmUp", "");
	m_MIDIButtonActionPgmDown = m_Properties.GetString ("MIDIButtonActionPgmDown", "");
	m_MIDIButtonActionBankUp = m_Properties.GetString ("MIDIButtonActionBankUp", "");
	m_MIDIButtonActionBankDown = m_Properties.GetString ("MIDIButtonActionBankDown", "");
	m_MIDIButtonActionTGUp = m_Properties.GetString ("MIDIButtonActionTGUp", "");
	m_MIDIButtonActionTGDown = m_Properties.GetString ("MIDIButtonActionTGDown", "");

	m_bDAWControllerEnabled = m_Properties.GetNumber ("DAWControllerEnabled", 0) != 0;

	m_bEncoderEnabled = m_Properties.GetNumber ("EncoderEnabled", 0) != 0;
	m_nEncoderPinClock = m_Properties.GetNumber ("EncoderPinClock", 10);
	m_nEncoderPinData = m_Properties.GetNumber ("EncoderPinData", 9);

	m_bMIDIDumpEnabled  = m_Properties.GetNumber ("MIDIDumpEnabled", 0) != 0;
	m_bProfileEnabled = m_Properties.GetNumber ("ProfileEnabled", 0) != 0;
	m_bPerformanceSelectToLoad = m_Properties.GetNumber ("PerformanceSelectToLoad", 0) != 0;
	m_bPerformanceSelectChannel = m_Properties.GetNumber ("PerformanceSelectChannel", 0);
	
	// Network
	m_bNetworkEnabled  = m_Properties.GetNumber ("NetworkEnabled", 0) != 0;
	m_bNetworkDHCP  = m_Properties.GetNumber ("NetworkDHCP", 0) != 0;
	m_NetworkType = m_Properties.GetString ("NetworkType", "wlan");
	m_NetworkHostname = m_Properties.GetString ("NetworkHostname", "MiniDexed");
	m_INetworkIPAddress = m_Properties.GetIPAddress("NetworkIPAddress") != 0;
	m_INetworkSubnetMask = m_Properties.GetIPAddress("NetworkSubnetMask") != 0;
	m_INetworkDefaultGateway = m_Properties.GetIPAddress("NetworkDefaultGateway") != 0;
	m_bSyslogEnabled  = m_Properties.GetNumber ("NetworkSyslogEnabled", 0) != 0;
	m_INetworkDNSServer = m_Properties.GetIPAddress("NetworkDNSServer") != 0;
	m_bNetworkFTPEnabled = m_Properties.GetNumber("NetworkFTPEnabled", 0) != 0;

	const u8 *pSyslogServerIP = m_Properties.GetIPAddress ("NetworkSyslogServerIPAddress");
	if (pSyslogServerIP)
	{
		m_INetworkSyslogServerIPAddress.Set (pSyslogServerIP);
	}

	m_nMasterVolume = m_Properties.GetNumber ("MasterVolume", 64);
}

unsigned CConfig::GetToneGenerators (void) const
{
	return m_nToneGenerators;
}

unsigned CConfig::GetPolyphony (void) const
{
	return m_nPolyphony;
}

unsigned CConfig::GetTGsCore1 (void) const
{
#ifndef ARM_ALLOW_MULTI_CORE
	return 0;
#else
	if (m_nToneGenerators > MinToneGenerators)
	{
		return TGsCore1 + TGsCore1Opt;
	}
	else
	{
		return TGsCore1;
	}
#endif
}

unsigned CConfig::GetTGsCore23 (void) const
{
#ifndef ARM_ALLOW_MULTI_CORE
	return 0;
#else
	if (m_nToneGenerators > MinToneGenerators)
	{
		return TGsCore23 + TGsCore23Opt;
	}
	else
	{
		return TGsCore23;
	}
#endif
}

bool CConfig::GetUSBGadget (void) const
{
	return m_bUSBGadget;
}

unsigned CConfig::GetUSBGadgetPin (void) const
{
	return m_nUSBGadgetPin;
}

bool CConfig::GetUSBGadgetMode (void) const
{
	return m_bUSBGadgetMode;
}

void CConfig::SetUSBGadgetMode (bool USBGadgetMode)
{
	m_bUSBGadgetMode = USBGadgetMode;
}

const char *CConfig::GetSoundDevice (void) const
{
	return m_SoundDevice.c_str ();
}

unsigned CConfig::GetSampleRate (void) const
{
	return m_nSampleRate;
}

unsigned CConfig::GetChunkSize (void) const
{
	return m_nChunkSize;
}

unsigned CConfig::GetDACI2CAddress (void) const
{
	return m_nDACI2CAddress;
}

bool CConfig::GetChannelsSwapped (void) const
{
	return m_bChannelsSwapped;
}

unsigned CConfig::GetEngineType (void) const
{
	return m_EngineType;
}

bool CConfig::GetQuadDAC8Chan (void) const
{
	return m_bQuadDAC8Chan;
}

unsigned CConfig::GetMIDIBaudRate (void) const
{
	return m_nMIDIBaudRate;
}

const char *CConfig::GetMIDIThruIn (void) const
{
	return m_MIDIThruIn.c_str ();
}

const char *CConfig::GetMIDIThruOut (void) const
{
	return m_MIDIThruOut.c_str ();
}

bool CConfig::GetMIDIRXProgramChange (void) const
{
	return m_bMIDIRXProgramChange;
}

bool CConfig::GetIgnoreAllNotesOff (void) const
{
	return m_bIgnoreAllNotesOff;
}

bool CConfig::GetMIDIAutoVoiceDumpOnPC (void) const
{
	return m_bMIDIAutoVoiceDumpOnPC;
}

bool CConfig::GetHeaderlessSysExVoices (void) const
{
	return m_bHeaderlessSysExVoices;
}

bool CConfig::GetExpandPCAcrossBanks (void) const
{
	return m_bExpandPCAcrossBanks;
}

unsigned CConfig::GetMIDISystemCCVol (void) const
{
	return m_nMIDISystemCCVol;
}

unsigned CConfig::GetMIDISystemCCPan (void) const
{
	return m_nMIDISystemCCPan;
}

unsigned CConfig::GetMIDISystemCCDetune (void) const
{
	return m_nMIDISystemCCDetune;
}

unsigned CConfig::GetMIDIGlobalExpression (void) const
{
	return m_nMIDIGlobalExpression;
}

bool CConfig::GetLCDEnabled (void) const
{
	return m_bLCDEnabled;
}

unsigned CConfig::GetLCDPinEnable (void) const
{
	return m_nLCDPinEnable;
}

unsigned CConfig::GetLCDPinRegisterSelect (void) const
{
	return m_nLCDPinRegisterSelect;
}

unsigned CConfig::GetLCDPinReadWrite (void) const
{
	return m_nLCDPinReadWrite;
}

unsigned CConfig::GetLCDPinData4 (void) const
{
	return m_nLCDPinData4;
}

unsigned CConfig::GetLCDPinData5 (void) const
{
	return m_nLCDPinData5;
}

unsigned CConfig::GetLCDPinData6 (void) const
{
	return m_nLCDPinData6;
}

unsigned CConfig::GetLCDPinData7 (void) const
{
	return m_nLCDPinData7;
}

unsigned CConfig::GetLCDI2CAddress (void) const
{
	return m_nLCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDI2CAddress (void) const
{
	return m_nSSD1306LCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDWidth (void) const
{
	return m_nSSD1306LCDWidth;
}

unsigned CConfig::GetSSD1306LCDHeight (void) const
{
	return m_nSSD1306LCDHeight;
}

bool CConfig::GetSSD1306LCDRotate (void) const
{
	return m_bSSD1306LCDRotate;
}

bool CConfig::GetSSD1306LCDMirror (void) const
{
	return m_bSSD1306LCDMirror;
}

unsigned CConfig::GetSPIBus (void) const
{
	return m_nSPIBus;
}

unsigned CConfig::GetSPIMode (void) const
{
	return m_nSPIMode;
}

unsigned CConfig::GetSPIClockKHz (void) const
{
	return m_nSPIClockKHz;
}

bool CConfig::GetST7789Enabled (void) const
{
	return m_bST7789Enabled;
}

unsigned CConfig::GetST7789Data (void) const
{
	return m_nST7789Data;
}

unsigned CConfig::GetST7789Select (void) const
{
	return m_nST7789Select;
}

unsigned CConfig::GetST7789Reset (void) const
{
	return m_nST7789Reset;
}

unsigned CConfig::GetST7789Backlight (void) const
{
	return m_nST7789Backlight;
}

unsigned CConfig::GetST7789Width (void) const
{
	return m_nST7789Width;
}

unsigned CConfig::GetST7789Height (void) const
{
	return m_nST7789Height;
}

unsigned CConfig::GetST7789Rotation (void) const
{
	return m_nST7789Rotation;
}

bool CConfig::GetST7789SmallFont (void) const
{
	return m_bST7789SmallFont;
}
unsigned CConfig::GetLCDColumns (void) const
{
	return m_nLCDColumns;
}

unsigned CConfig::GetLCDRows (void) const
{
	return m_nLCDRows;
}

unsigned CConfig::GetButtonPinPrev (void) const
{
	return m_nButtonPinPrev;
}

unsigned CConfig::GetButtonPinNext (void) const
{
	return m_nButtonPinNext;
}

unsigned CConfig::GetButtonPinBack (void) const
{
	return m_nButtonPinBack;
}

unsigned CConfig::GetButtonPinSelect (void) const
{
	return m_nButtonPinSelect;
}

unsigned CConfig::GetButtonPinHome (void) const
{
	return m_nButtonPinHome;
}

unsigned CConfig::GetButtonPinShortcut (void) const
{
	return m_nButtonPinShortcut;
}

const char *CConfig::GetButtonActionPrev (void) const
{
	return m_ButtonActionPrev.c_str();
}

const char *CConfig::GetButtonActionNext (void) const
{
	return m_ButtonActionNext.c_str();
}

const char *CConfig::GetButtonActionBack (void) const
{
	return m_ButtonActionBack.c_str();
}

const char *CConfig::GetButtonActionSelect (void) const
{
	return m_ButtonActionSelect.c_str();
}

const char *CConfig::GetButtonActionHome (void) const
{
	return m_ButtonActionHome.c_str();
}

unsigned CConfig::GetDoubleClickTimeout (void) const
{
	return m_nDoubleClickTimeout;
}

unsigned CConfig::GetLongPressTimeout (void) const
{
	return m_nLongPressTimeout;
}

unsigned CConfig::GetButtonPinPgmUp (void) const
{
	return m_nButtonPinPgmUp;
}

unsigned CConfig::GetButtonPinPgmDown (void) const
{
	return m_nButtonPinPgmDown;
}

unsigned CConfig::GetButtonPinBankUp (void) const
{
	return m_nButtonPinBankUp;
}

unsigned CConfig::GetButtonPinBankDown (void) const
{
	return m_nButtonPinBankDown;
}

unsigned CConfig::GetButtonPinTGUp (void) const
{
	return m_nButtonPinTGUp;
}

unsigned CConfig::GetButtonPinTGDown (void) const
{
	return m_nButtonPinTGDown;
}

const char *CConfig::GetButtonActionPgmUp (void) const
{
	return m_ButtonActionPgmUp.c_str();
}

const char *CConfig::GetButtonActionPgmDown (void) const
{
	return m_ButtonActionPgmDown.c_str();
}

const char *CConfig::GetButtonActionBankUp (void) const
{
	return m_ButtonActionBankUp.c_str();
}

const char *CConfig::GetButtonActionBankDown (void) const
{
	return m_ButtonActionBankDown.c_str();
}

const char *CConfig::GetButtonActionTGUp (void) const
{
	return m_ButtonActionTGUp.c_str();
}

const char *CConfig::GetButtonActionTGDown (void) const
{
	return m_ButtonActionTGDown.c_str();
}

unsigned CConfig::GetMIDIButtonCh (void) const
{
	return m_nMIDIButtonCh;
}

unsigned CConfig::GetMIDIButtonNotes (void) const
{
	return m_nMIDIButtonNotes;
}

unsigned CConfig::GetMIDIButtonPrev (void) const
{
	return m_nMIDIButtonPrev;
}

unsigned CConfig::GetMIDIButtonNext (void) const
{
	return m_nMIDIButtonNext;
}

unsigned CConfig::GetMIDIButtonBack (void) const
{
	return m_nMIDIButtonBack;
}

unsigned CConfig::GetMIDIButtonSelect (void) const
{
	return m_nMIDIButtonSelect;
}

unsigned CConfig::GetMIDIButtonHome (void) const
{
	return m_nMIDIButtonHome;
}

const char *CConfig::GetMIDIButtonActionPrev (void) const
{
	return m_MIDIButtonActionPrev.c_str();
}

const char *CConfig::GetMIDIButtonActionNext (void) const
{
	return m_MIDIButtonActionNext.c_str();
}

const char *CConfig::GetMIDIButtonActionBack (void) const
{
	return m_MIDIButtonActionBack.c_str();
}

const char *CConfig::GetMIDIButtonActionSelect (void) const
{
	return m_MIDIButtonActionSelect.c_str();
}

const char *CConfig::GetMIDIButtonActionHome (void) const
{
	return m_MIDIButtonActionHome.c_str();
}

unsigned CConfig::GetMIDIButtonPgmUp (void) const
{
	return m_nMIDIButtonPgmUp;
}

unsigned CConfig::GetMIDIButtonPgmDown (void) const
{
	return m_nMIDIButtonPgmDown;
}

unsigned CConfig::GetMIDIButtonBankUp (void) const
{
	return m_nMIDIButtonBankUp;
}

unsigned CConfig::GetMIDIButtonBankDown (void) const
{
	return m_nMIDIButtonBankDown;
}

unsigned CConfig::GetMIDIButtonTGUp (void) const
{
	return m_nMIDIButtonTGUp;
}

unsigned CConfig::GetMIDIButtonTGDown (void) const
{
	return m_nMIDIButtonTGDown;
}

const char *CConfig::GetMIDIButtonActionPgmUp (void) const
{
	return m_MIDIButtonActionPgmUp.c_str();
}

const char *CConfig::GetMIDIButtonActionPgmDown (void) const
{
	return m_MIDIButtonActionPgmDown.c_str();
}

const char *CConfig::GetMIDIButtonActionBankUp (void) const
{
	return m_MIDIButtonActionBankUp.c_str();
}

const char *CConfig::GetMIDIButtonActionBankDown (void) const
{
	return m_MIDIButtonActionBankDown.c_str();
}

const char *CConfig::GetMIDIButtonActionTGUp (void) const
{
	return m_MIDIButtonActionTGUp.c_str();
}

const char *CConfig::GetMIDIButtonActionTGDown (void) const
{
	return m_MIDIButtonActionTGDown.c_str();
}

bool CConfig::GetDAWControllerEnabled (void) const
{
	return m_bDAWControllerEnabled;
}

bool CConfig::GetEncoderEnabled (void) const
{
	return m_bEncoderEnabled;
}

unsigned CConfig::GetEncoderPinClock (void) const
{
	return m_nEncoderPinClock;
}

unsigned CConfig::GetEncoderPinData (void) const
{
	return m_nEncoderPinData;
}

bool CConfig::GetMIDIDumpEnabled (void) const
{
	return m_bMIDIDumpEnabled;
}

bool CConfig::GetProfileEnabled (void) const
{
	return m_bProfileEnabled;
}

bool CConfig::GetPerformanceSelectToLoad (void) const
{
	return m_bPerformanceSelectToLoad;
}

unsigned CConfig::GetPerformanceSelectChannel (void) const
{
	return m_bPerformanceSelectChannel;
}

// Network
bool CConfig::GetNetworkEnabled (void) const
{
	return m_bNetworkEnabled;
}

bool CConfig::GetNetworkDHCP (void) const
{
	return m_bNetworkDHCP;
}

const char *CConfig::GetNetworkType (void) const
{
	return m_NetworkType.c_str();
}

const char *CConfig::GetNetworkHostname (void) const
{
	return m_NetworkHostname.c_str();
}

CIPAddress CConfig::GetNetworkIPAddress (void) const
{	
	return m_INetworkIPAddress;
}

CIPAddress CConfig::GetNetworkSubnetMask (void) const
{
	return m_INetworkSubnetMask;
}

CIPAddress CConfig::GetNetworkDefaultGateway (void) const
{
	return m_INetworkDefaultGateway;
}

CIPAddress CConfig::GetNetworkDNSServer (void) const
{
	return m_INetworkDNSServer;
}

bool CConfig::GetSyslogEnabled (void) const
{
	return m_bSyslogEnabled;
}

CIPAddress CConfig::GetNetworkSyslogServerIPAddress (void) const
{
	return m_INetworkSyslogServerIPAddress;
}

bool CConfig::GetNetworkFTPEnabled (void) const
{
	return m_bNetworkFTPEnabled;
}
