﻿// Copyright (c) Microsoft Corporation
// The Microsoft Corporation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Threading;
using Microsoft.PowerToys.Settings.UI.OOBE.Enums;
using Microsoft.PowerToys.Settings.UI.OOBE.ViewModel;
using Microsoft.PowerToys.Settings.UI.Views;
using Windows.UI.Xaml.Controls;

namespace Microsoft.PowerToys.Settings.UI.OOBE.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class OobeShortcutGuide : Page
    {
        public OobePowerToysModule ViewModel { get; set; }

        public OobeShortcutGuide()
        {
            this.InitializeComponent();
            ViewModel = new OobePowerToysModule(OobeShellPage.OobeShellHandler.Modules[(int)PowerToysModulesEnum.ShortcutGuide]);
            DataContext = ViewModel;
        }

        private void Start_ShortcutGuide_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            using (var eventHandle = new EventWaitHandle(false, EventResetMode.AutoReset, OobeShellPage.ShortcutGuideSharedEvent()))
            {
                eventHandle.Set();
            }

            ViewModel.LogRunningModuleEvent();
        }

        private void SettingsLaunchButton_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            OobeShellPage.OpenMainWindowCallback(typeof(ShortcutGuidePage));
            ViewModel.LogOpeningSettingsEvent();
        }
    }
}
