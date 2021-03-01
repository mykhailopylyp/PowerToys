// Copyright (c) Microsoft Corporation
// The Microsoft Corporation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Windows;
using System.Windows.Input;
using Microsoft.PowerToys.Settings.UI.OOBE.Views;

namespace PowerToys.Settings
{
    /// <summary>
    /// Interaction logic for OobeWindow.xaml
    /// </summary>
    public partial class OobeWindow : Window
    {
        private static Window inst;

        public static bool IsOpened
        {
            get
            {
                return inst != null;
            }
        }

        public OobeWindow()
        {
            InitializeComponent();
            OobeShellPage.SetCloseOobeWindowCallback(() =>
            {
                App.Current.Dispatcher.BeginInvoke(new Action(() =>
                {
                    inst.Close();
                }));
            });
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            inst = null;
            MainWindow.CloseHiddenWindow();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            if (inst != null)
            {
                inst.Close();
            }

            inst = this;
        }

        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            this.DragMove();
        }
    }
}
