#ifndef _dawcontroller_h
#define _dawcontroller_h

#include <circle/types.h>
#include "minidexed.h"
#include "mididevice.h"
#include "userinterface.h"

// DAW MIDI message types
#define MIDI_DAW_CHANGE 0x0B  // Control Change on channel 10

// DAW MIDI CC numbers for looper control
#define MIDI_DAW_LOOPER_REC    0x24  // CC 36
#define MIDI_DAW_LOOPER_STOP   0x25  // CC 37
#define MIDI_DAW_LOOPER_CLEAR  0x26  // CC 38
#define MIDI_DAW_LOOPER_OVERDUB 0x27 // CC 39
#define MIDI_DAW_LOOPER_MUTE   0x28  // CC 40
#define MIDI_DAW_LOOPER_SYNC   0x29  // CC 41

class CDAWController
{
public:
    CDAWController(CMiniDexed* pSynthesizer, CMIDIDevice* pMIDIDevice, CUserInterface* pUI);
    ~CDAWController();

    void HandleMIDIMessage(u8 ucStatus, u8 ucData1, u8 ucData2);
    void Process();

private:
    void HandlePadEvent(u8 ucPadNumber, u8 ucVelocity);
    void UpdatePadColors();

private:
    CMiniDexed* m_pSynthesizer;
    CMIDIDevice* m_pMIDIDevice;
    CUserInterface* m_pUI;
};

#endif 