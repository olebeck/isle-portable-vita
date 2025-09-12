// From NetStream, by @GrapheneCt

#include "Dialog.hpp"

#include <common_gui_dialog.h>
#include <paf.h>

#define CURRENT_DIALOG_NONE -1

static SceInt32 s_currentDialog = CURRENT_DIALOG_NONE;
static std::function<void(dialog::ButtonCode)> s_buttonCallback = nullptr;

static SceUInt32 s_twoButtonContTable[12];
static SceUInt32 s_threeButtonContTable[16];

static void CommonGuiEventHandler(SceInt32 instanceSlot, sce::CommonGuiDialog::DIALOG_CB buttonCode, ScePVoid pUserArg)
{
	sce::CommonGuiDialog::Dialog::Close(instanceSlot);
	s_currentDialog = CURRENT_DIALOG_NONE;

	if (s_buttonCallback) {
		s_buttonCallback((dialog::ButtonCode) buttonCode);
		s_buttonCallback = nullptr;
	}
}

void OpenDialog(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	sce::CommonGuiDialog::Param* dialogParam,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	if (s_currentDialog != CURRENT_DIALOG_NONE) {
		return;
	}

	paf::wstring title;
	paf::wstring* pTitle = nullptr;
	if (titleText != nullptr) {
		title = titleText;
		pTitle = &title;
	}

	paf::wstring message;
	paf::wstring* pMessage = nullptr;
	if (messageText != nullptr) {
		message = messageText;
		pMessage = &message;
	}

	s_buttonCallback = onClick;
	bool isMainThread = paf::thread::ThreadIDCache::Check(paf::thread::ThreadIDCache::Type_Main);
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Lock();
	}
	s_currentDialog =
		sce::CommonGuiDialog::Dialog::Show(workPlugin, pTitle, pMessage, dialogParam, CommonGuiEventHandler, nullptr);
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Unlock();
	}
}

void dialog::OpenPleaseWait(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	bool withCancel,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	sce::CommonGuiDialog::Param* dialogParam = withCancel ? &sce::CommonGuiDialog::Param::s_dialogCancelBusy
														  : &sce::CommonGuiDialog::Param::s_dialogTextSmallBusy;
	OpenDialog(workPlugin, titleText, messageText, dialogParam, onClick);
}

void dialog::OpenYesNo(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	OpenDialog(workPlugin, titleText, messageText, &sce::CommonGuiDialog::Param::s_dialogYesNo, onClick);
}

void dialog::OpenOk(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	OpenDialog(workPlugin, titleText, messageText, &sce::CommonGuiDialog::Param::s_dialogOk, onClick);
}

void dialog::OpenError(
	paf::Plugin* workPlugin,
	SceInt32 errorCode,
	const wchar_t* messageText,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	if (s_currentDialog != CURRENT_DIALOG_NONE) {
		return;
	}

	sce::CommonGuiDialog::ErrorDialog dialog;
	dialog.work_plugin = workPlugin;
	dialog.error = errorCode;
	dialog.listener = new sce::CommonGuiDialog::EventCBListener(CommonGuiEventHandler, nullptr);
	dialog.message = messageText;

	s_buttonCallback = onClick;
	bool isMainThread = paf::thread::ThreadIDCache::Check(paf::thread::ThreadIDCache::Type_Main);
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Lock();
	}
	s_currentDialog = dialog.Show();
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Unlock();
	}
}

void dialog::OpenThreeButton(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	uint32_t button1TextHashref,
	uint32_t button2TextHashref,
	uint32_t button3TextHashref,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	sce::CommonGuiDialog::Param dialogParam = sce::CommonGuiDialog::Param::s_dialogYesNoCancel;
	sce_paf_memcpy(
		s_threeButtonContTable,
		sce::CommonGuiDialog::Param::s_dialogYesNoCancel.contents_list,
		sizeof(s_threeButtonContTable)
	);
	s_threeButtonContTable[1] = button1TextHashref;
	s_threeButtonContTable[5] = button2TextHashref;
	s_threeButtonContTable[9] = button3TextHashref;
	s_threeButtonContTable[7] = 0x20413274;
	s_threeButtonContTable[11] = 0x20413274;
	dialogParam.contents_list = (sce::CommonGuiDialog::ContentsHashTable*) s_threeButtonContTable;

	OpenDialog(workPlugin, titleText, messageText, &dialogParam, onClick);
}

