#ifndef _minidexed_h
#define _minidexed_h

#include <circle/interrupt.h>
#include <circle/i2cmaster.h>
#include <circle/spimaster.h>
#include <circle/gpiomanager.h>
#include <circle/timer.h>
#include <circle/sounddevice.h>
#include <circle/types.h>
#include <circle/multicore.h>
#include <circle/spinlock.h>
#include <circle/logger.h>
#include <fatfs/ff.h>
#include <string>
#include <arm_neon.h>
#include "config.h"
#include "dexedadapter.h"
#include "mididevice.h"
#include "midikeyboard.h"
#include "serialmididevice.h"
#include "pckeyboard.h"
#include "userinterface.h"
#include "sysexfileloader.h"
#include "performanceconfig.h"
#include "midilooper.h"
#include "dawcontroller.h"

class CMiniDexed
#ifdef ARM_ALLOW_MULTI_CORE
:	public CMultiCoreSupport
#endif
{
public:
	enum TParameter
	{
		ParameterCompressorEnable,
		ParameterReverbEnable,
		ParameterReverbSize,
		ParameterReverbHighDamp,
		ParameterReverbLowDamp,
		ParameterReverbLowPass,
		ParameterReverbDiffusion,
		ParameterReverbLevel,
		ParameterPerformanceSelectChannel,
		ParameterUnknown
	};

	enum TTGParameter
	{
		TGParameterVoiceBank,
		TGParameterVoiceBankMSB,
		TGParameterVoiceBankLSB,
		TGParameterProgram,
		TGParameterVolume,
		TGParameterPan,
		TGParameterMasterTune,
		TGParameterCutoff,
		TGParameterResonance,
		TGParameterMIDIChannel,
		TGParameterReverbSend,
		TGParameterPitchBendRange,
		TGParameterPitchBendStep,
		TGParameterPortamentoMode,
		TGParameterPortamentoGlissando,
		TGParameterPortamentoTime,
		TGParameterMonoMode,
		TGParameterEnabled,
		TGParameterMWRange,
		TGParameterMWPitch,
		TGParameterMWAmplitude,
		TGParameterMWEGBias,
		TGParameterFCRange,
		TGParameterFCPitch,
		TGParameterFCAmplitude,
		TGParameterFCEGBias,
		TGParameterBCRange,
		TGParameterBCPitch,
		TGParameterBCAmplitude,
		TGParameterBCEGBias,
		TGParameterATRange,
		TGParameterATPitch,
		TGParameterATAmplitude,
		TGParameterATEGBias,
		TGParameterUnknown
	};

public:
	CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
		    CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, CSPIMaster *pSPIMaster, FATFS *pFileSystem);
	~CMiniDexed (void);

	bool Initialize (void);

	void Process (bool bPlugAndPlayUpdated);

#ifdef ARM_ALLOW_MULTI_CORE
	void Run (unsigned nCore);
#endif

	CSysExFileLoader *GetSysExFileLoader (void);
	CPerformanceConfig *GetPerformanceConfig (void);

	void BankSelect    (unsigned nBank, unsigned nTG);
	void BankSelectPerformance    (unsigned nBank);
	void BankSelectMSB (unsigned nBankMSB, unsigned nTG);
	void BankSelectMSBPerformance (unsigned nBankMSB);
	void BankSelectLSB (unsigned nBankLSB, unsigned nTG);
	void BankSelectLSBPerformance (unsigned nBankLSB);
	void ProgramChange (unsigned nProgram, unsigned nTG);
	void ProgramChangePerformance (unsigned nProgram);
	void SetVolume (unsigned nVolume, unsigned nTG);
	void SetExpression (unsigned nExpression, unsigned nTG);
	void SetPan (unsigned nPan, unsigned nTG);			// 0 .. 127
	void SetMasterTune (int nMasterTune, unsigned nTG);		// -99 .. 99
	void SetCutoff (int nCutoff, unsigned nTG);			// 0 .. 99
	void SetResonance (int nResonance, unsigned nTG);		// 0 .. 99
	void SetMIDIChannel (uint8_t uchChannel, unsigned nTG);

	void keyup (int16_t pitch, unsigned nTG);
	void keydown (int16_t pitch, uint8_t velocity, unsigned nTG);

	void setSustain (bool sustain, unsigned nTG);
	void setSostenuto (bool sostenuto, unsigned nTG);
	void setHoldMode(bool holdmode, unsigned nTG);
	void panic (uint8_t value, unsigned nTG);
	void notesOff (uint8_t value, unsigned nTG);
	void setModWheel (uint8_t value, unsigned nTG);
	void setPitchbend (int16_t value, unsigned nTG);
	void ControllersRefresh (unsigned nTG);

	void setFootController (uint8_t value, unsigned nTG);
	void setBreathController (uint8_t value, unsigned nTG);
	void setAftertouch (uint8_t value, unsigned nTG);

	void SetReverbSend (unsigned nReverbSend, unsigned nTG);			// 0 .. 127

	void SetParameter (TParameter Parameter, int nValue);
	int GetParameter (TParameter Parameter);

	void SetTGParameter (TTGParameter Parameter, int nValue, unsigned nTG);
	int GetTGParameter (TTGParameter Parameter, unsigned nTG);

	static const unsigned NoOP = 6;		// for global parameters
	void SetVoiceParameter (uint8_t uchOffset, uint8_t uchValue, unsigned nOP, unsigned nTG);
	uint8_t GetVoiceParameter (uint8_t uchOffset, unsigned nOP, unsigned nTG);

	std::string GetVoiceName (unsigned nTG);

	bool SavePerformance (void);
	bool DoSavePerformance (void);

	void setMasterVolume (float32_t vol);

	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
			   bool bArrowDown, bool bArrowUp);

	void UpdateDAWState ();
	void UpdateDAWMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG);

	// MIDI Looper integration
	CMIDILooper* GetLooper() { return m_pLooper; }
	void ProcessLooper();
	void HandlePadEvent(uint8_t padNumber, uint8_t velocity);

	// DAW Controller integration
	CDAWController* GetDAWController() { return m_pDAWController; }
	void ProcessDAWController();

private:
	int16_t ApplyNoteLimits (int16_t pitch, unsigned nTG);	// returns < 0 to ignore note
	uint8_t m_uchOPMask[CConfig::AllToneGenerators];
	void LoadPerformanceParameters(void); 
	void ProcessSound (void);

#ifdef ARM_ALLOW_MULTI_CORE
	enum TCoreStatus
	{
		CoreStatusInit,
		CoreStatusIdle,
		CoreStatusBusy,
		CoreStatusExit,
		CoreStatusUnknown
	};
#endif

private:
	CConfig *m_pConfig;

	int m_nParameter[ParameterUnknown];			// global (non-TG) parameters
	
	unsigned m_nToneGenerators;
	unsigned m_nPolyphony;

	CDexedAdapter *m_pTG[CConfig::AllToneGenerators];
	CMIDILooper *m_pLooper;  // MIDI looper instance

	CDAWController* m_pDAWController;  // DAW controller instance

	CMIDIDevice* m_pMIDIDevice;  // MIDI device instance
	CUserInterface* m_pUI;       // User interface instance
};

#endif 