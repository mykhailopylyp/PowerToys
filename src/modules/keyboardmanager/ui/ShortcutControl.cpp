#include "pch.h"
#include "ShortcutControl.h"
#include "KeyDropDownControl.h"
#include "keyboardmanager/common/KeyboardManagerState.h"
#include "keyboardmanager/common/Helpers.h"
#include "keyboardmanager/dll/Generated Files/resource.h"
#include <common/interop/shared_constants.h>

//Both static members are initialized to null
HWND ShortcutControl::EditShortcutsWindowHandle = nullptr;
KeyboardManagerState* ShortcutControl::keyboardManagerState = nullptr;
// Initialized as new vector
RemapBuffer ShortcutControl::shortcutRemapBuffer;

ShortcutControl::ShortcutControl(StackPanel table, StackPanel row, const int colIndex, TextBox targetApp)
{
    shortcutDropDownStackPanel = StackPanel();
    typeShortcut = Button();
    shortcutControlLayout = StackPanel();
    bool isHybridControl = colIndex == 1 ? true : false;

    shortcutDropDownStackPanel.as<StackPanel>().Spacing(KeyboardManagerConstants::ShortcutTableDropDownSpacing);
    shortcutDropDownStackPanel.as<StackPanel>().Orientation(Windows::UI::Xaml::Controls::Orientation::Horizontal);

    typeShortcut.as<Button>().Content(winrt::box_value(GET_RESOURCE_STRING(IDS_TYPE_BUTTON)));
    typeShortcut.as<Button>().Width(KeyboardManagerConstants::ShortcutTableDropDownWidth);
    typeShortcut.as<Button>().Click([&, table, row, colIndex, isHybridControl, targetApp](winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&) {
        keyboardManagerState->SetUIState(KeyboardManagerUIState::DetectShortcutWindowActivated, EditShortcutsWindowHandle);
        // Using the XamlRoot of the typeShortcut to get the root of the XAML host
        createDetectShortcutWindow(sender, sender.as<Button>().XamlRoot(), *keyboardManagerState, colIndex, table, keyDropDownControlObjects, row, targetApp, isHybridControl, false, EditShortcutsWindowHandle, shortcutRemapBuffer);
    });
    // Set an accessible name for the type shortcut button
    typeShortcut.as<Button>().SetValue(Automation::AutomationProperties::NameProperty(), box_value(GET_RESOURCE_STRING(IDS_TYPE_BUTTON)));

    shortcutControlLayout.as<StackPanel>().Spacing(KeyboardManagerConstants::ShortcutTableDropDownSpacing);

    shortcutControlLayout.as<StackPanel>().Children().Append(typeShortcut.as<Button>());
    shortcutControlLayout.as<StackPanel>().Children().Append(shortcutDropDownStackPanel.as<StackPanel>());
    KeyDropDownControl::AddDropDown(table, row, shortcutDropDownStackPanel.as<StackPanel>(), colIndex, shortcutRemapBuffer, keyDropDownControlObjects, targetApp, isHybridControl, false);
    shortcutControlLayout.as<StackPanel>().UpdateLayout();
}

// Function to set the accessible name of the target App text box
void ShortcutControl::SetAccessibleNameForTextBox(TextBox targetAppTextBox, int rowIndex)
{
    // To set the accessible name of the target App text box by adding the string `All Apps` if the text box is empty, if not the application name is read by narrator.
    std::wstring targetAppTextBoxAccessibleName = GET_RESOURCE_STRING(IDS_AUTOMATIONPROPERTIES_ROW) + std::to_wstring(rowIndex) + L", " + GET_RESOURCE_STRING(IDS_EDITSHORTCUTS_TARGETAPPHEADER);
    if (targetAppTextBox.Text() == L"")
    {
        targetAppTextBoxAccessibleName += GET_RESOURCE_STRING(IDS_EDITSHORTCUTS_ALLAPPS);
    }
    targetAppTextBox.SetValue(Automation::AutomationProperties::NameProperty(), box_value(targetAppTextBoxAccessibleName));
}

