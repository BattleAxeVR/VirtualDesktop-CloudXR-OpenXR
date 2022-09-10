﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace companion
{
    public partial class ExperimentalSettings : Form
    {
        private bool loading = true;

        public ExperimentalSettings()
        {
            InitializeComponent();

            SuspendLayout();

            Microsoft.Win32.RegistryKey key = null;

            // Read the PimaxXR configuration.
            try
            {
                key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey(MainForm.RegPrefix);

                // Must match the defaults in the runtime!
                enableFrameTiming.Checked = (int)key.GetValue("use_frame_timing_override", 1) == 1 ? true : false;
                filterLength.Value = (int)key.GetValue("frame_time_filter_length", 5);
                var multiplier = (int)key.GetValue("frame_time_override_multiplier", 0);
                if (multiplier == 1000)
                {

                    forceHalf.Checked = true;
                }
                else if(multiplier == 2000)
                {
                    forceThird.Checked = true;
                }
                else
                {
                    forceHalf.Checked = forceThird.Checked = true;
                }
                // Convert value from microseconds to tenth of milliseconds.
                timingBias.Value = multiplier == 0 ? ((int)key.GetValue("frame_time_override_offset", 0) / 100) : 0;
            }
            catch (Exception)
            {
                MessageBox.Show(this, "Failed to write to registry. Please make sure the app is running elevated.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }

            RefreshEnabledState();
            filterLength_Scroll(null, null);
            timingBias_Scroll(null, null);

            ResumeLayout();

            loading = false;
        }

        private void ExperimentalSettings_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                e.Cancel = true;
                Hide();
            }
        }

        private void RefreshEnabledState()
        {
            forceRateLabel.Enabled = forceHalf.Enabled = forceThird.Enabled = enableFrameTiming.Checked;
            filterLength.Enabled = filterLengthLabel.Enabled = filterLengthValue.Enabled =
                timingBias.Enabled = timingBiasLabel.Enabled = timingBiasValue.Enabled = enableFrameTiming.Checked && !(forceHalf.Checked || forceThird.Checked);
        }

        private void enableFrameTiming_CheckedChanged(object sender, EventArgs e)
        {
            RefreshEnabledState();

            if (loading)
            {
                return;
            }

            MainForm.WriteSetting("use_frame_timing_override", enableFrameTiming.Checked ? 1 : 0);
        }

        private void filterLength_Scroll(object sender, EventArgs e)
        {
            filterLengthValue.Text = filterLength.Value.ToString();

            if (loading)
            {
                return;
            }

            MainForm.WriteSetting("frame_time_filter_length", filterLength.Value);
        }

        private void timingBias_Scroll(object sender, EventArgs e)
        {
            // Use the input in tenth of milliseconds to allow one decimal.
            timingBiasValue.Text = timingBias.Value != 0 ? (timingBias.Value / 10.0f).ToString("#.##") : "0";

            if (loading)
            {
                return;
            }

            // Store in microseconds.
            MainForm.WriteSetting("frame_time_override_offset", timingBias.Value * 100);
        }

        private void forceHalf_CheckedChanged(object sender, EventArgs e)
        {
            forceThird.Checked = false;
            RefreshEnabledState();

            if (loading)
            {
                return;
            }

            if (forceHalf.Checked)
            {
                // Force 100% frame duration + 1ms.
                MainForm.WriteSetting("frame_time_override_multiplier", 100);
                MainForm.WriteSetting("frame_time_override_offset", 1000);
            }
            else
            {
                MainForm.WriteSetting("frame_time_override_multiplier", 0);
                timingBias_Scroll(null, null);
            }
        }

        private void forceThird_CheckedChanged(object sender, EventArgs e)
        {
            forceHalf.Checked = false;
            RefreshEnabledState();

            if (loading)
            {
                return;
            }

            if (forceThird.Checked)
            {
                // Force 200% frame duration + 1ms.
                MainForm.WriteSetting("frame_time_override_multiplier", 200);
                MainForm.WriteSetting("frame_time_override_offset", 1000);
            }
            else
            {
                MainForm.WriteSetting("frame_time_override_multiplier", 0);
                timingBias_Scroll(null, null);
            }
        }

        private void restoreDefaults_Click(object sender, EventArgs e)
        {
            enableFrameTiming.Checked = true;
            forceHalf.Checked = forceThird.Checked = false;
            filterLength.Value = 5;
            timingBias.Value = 0;

            enableFrameTiming_CheckedChanged(null, null);
            filterLength_Scroll(null, null);
            timingBias_Scroll(null, null);
            forceHalf_CheckedChanged(null, null);
        }
    }
}