void dialog::OpenTwoButton(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	const wchar_t* messageText,
	uint32_t button1TextHashref,
	uint32_t button2TextHashref,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	sce::CommonGuiDialog::Param dialogParam = sce::CommonGuiDialog::Param::s_dialogYesNo;
	sce_paf_memcpy(
		s_twoButtonContTable,
		sce::CommonGuiDialog::Param::s_dialogYesNo.contents_list,
		sizeof(s_twoButtonContTable)
	);
	s_twoButtonContTable[1] = button2TextHashref;
	s_twoButtonContTable[5] = button1TextHashref;
	s_twoButtonContTable[3] = 0x20413274;
	dialogParam.contents_list = (sce::CommonGuiDialog::ContentsHashTable*) s_twoButtonContTable;

	OpenDialog(workPlugin, titleText, messageText, &dialogParam, onClick);
}

paf::ui::ListView* dialog::OpenListView(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	if (s_currentDialog != CURRENT_DIALOG_NONE) {
		return nullptr;
	}

	s_buttonCallback = onClick;

	bool isMainThread = paf::thread::ThreadIDCache::Check(paf::thread::ThreadIDCache::Type_Main);
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Lock();
	}

	paf::wstring title = titleText;

	s_currentDialog = sce::CommonGuiDialog::Dialog::Show(
		workPlugin,
		&title,
		nullptr,
		&sce::CommonGuiDialog::Param::s_dialogXLView,
		CommonGuiEventHandler,
		nullptr
	);
	paf::ui::Widget* ret =
		sce::CommonGuiDialog::Dialog::GetWidget(s_currentDialog, sce::CommonGuiDialog::REGISTER_ID_LIST_VIEW);

	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Unlock();
	}

	return (paf::ui::ListView*) ret;
}

paf::ui::ScrollView* dialog::OpenScrollView(
	paf::Plugin* workPlugin,
	const wchar_t* titleText,
	std::function<void(dialog::ButtonCode)> onClick
)
{
	if (s_currentDialog != CURRENT_DIALOG_NONE) {
		return nullptr;
	}

	s_buttonCallback = onClick;

	bool isMainThread = paf::thread::ThreadIDCache::Check(paf::thread::ThreadIDCache::Type_Main);
	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Lock();
	}

	paf::wstring title = titleText;

	s_currentDialog = sce::CommonGuiDialog::Dialog::Show(
		workPlugin,
		&title,
		nullptr,
		&sce::CommonGuiDialog::Param::s_dialogXView,
		CommonGuiEventHandler,
		nullptr
	);
	paf::ui::Widget* ret =
		sce::CommonGuiDialog::Dialog::GetWidget(s_currentDialog, sce::CommonGuiDialog::REGISTER_ID_SCROLL_VIEW);

	if (!isMainThread) {
		paf::thread::RMutex::MainThreadMutex()->Unlock();
	}

	return (paf::ui::ScrollView*) ret;
}

void dialog::Close()
{
	if (s_currentDialog == CURRENT_DIALOG_NONE) {
		return;
	}

	sce::CommonGuiDialog::Dialog::Close(s_currentDialog);
	s_currentDialog = CURRENT_DIALOG_NONE;
	s_buttonCallback = nullptr;
}

void dialog::WaitEnd()
{
	if (s_currentDialog == CURRENT_DIALOG_NONE) {
		return;
	}

	while (s_currentDialog != CURRENT_DIALOG_NONE) {
		paf::thread::Sleep(100);
	}
}

int dialog::Current()
{
	return s_currentDialog;
}
