#ifndef REGISTRATIONBOOK_H
#define REGISTRATIONBOOK_H

#include "legogamestate.h"
#include "legoworld.h"

class InfocenterState;
class MxControlPresenter;
class MxEndActionNotificationParam;
class MxStillPresenter;
class LegoControlManagerNotificationParam;
class LegoPathStructNotificationParam;

// VTABLE: LEGO1 0x100d9928
// VTABLE: BETA10 0x101bfb48
// SIZE 0x2d0
class RegistrationBook : public LegoWorld {
public:
	RegistrationBook();
	~RegistrationBook() override; // vtable+0x00

	MxLong Notify(MxParam& p_param) override; // vtable+0x04
	MxResult Tickle() override;               // vtable+0x08

	// FUNCTION: LEGO1 0x10076e10
	// FUNCTION: BETA10 0x100f4000
	const char* ClassName() const override // vtable+0x0c
	{
		// STRING: LEGO1 0x100f04c8
		return "RegistrationBook";
	}

	// FUNCTION: LEGO1 0x10076e20
	MxBool IsA(const char* p_name) const override // vtable+0x10
	{
		return !strcmp(p_name, RegistrationBook::ClassName()) || LegoWorld::IsA(p_name);
	}

	MxResult Create(MxDSAction& p_dsAction) override; // vtable+0x18
	void ReadyWorld() override;                       // vtable+0x50
	MxBool Escape() override;                         // vtable+0x64
	void Enable(MxBool p_enable) override;            // vtable+0x68

	inline static void PlayAction(MxU32 p_objectId);

	// SYNTHETIC: LEGO1 0x10076f30
	// RegistrationBook::`scalar deleting destructor'

private:
	MxS32 m_registerDialogueTimer;    // 0xf8
	undefined m_unk0xfc;              // 0xfc
	undefined m_unk0xfd[3];           // 0xfd
	MxStillPresenter* m_alphabet[26]; // 0x100
	MxStillPresenter* m_name[10][7];  // 0x168
	struct {
		MxS16 m_letters[7];                // 0x00
		MxS16 m_cursorPos;                 // 0x0e
	} m_unk0x280;                          // 0x280
	MxControlPresenter* m_checkmark[10];   // 0x290
	undefined2 m_unk0x2b8;                 // 0x2b8
	InfocenterState* m_infocenterState;    // 0x2bc
	undefined m_unk0x2c0;                  // 0x2c0
	undefined m_unk0x2c1;                  // 0x2c1
	undefined m_unk0x2c2[0x02];            // 0x2c2
	LPDIRECTDRAWSURFACE m_checkboxHilite;  // 0x2c4
	LPDIRECTDRAWSURFACE m_checkboxSurface; // 0x2c8
	LPDIRECTDRAWSURFACE m_checkboxNormal;  // 0x2cc
	MxStillPresenter* m_intAlphabet[sizeOfArray(LegoGameState::g_intCharacters)];
	MxU32 m_intAlphabetOffset;

	MxLong HandleEndAction(MxEndActionNotificationParam& p_param);
	MxLong HandleKeyPress(SDL_Keycode p_key);
	MxLong HandleControl(LegoControlManagerNotificationParam& p_param);
	MxLong HandlePathStruct(LegoPathStructNotificationParam& p_param);
	void FUN_100775c0(MxS16 p_playerIndex);
	void WriteInfocenterLetters(MxS16);
	void FUN_100778c0();
	MxBool CreateSurface();
};

#endif // REGISTRATIONBOOK_H
