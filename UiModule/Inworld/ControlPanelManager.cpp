//$ HEADER_MOD_FILE $ 
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ControlPanelManager.h"

#include "Common/AnchorLayoutManager.h"
#include "Common/UiAction.h"
#include "Common/ControlButtonAction.h"

#include "Inworld/ControlPanel/BackdropWidget.h"
#include "Inworld/ControlPanel/ControlPanelButton.h"
#include "Inworld/ControlPanel/SettingsWidget.h"
#include "Inworld/ControlPanel/LanguageWidget.h"
#include "Inworld/ControlPanel/CacheSettingsWidget.h"
#include "Inworld/ControlPanel/ChangeThemeWidget.h"

#include "UiServiceInterface.h"
#include "Inworld/InworldSceneController.h"

#include <QAction>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    ControlPanelManager::ControlPanelManager(QObject *parent, AnchorLayoutManager *layout_manager) :
        QObject(parent),
        layout_manager_(layout_manager),
        backdrop_widget_(new CoreUi::BackdropWidget()),
        settings_widget_(0),
        language_widget_(0),
        changetheme_widget_(0)
    {
        // Controls panel
        layout_manager_->AddCornerAnchor(backdrop_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        CreateBasicControls();

        // Settings widget
        settings_widget_ = new SettingsWidget(layout_manager_->GetScene(), this);
        ControlButtonAction *settings_action = new ControlButtonAction(GetButtonForType(UiServices::Settings), settings_widget_, this);

        SetHandler(UiServices::Settings, settings_action);
        connect(settings_action, SIGNAL(toggled(bool)), SLOT(ToggleSettingsVisibility(bool)));
        connect(settings_widget_, SIGNAL(Hidden()), SLOT(CheckSettingsButtonStyle()));

        // Adding cache tab
        cache_settings_widget_ = new CacheSettingsWidget(settings_widget_);
        settings_widget_->AddWidget(cache_settings_widget_, "Cache");

        // Adding a language tab.
        language_widget_ = new LanguageWidget(settings_widget_);
        settings_widget_->AddWidget(language_widget_, "Language");

        // Adding change theme tab
        changetheme_widget_ = new ChangeThemeWidget(settings_widget_);
        settings_widget_->AddWidget(changetheme_widget_, "Change theme");
    }

    ControlPanelManager::~ControlPanelManager()
    {
        SAFE_DELETE(settings_widget_);
    }

    // Private 

    void ControlPanelManager::CreateBasicControls()
    {
        QList<UiServices::ControlButtonType> buttons;
        /// @todo: Read from ini
		buttons << UiServices::Notifications /*<< UiServices::Teleport*/ << UiServices::Settings << UiServices::Quit; // << UiServices::Build << UiServices::Ether;

        ControlPanelButton *button = 0;
        previous_button = 0;
        foreach(UiServices::ControlButtonType button_type, buttons)
        {
            // Create the button and anchor in scene
            button = new ControlPanelButton(button_type); 
            if (previous_button)
                layout_manager_->AnchorWidgetsHorizontally(previous_button, button);
            else
                layout_manager_->AddCornerAnchor(button, Qt::TopRightCorner, Qt::TopRightCorner);

            // Add to internal lists
            control_buttons_.append(button);
            if (button_type == UiServices::Notifications || button_type == UiServices::Settings /*|| button_type == UiServices::Teleport*/)
                backdrop_area_buttons_map_[button_type] = button;

            connect(button, SIGNAL(ControlButtonClicked(UiServices::ControlButtonType)), SLOT(ControlButtonClicked(UiServices::ControlButtonType)));
            previous_button = button;
        }
        UpdateBackdrop();
    }

    void ControlPanelManager::UpdateBackdrop()
    {
        qreal width = 0;
        foreach (ControlPanelButton *button, backdrop_area_buttons_map_.values())
            width += button->GetContentWidth();
        backdrop_widget_->SetContentWidth(width);
    }

    void ControlPanelManager::ControlButtonClicked(UiServices::ControlButtonType type)
    {
      if (action_map_.contains(type))
            action_map_[type]->trigger();
    }

    void ControlPanelManager::ToggleSettingsVisibility(bool visible)
    {
		//Show it with Ui
		UiServiceInterface *ui = (dynamic_cast<UiServices::InworldSceneController *>(parent()))->framework_->GetService<UiServiceInterface>();
		if (ui)      
			if (settings_widget_->GetInternalWidget()->isVisible())
				ui->HideWidget(settings_widget_->GetInternalWidget());
			else
				ui->ShowWidget(settings_widget_->GetInternalWidget());//settings_widget_->AnimatedHide();				
				

    }

    void ControlPanelManager::CheckSettingsButtonStyle()
    {
        backdrop_area_buttons_map_[UiServices::Settings]->CheckStyle(false);
    }

    // Public

    void ControlPanelManager::SetHandler(UiServices::ControlButtonType type, UiServices::UiAction *action)
    {
        action_map_[type] = action;
    }

    ControlPanelButton *ControlPanelManager::GetButtonForType(UiServices::ControlButtonType type) const
    {
        if (backdrop_area_buttons_map_.contains(type))
            return backdrop_area_buttons_map_[type];
        else
            return 0;
    }

    qreal ControlPanelManager::GetContentHeight() const
    { 
        return backdrop_widget_->GetContentHeight();
    }

    qreal ControlPanelManager::GetContentWidth() const
    { 
        return backdrop_widget_->GetContentWidth();
    }

}
