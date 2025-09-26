#include "StartupPage.hpp"

#include "Dialog.hpp"
#include "util.hpp"

class VersionItemFactory : public paf::ui::listview::ItemFactory {
private:
	paf::Plugin* plugin;
	EVersion selected;
	std::function<void(EVersion)> OnSelect;

public:
	VersionItemFactory(paf::Plugin* plugin, EVersion selected, std::function<void(EVersion)> OnSelect)
	{
		this->plugin = plugin;
		this->selected = selected;
		this->OnSelect = OnSelect;
	}

	virtual paf::ui::ListItem* Create(CreateParam& param) override
	{
		EVersion version = (EVersion) (param.cell_index + 1);

		paf::Plugin::TemplateOpenParam openParam;
		int res = this->plugin->TemplateOpen(param.parent, "tmpl_version_list_item", openParam);
		if (res != 0) {
			sceClibPrintf("TemplateOpen 0x%X\n", res);
		}

		paf::ui::ListItem* list_item = (paf::ui::ListItem*) param.parent->GetChild(param.parent->GetChildrenNum() - 1);
		auto button = static_cast<paf::ui::Button*>(list_item->FindChild("button"));
		button->SetString(StringToWString(EVersionToString(version)));
		AddClickCallback(button, std::bind(this->OnSelect, version));
		return list_item;
	}

	virtual void Start(StartParam& param) override { param.list_item->Show(paf::common::transition::Type_FadeinSlow); }
	virtual void Stop(StopParam& param) override { param.list_item->Hide(paf::common::transition::Type_FadeinSlow); }
	virtual void Dispose(DisposeParam& param) override{};
};

StartupPage::StartupPage(EVersion version, int missingCount)
{
	this->version = version;
	this->missingCount = missingCount;
}

void StartupPage::Mount()
{
	auto download_button = this->GetElementById<paf::ui::Button>("download_button");
	auto verify_button = this->GetElementById<paf::ui::Button>("verify_button");
	auto versions_button = this->GetElementById<paf::ui::Button>("versions_button");
	auto version_text = this->GetElementById<paf::ui::Text>("version_text");
	AddClickCallback(download_button, this->OnDownloadButton);
	AddClickCallback(verify_button, this->OnVerifyButton);
	AddClickCallback(versions_button, std::bind(&StartupPage::OpenVersionsList, this));

	wchar_t wbuf[128];
	sce_paf_swprintf(
		wbuf,
		sizeof(wbuf),
		L"Installed Version: %s\nMissing Files: %d",
		StringToWString(EVersionToString(this->version)).c_str(),
		this->missingCount
	);
	version_text->SetString(wbuf);
}

void StartupPage::OpenVersionsList()
{
	auto listView = dialog::OpenListView(this->plugin, L"Select Version");

	listView->SetItemFactory(new VersionItemFactory(this->plugin, this->version, [this](EVersion version) {
		dialog::Close();
		this->OnChangeVersion(version);
	}));
	listView->InsertSegment(0, 1);
	listView->SetCellSizeDefault(0, {240, 70, 0, 0});
	listView->SetSegmentLayoutType(0, paf::ui::ListView::LAYOUT_TYPE_LIST);
	listView->InsertCell(0, 0, (int) EVersion::Spanish);
}
