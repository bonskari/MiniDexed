#ifndef _dawcontroller_h
#define _dawcontroller_h

#include <circle/types.h>
#include <circle/timer.h>
#include <circle/string.h>

class CMiniDexed;
class CMIDIKeyboard;
class CConfig;
class CUserInterface;
class CUIMenu;
class CMIDILooper;

class CDAWConnection
{
public:
    virtual void DisplayWrite(const char *pMenu, const char *pParam,
                            const char *pValue, bool bArrowDown, bool bArrowUp) = 0;
    virtual void UpdateState() = 0;
    virtual void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) = 0;
    virtual void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) = 0;
    virtual ~CDAWConnection(void) = default;

    static const unsigned nDefaultDisplayUpdateDelay = 2000;
};

struct CColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Looper-specific colors
static const CColor COLOR_RECORDING = {255, 0, 0};    // Red
static const CColor COLOR_PLAYING = {0, 255, 0};      // Green
static const CColor COLOR_HAS_CONTENT = {0, 0, 255};  // Blue
static const CColor COLOR_OVERDUB = {255, 165, 0};    // Orange
static const CColor COLOR_MUTE = {255, 255, 0};       // Yellow
static const CColor COLOR_SYNC = {128, 0, 128};       // Purple
static const CColor COLOR_OFF = {0, 0, 0};           // Off

class CMiniLab3DawConnection : public CDAWConnection
{
public:
    CMiniLab3DawConnection(CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
    ~CMiniLab3DawConnection();
    
    void DisplayWrite(const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
    void UpdateState() override;
    void UpdateMenu(CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
    void MIDIListener(u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;

private:
    enum TPadID {
        // Bank A pads
        MonoPad = 0,
        PortamentoPad = 1,
        SostenutoPad = 2,
        SustainPad = 3,
        SoundOffPad = 4,
        HoldPad = 5,
        TBDPad7 = 6,
        ATPad = 7,

        // Bank B pads (Looper)
        Loop1Pad = 0,
        Loop2Pad = 1,
        Loop3Pad = 2,
        Loop4Pad = 3,
        ClearAllPad = 4,
        OverdubPad = 5,
        MutePad = 6,
        SyncPad = 7
    };

    enum TBankID {
        BankA = 0x34,
        BankB = 0x44,
    };

    static const u8 AllOP = 8;

    void DisplayWriteSimple(const char *pMenu, const char *pParam, const char *pValue);
    static void s_UpdateDisplay(TKernelTimerHandle hTimer, void *pParam, void *pContext);
    void QueueUpdateDisplay(unsigned msec);
    void UpdateDisplay();
    void ShowEncoderDisplay();
    void ShowValueDisplay();

    void UpdateEncoder(uint8_t ucEncID, uint8_t ucValue);
    void UpdateEncoders();
    void UpdateTGColors();
    void UpdateMonoColor();
    void UpdatePortamentoColor();
    void UpdateATColor(u8 ucAT);
    void UpdateLooperColors();
    void HandleLooperPad(TPadID PadID, u8 ucValue);

    CMiniDexed *m_pSynthesizer;
    CMIDIKeyboard *m_pKeyboard;
    CConfig *m_pConfig;
    CUserInterface *m_pUI;
    CMIDILooper *m_pLooper;

    bool m_bDisableEncoderUpdate = false;

    CUIMenu::TCPageType m_encoderPageType = CUIMenu::PageMain;
    u8 m_ucEncoderPage = 0;
    u8 m_ucEncoderOP = 0;
    u8 m_ucEncoderTG = 0;

    enum DisplayState {
        DisplayMenu,
        DisplayEncoder,
        DisplayValues,
    };

    DisplayState m_DisplayState = DisplayMenu;

    static constexpr size_t nEncoder = 8;
    static constexpr size_t nFader = 4;

    TKernelTimerHandle m_DisplayTimer = 0;

    u8 m_ucFirstTG = 0;
};

#endif