// Function to set the accessible names for all the controls in a row
void ShortcutControl::UpdateAccessibleNames(StackPanel sourceColumn, StackPanel mappedToColumn, TextBox targetAppTextBox, Button deleteButton, int rowIndex)
{
    sourceColumn.SetValue(Automation::AutomationProperties::NameProperty(), box_value(GET_RESOURCE_STRING(IDS_AUTOMATIONPROPERTIES_ROW) + std::to_wstring(rowIndex) + L", " + GET_RESOURCE_STRING(IDS_EDITSHORTCUTS_SOURCEHEADER)));
    mappedToColumn.SetValue(Automation::AutomationProperties::NameProperty(), box_value(GET_RESOURCE_STRING(IDS_AUTOMATIONPROPERTIES_ROW) + std::to_wstring(rowIndex) + L", " + GET_RESOURCE_STRING(IDS_EDITSHORTCUTS_TARGETHEADER)));
    ShortcutControl::SetAccessibleNameForTextBox(targetAppTextBox, rowIndex);
    deleteButton.SetValue(Automation::AutomationProperties::NameProperty(), box_value(GET_RESOURCE_STRING(IDS_AUTOMATIONPROPERTIES_ROW) + std::to_wstring(rowIndex) + L", " + GET_RESOURCE_STRING(IDS_DELETE_REMAPPING_BUTTON)));
}

