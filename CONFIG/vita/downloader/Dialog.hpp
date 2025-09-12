#pragma once

#include <paf.h>
#include <common_gui_dialog.h>

namespace dialog
{
	enum ButtonCode
	{
		ButtonCode_X = 1,
		ButtonCode_Ok,
		ButtonCode_Cancel,
		ButtonCode_Yes,
		ButtonCode_No,
		ButtonCode_Button1 = ButtonCode_Yes,
		ButtonCode_Button2 = ButtonCode_No,
		ButtonCode_Button3 = ButtonCode_Cancel
	};

    int Current();

	void OpenPleaseWait(
        paf::Plugin *workPlugin,
        const wchar_t* titleText,
        const wchar_t* messageText,
        bool withCancel = false,
        std::function<void(ButtonCode)> onClick = nullptr
    );

	void OpenYesNo(paf::Plugin *workPlugin, 
        const wchar_t* titleText, 
        const wchar_t* messageText, 
        std::function<void(ButtonCode)> onClick = nullptr
    );

	void OpenOk(paf::Plugin *workPlugin, 
        const wchar_t* titleText, 
        const wchar_t* messageText, 
        std::function<void(ButtonCode)> onClick = nullptr
    );

	void OpenError(paf::Plugin *workPlugin, 
        SceInt32 errorCode, 
        const wchar_t* messageText, 
        std::function<void(ButtonCode)> onClick = nullptr
    );

	void OpenTwoButton(
		paf::Plugin *workPlugin,
		const wchar_t* titleText,
		const wchar_t* messageText,
		uint32_t button1TextHashref,
		uint32_t button2TextHashref,
		std::function<void(ButtonCode)> onClick = nullptr
    );

	void OpenThreeButton(
		paf::Plugin *workPlugin,
		const wchar_t* titleText,
		const wchar_t* messageText,
		uint32_t button1TextHashref,
		uint32_t button2TextHashref,
		uint32_t button3TextHashref,
		std::function<void(ButtonCode)> onClick = nullptr
    );

	paf::ui::ListView *OpenListView(
		paf::Plugin *workPlugin,
		const wchar_t* titleText,
		std::function<void(ButtonCode)> onClick = nullptr
    );

	paf::ui::ScrollView *OpenScrollView(
		paf::Plugin *workPlugin,
		const wchar_t* titleText,
		std::function<void(ButtonCode)> onClick = nullptr
    );

	void Close();

	void WaitEnd();
};
