#pragma once

#include <paf.h>
#include <psp2/kernel/clib.h>

struct MessageBoxParam {
    paf::string& title;
    paf::string& text;
    paf::string& button1_text;
    std::function<void()> button1_click;
    paf::string& button2_text;
    std::function<void()> button2_click;
};

class Page {
protected:
    paf::ui::Widget* scene;
    paf::IDParam opened_id;
    paf::Plugin* plugin;
public:
    Page() = default;
    virtual ~Page() = default;
    virtual paf::IDParam PageId() = 0;
    virtual void Mount() = 0;

    void Open(paf::Plugin* plugin);
    void Close();

    inline paf::ui::Widget* Root() {
        return this->scene;
    }

    template<class T> T* GetElementById(paf::IDParam const& id_name) {
        T* element = static_cast<T*>(this->scene->FindChild(id_name));
        if(element == nullptr) {
            sceClibPrintf("element: %s not found!!\n", id_name.GetID().c_str());
            sceClibAbort();
        }
        return element;
    }
};

void AddClickCallback(paf::ui::ButtonBase* button, std::function<void()>* func);