// Function to add a new row to the shortcut table. If the originalKeys and newKeys args are provided, then the displayed shortcuts are set to those values.
void ShortcutControl::AddNewShortcutControlRow(StackPanel& parent, std::vector<std::vector<std::unique_ptr<ShortcutControl>>>& keyboardRemapControlObjects, const Shortcut& originalKeys, const KeyShortcutUnion& newKeys, const std::wstring& targetAppName)
{
    // Textbox for target application
    TextBox targetAppTextBox;

    // Create new ShortcutControl objects dynamically so that we does not get destructed
    std::vector<std::unique_ptr<ShortcutControl>> newrow;
    StackPanel row = StackPanel();
    try
    {
        parent.Children().Append(row);
        newrow.emplace_back(std::make_unique<ShortcutControl>(parent, row, 0, targetAppTextBox));
        newrow.emplace_back(std::make_unique<ShortcutControl>(parent, row, 1, targetAppTextBox));
        keyboardRemapControlObjects.push_back(std::move(newrow));
        row.Padding({ 10, 10, 10, 10 });
        row.Orientation(Orientation::Horizontal);
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    Windows::UI::Xaml::Media::SolidColorBrush brush;

    try
    {
        brush = Windows::UI::Xaml::Application::Current().Resources().Lookup(box_value(L"SystemControlBackgroundListLowBrush")).as<Windows::UI::Xaml::Media::SolidColorBrush>();
        if (keyboardRemapControlObjects.size() % 2)
        {
            row.Background(brush);
        }
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    // ShortcutControl for the original shortcut
    StackPanel origin;
    try
    {
        origin = keyboardRemapControlObjects.back()[0]->getShortcutControl();
        origin.Width(KeyboardManagerConstants::ShortcutOriginColumnWidth);
        row.Children().Append(origin);
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }
    
    // Arrow icon
    FontIcon arrowIcon;
    StackPanel arrowIconContainer;
    try
    {
        arrowIcon.FontFamily(Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
        arrowIcon.Glyph(L"\xE72A");
        arrowIcon.VerticalAlignment(VerticalAlignment::Center);
        arrowIcon.HorizontalAlignment(HorizontalAlignment::Center);
        arrowIconContainer = KeyboardManagerHelper::GetWrapped(arrowIcon, KeyboardManagerConstants::ShortcutArrowColumnWidth).as<StackPanel>();
        arrowIconContainer.Orientation(Orientation::Vertical);
        arrowIconContainer.VerticalAlignment(VerticalAlignment::Center);
        row.Children().Append(arrowIconContainer);    
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    // ShortcutControl for the new shortcut
    StackPanel target;
    try
    {
        target = keyboardRemapControlObjects.back()[1]->getShortcutControl();
        target.Width(KeyboardManagerConstants::ShortcutTargetColumnWidth);
        row.Children().Append(target);

        targetAppTextBox.Width(KeyboardManagerConstants::ShortcutTableDropDownWidth);
        targetAppTextBox.PlaceholderText(KeyboardManagerConstants::DefaultAppName);
        targetAppTextBox.Text(targetAppName);

        // GotFocus handler will be called whenever the user tabs into or clicks on the textbox
        targetAppTextBox.GotFocus([targetAppTextBox](auto const& sender, auto const& e) {
            // Select all text for accessible purpose
            targetAppTextBox.SelectAll();
        });

        // LostFocus handler will be called whenever text is updated by a user and then they click something else or tab to another control. Does not get called if Text is updated while the TextBox isn't in focus (i.e. from code)
        targetAppTextBox.LostFocus([&keyboardRemapControlObjects, parent, row, targetAppTextBox](auto const& sender, auto const& e) {
            // Get index of targetAppTextBox button
            uint32_t rowIndex;
            if (!parent.Children().IndexOf(row, rowIndex))
            {
                return;
            }

            // rowIndex could be out of bounds if the the row got deleted after LostFocus handler was invoked. In this case it should return
            if (rowIndex >= keyboardRemapControlObjects.size())
            {
                return;
            }

            // Validate both set of drop downs
            KeyDropDownControl::ValidateShortcutFromDropDownList(parent, row, keyboardRemapControlObjects[rowIndex][0]->shortcutDropDownStackPanel.as<StackPanel>(), 0, ShortcutControl::shortcutRemapBuffer, keyboardRemapControlObjects[rowIndex][0]->keyDropDownControlObjects, targetAppTextBox, false, false);
            KeyDropDownControl::ValidateShortcutFromDropDownList(parent, row, keyboardRemapControlObjects[rowIndex][1]->shortcutDropDownStackPanel.as<StackPanel>(), 1, ShortcutControl::shortcutRemapBuffer, keyboardRemapControlObjects[rowIndex][1]->keyDropDownControlObjects, targetAppTextBox, true, false);

            // Reset the buffer based on the selected drop down items
            std::get<Shortcut>(shortcutRemapBuffer[rowIndex].first[0]).SetKeyCodes(KeyDropDownControl::GetSelectedCodesFromStackPanel(keyboardRemapControlObjects[rowIndex][0]->shortcutDropDownStackPanel.as<StackPanel>()));
            // second column is a hybrid column

            std::vector<int32_t> selectedKeyCodes = KeyDropDownControl::GetSelectedCodesFromStackPanel(keyboardRemapControlObjects[rowIndex][1]->shortcutDropDownStackPanel.as<StackPanel>());

            // If exactly one key is selected consider it to be a key remap
            if (selectedKeyCodes.size() == 1)
            {
                shortcutRemapBuffer[rowIndex].first[1] = selectedKeyCodes[0];
            }
            else
            {
                Shortcut tempShortcut;
                tempShortcut.SetKeyCodes(selectedKeyCodes);
                // Assign instead of setting the value in the buffer since the previous value may not be a Shortcut
                shortcutRemapBuffer[rowIndex].first[1] = tempShortcut;
            }
            std::wstring newText = targetAppTextBox.Text().c_str();
            std::wstring lowercaseDefAppName = KeyboardManagerConstants::DefaultAppName;
            std::transform(newText.begin(), newText.end(), newText.begin(), towlower);
            std::transform(lowercaseDefAppName.begin(), lowercaseDefAppName.end(), lowercaseDefAppName.begin(), towlower);
            if (newText == lowercaseDefAppName)
            {
                shortcutRemapBuffer[rowIndex].second = L"";
            }
            else
            {
                shortcutRemapBuffer[rowIndex].second = targetAppTextBox.Text().c_str();
            }

            // To set the accessibile name of the target app text box when focus is lost
            ShortcutControl::SetAccessibleNameForTextBox(targetAppTextBox, rowIndex + 1);
        });    
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    // We need two containers in order to align it horizontally and vertically
    StackPanel targetAppHorizontal;
    StackPanel targetAppContainer;
    try
    {
        targetAppHorizontal = KeyboardManagerHelper::GetWrapped(targetAppTextBox, KeyboardManagerConstants::TableTargetAppColWidth).as<StackPanel>();
        targetAppHorizontal.Orientation(Orientation::Horizontal);
        targetAppHorizontal.HorizontalAlignment(HorizontalAlignment::Left);
        targetAppContainer = KeyboardManagerHelper::GetWrapped(targetAppHorizontal, KeyboardManagerConstants::TableTargetAppColWidth).as<StackPanel>();
        targetAppContainer.Orientation(Orientation::Vertical);
        targetAppContainer.VerticalAlignment(VerticalAlignment::Bottom);
        row.Children().Append(targetAppContainer);
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    // Delete row button
    Windows::UI::Xaml::Controls::Button deleteShortcut;
    FontIcon deleteSymbol;
    try
    {
        deleteSymbol.FontFamily(Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
        deleteSymbol.Glyph(L"\xE74D");
        deleteShortcut.Content(deleteSymbol);
        deleteShortcut.Background(Media::SolidColorBrush(Colors::Transparent()));
        deleteShortcut.HorizontalAlignment(HorizontalAlignment::Center);
        deleteShortcut.Click([&, parent, row, brush](winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&) {
            Button currentButton = sender.as<Button>();
            uint32_t rowIndex;
            // Get index of delete button
            UIElementCollection children = parent.Children();
            bool indexFound = children.IndexOf(row, rowIndex);

            // IndexOf could fail if the the row got deleted and the button handler was invoked twice. In this case it should return
            if (!indexFound)
            {
                return;
            }

            for (uint32_t i = rowIndex + 1; i < children.Size(); i++)
            {
                StackPanel row = children.GetAt(i).as<StackPanel>();
                row.Background(i % 2 ? brush : Media::SolidColorBrush(Colors::Transparent()));
                StackPanel sourceCol = row.Children().GetAt(0).as<StackPanel>();
                StackPanel targetCol = row.Children().GetAt(2).as<StackPanel>();
                TextBox targetApp = row.Children().GetAt(3).as<StackPanel>().Children().GetAt(0).as<StackPanel>().Children().GetAt(0).as<TextBox>();
                Button delButton = row.Children().GetAt(4).as<StackPanel>().Children().GetAt(0).as<Button>();
                UpdateAccessibleNames(sourceCol, targetCol, targetApp, delButton, i);
            }

            children.RemoveAt(rowIndex);
            parent.UpdateLayout();
            shortcutRemapBuffer.erase(shortcutRemapBuffer.begin() + rowIndex);
            // delete the SingleKeyRemapControl objects so that they get destructed
            keyboardRemapControlObjects.erase(keyboardRemapControlObjects.begin() + rowIndex);
        });

        // To set the accessible name of the delete button
        deleteShortcut.SetValue(Automation::AutomationProperties::NameProperty(), box_value(GET_RESOURCE_STRING(IDS_DELETE_REMAPPING_BUTTON)));
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    // Add tooltip for delete button which would appear on hover
    ToolTip deleteShortcuttoolTip;
    try
    {
        deleteShortcuttoolTip.Content(box_value(GET_RESOURCE_STRING(IDS_DELETE_REMAPPING_BUTTON)));
        ToolTipService::SetToolTip(deleteShortcut, deleteShortcuttoolTip);

        StackPanel deleteShortcutContainer = StackPanel();
        deleteShortcutContainer.Children().Append(deleteShortcut);
        deleteShortcutContainer.Orientation(Orientation::Vertical);
        deleteShortcutContainer.VerticalAlignment(VerticalAlignment::Center);
        row.Children().Append(deleteShortcutContainer);
        parent.UpdateLayout();
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    try
    {
        // Set accessible names
        UpdateAccessibleNames(keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][0]->getShortcutControl(), keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][1]->getShortcutControl(), targetAppTextBox, deleteShortcut, (int)keyboardRemapControlObjects.size());    
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }

    try
    {
        // Set the shortcut text if the two vectors are not empty (i.e. default args)
        if (originalKeys.IsValidShortcut() && !(newKeys.index() == 0 && std::get<DWORD>(newKeys) == NULL) && !(newKeys.index() == 1 && !std::get<Shortcut>(newKeys).IsValidShortcut()))
        {
            Logger::warn("{} {}", __FILE__, __LINE__);

            // change to load app name
            shortcutRemapBuffer.push_back(std::make_pair<RemapBufferItem, std::wstring>(RemapBufferItem{ Shortcut(), Shortcut() }, std::wstring(targetAppName)));
            KeyDropDownControl::AddShortcutToControl(originalKeys, parent, keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][0]->shortcutDropDownStackPanel.as<StackPanel>(), *keyboardManagerState, 0, keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][0]->keyDropDownControlObjects, shortcutRemapBuffer, row, targetAppTextBox, false, false);

            if (newKeys.index() == 0)
            {
                Logger::warn("{} {}", __FILE__, __LINE__);
                keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][1]->keyDropDownControlObjects[0]->SetSelectedValue(std::to_wstring(std::get<DWORD>(newKeys)));
            }
            else
            {
                Logger::warn("{} {}", __FILE__, __LINE__);
                KeyDropDownControl::AddShortcutToControl(std::get<Shortcut>(newKeys), parent, keyboardRemapControlObjects.back()[1]->shortcutDropDownStackPanel.as<StackPanel>(), *keyboardManagerState, 1, keyboardRemapControlObjects[keyboardRemapControlObjects.size() - 1][1]->keyDropDownControlObjects, shortcutRemapBuffer, row, targetAppTextBox, true, false);
            }
        }
        else
        {
            Logger::warn("{} {}", __FILE__, __LINE__);

            // Initialize both shortcuts as empty shortcuts
            shortcutRemapBuffer.push_back(std::make_pair<RemapBufferItem, std::wstring>(RemapBufferItem{ Shortcut(), Shortcut() }, std::wstring(targetAppName)));
        }
    }
    catch (...)
    {
        Logger::error("{} {}", __FILE__, __LINE__);
        throw -1;
    }
}

// Function to return the stack panel element of the ShortcutControl. This is the externally visible UI element which can be used to add it to other layouts
StackPanel ShortcutControl::getShortcutControl()
{
    return shortcutControlLayout.as<StackPanel>();
}

// Function to create the detect shortcut UI window
void ShortcutControl::createDetectShortcutWindow(winrt::Windows::Foundation::IInspectable const& sender, XamlRoot xamlRoot, KeyboardManagerState& keyboardManagerState, const int colIndex, StackPanel table, std::vector<std::unique_ptr<KeyDropDownControl>>& keyDropDownControlObjects, StackPanel row, TextBox targetApp, bool isHybridControl, bool isSingleKeyWindow, HWND parentWindow, RemapBuffer& remapBuffer)
{
    // ContentDialog for detecting shortcuts. This is the parent UI element.
    ContentDialog detectShortcutBox;

    // ContentDialog requires manually setting the XamlRoot (https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.contentdialog#contentdialog-in-appwindow-or-xaml-islands)
    detectShortcutBox.XamlRoot(xamlRoot);
    detectShortcutBox.Title(box_value(GET_RESOURCE_STRING(IDS_TYPESHORTCUT_TITLE)));
    detectShortcutBox.IsPrimaryButtonEnabled(false);
    detectShortcutBox.IsSecondaryButtonEnabled(false);

    // Get the linked stack panel for the "Type shortcut" button that was clicked
    StackPanel linkedShortcutStackPanel = KeyboardManagerHelper::getSiblingElement(sender).as<StackPanel>();

    auto unregisterKeys = [&keyboardManagerState]() {
        keyboardManagerState.ClearRegisteredKeyDelays();
    };

    auto selectDetectedShortcutAndResetKeys = [&keyboardManagerState](DWORD key) {
        keyboardManagerState.SelectDetectedShortcut(key);
        keyboardManagerState.ResetDetectedShortcutKey(key);
    };

    auto onPressEnter = [linkedShortcutStackPanel,
                         detectShortcutBox,
                         &keyboardManagerState,
                         unregisterKeys,
                         colIndex,
                         table,
                         targetApp,
                         &keyDropDownControlObjects,
                         row,
                         isHybridControl,
                         isSingleKeyWindow,
                         &remapBuffer] {
        // Save the detected shortcut in the linked text block
        Shortcut detectedShortcutKeys = keyboardManagerState.GetDetectedShortcut();

        if (!detectedShortcutKeys.IsEmpty())
        {
            // The shortcut buffer gets set in this function
            KeyDropDownControl::AddShortcutToControl(detectedShortcutKeys, table, linkedShortcutStackPanel, keyboardManagerState, colIndex, keyDropDownControlObjects, remapBuffer, row, targetApp, isHybridControl, isSingleKeyWindow);
        }
        // Hide the type shortcut UI
        detectShortcutBox.Hide();
    };

    auto onReleaseEnter = [&keyboardManagerState,
                           unregisterKeys,
                           isSingleKeyWindow,
                           parentWindow] {
        // Reset the keyboard manager UI state
        keyboardManagerState.ResetUIState();
        if (isSingleKeyWindow)
        {
            // Revert UI state back to Edit Keyboard window
            keyboardManagerState.SetUIState(KeyboardManagerUIState::EditKeyboardWindowActivated, parentWindow);
        }
        else
        {
            // Revert UI state back to Edit Shortcut window
            keyboardManagerState.SetUIState(KeyboardManagerUIState::EditShortcutsWindowActivated, parentWindow);
        }

        unregisterKeys();
    };

    auto onAccept = [onPressEnter,
                     onReleaseEnter] {
        onPressEnter();
        onReleaseEnter();
    };

    TextBlock primaryButtonText;
    primaryButtonText.Text(GET_RESOURCE_STRING(IDS_OK_BUTTON));

    Button primaryButton;
    primaryButton.HorizontalAlignment(HorizontalAlignment::Stretch);
    primaryButton.Margin({ 2, 2, 2, 2 });
    primaryButton.Content(primaryButtonText);

    // OK button
    primaryButton.Click([onAccept](winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&) {
        onAccept();
    });

    // NOTE: UnregisterKeys should never be called on the DelayThread, as it will re-enter the mutex. To avoid this it is run on the dispatcher thread
    keyboardManagerState.RegisterKeyDelay(
        VK_RETURN,
        selectDetectedShortcutAndResetKeys,
        [primaryButton, onPressEnter, detectShortcutBox](DWORD) {
            detectShortcutBox.Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [primaryButton, onPressEnter] {
                    // Use the base medium low brush to be consistent with the theme
                    primaryButton.Background(Windows::UI::Xaml::Application::Current().Resources().Lookup(box_value(L"SystemControlBackgroundBaseMediumLowBrush")).as<Windows::UI::Xaml::Media::SolidColorBrush>());
                    onPressEnter();
                });
        },
        [onReleaseEnter, detectShortcutBox](DWORD) {
            detectShortcutBox.Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [onReleaseEnter]() {
                    onReleaseEnter();
                });
        });

    TextBlock cancelButtonText;
    cancelButtonText.Text(GET_RESOURCE_STRING(IDS_CANCEL_BUTTON));

    auto onCancel = [&keyboardManagerState,
                     detectShortcutBox,
                     unregisterKeys,
                     isSingleKeyWindow,
                     parentWindow] {
        detectShortcutBox.Hide();

        // Reset the keyboard manager UI state
        keyboardManagerState.ResetUIState();
        if (isSingleKeyWindow)
        {
            // Revert UI state back to Edit Keyboard window
            keyboardManagerState.SetUIState(KeyboardManagerUIState::EditKeyboardWindowActivated, parentWindow);
        }
        else
        {
            // Revert UI state back to Edit Shortcut window
            keyboardManagerState.SetUIState(KeyboardManagerUIState::EditShortcutsWindowActivated, parentWindow);
        }
        unregisterKeys();
    };

    Button cancelButton;
    cancelButton.HorizontalAlignment(HorizontalAlignment::Stretch);
    cancelButton.Margin({ 2, 2, 2, 2 });
    cancelButton.Content(cancelButtonText);
    // Cancel button
    cancelButton.Click([onCancel](winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&) {
        onCancel();
    });

    // NOTE: UnregisterKeys should never be called on the DelayThread, as it will re-enter the mutex. To avoid this it is run on the dispatcher thread
    keyboardManagerState.RegisterKeyDelay(
        VK_ESCAPE,
        selectDetectedShortcutAndResetKeys,
        [onCancel, detectShortcutBox](DWORD) {
            detectShortcutBox.Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [onCancel] {
                    onCancel();
                });
        },
        nullptr);

    // StackPanel parent for the displayed text in the dialog
    Windows::UI::Xaml::Controls::StackPanel stackPanel;
    detectShortcutBox.Content(stackPanel);

    // Header textblock
    TextBlock text;
    text.Text(GET_RESOURCE_STRING(IDS_TYPESHORTCUT_HEADER));
    text.Margin({ 0, 0, 0, 10 });
    stackPanel.Children().Append(text);

    // Target StackPanel to place the selected key - first row (for 1-3 keys)
    Windows::UI::Xaml::Controls::StackPanel keyStackPanel1;
    keyStackPanel1.Orientation(Orientation::Horizontal);
    stackPanel.Children().Append(keyStackPanel1);

    // Target StackPanel to place the selected key - second row (for 4-5 keys)
    Windows::UI::Xaml::Controls::StackPanel keyStackPanel2;
    keyStackPanel2.Orientation(Orientation::Horizontal);
    keyStackPanel2.Margin({ 0, 20, 0, 0 });
    keyStackPanel2.Visibility(Visibility::Collapsed);
    stackPanel.Children().Append(keyStackPanel2);

    TextBlock holdEscInfo;
    holdEscInfo.Text(GET_RESOURCE_STRING(IDS_TYPE_HOLDESC));
    holdEscInfo.FontSize(12);
    holdEscInfo.Margin({ 0, 20, 0, 0 });
    stackPanel.Children().Append(holdEscInfo);

    TextBlock holdEnterInfo;
    holdEnterInfo.Text(GET_RESOURCE_STRING(IDS_TYPE_HOLDENTER));
    holdEnterInfo.FontSize(12);
    holdEnterInfo.Margin({ 0, 0, 0, 0 });
    stackPanel.Children().Append(holdEnterInfo);

    ColumnDefinition primaryButtonColumn;
    ColumnDefinition cancelButtonColumn;

    Grid buttonPanel;
    buttonPanel.Margin({ 0, 20, 0, 0 });
    buttonPanel.HorizontalAlignment(HorizontalAlignment::Stretch);
    buttonPanel.ColumnDefinitions().Append(primaryButtonColumn);
    buttonPanel.ColumnDefinitions().Append(cancelButtonColumn);
    buttonPanel.SetColumn(primaryButton, 0);
    buttonPanel.SetColumn(cancelButton, 1);

    buttonPanel.Children().Append(primaryButton);
    buttonPanel.Children().Append(cancelButton);

    stackPanel.Children().Append(buttonPanel);
    stackPanel.UpdateLayout();

    // Configure the keyboardManagerState to store the UI information.
    keyboardManagerState.ConfigureDetectShortcutUI(keyStackPanel1, keyStackPanel2);

    // Show the dialog
    detectShortcutBox.ShowAsync();
}
