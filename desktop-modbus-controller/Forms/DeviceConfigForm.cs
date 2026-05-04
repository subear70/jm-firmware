/*
 * DeviceConfigForm.cs — Per-device configuration and calibration dialog
 *
 * DESIGNER LAYOUT (implemented manually — no Designer.cs):
 * ┌─────────────────────────────────────────────────────┐
 * │  TabControl                                          │
 * │  ┌──────────────┬───────────────────────────────┐   │
 * │  │ Configuration│ Calibration                   │   │
 * │  ├──────────────┴───────────────────────────────┤   │
 * │  │  Tab 1 — Configuration                       │   │
 * │  │    Min Frequency (Hz): [NUD 1–999]            │   │
 * │  │    Max Frequency (Hz): [NUD 2–1000]           │   │
 * │  │    Output Enable:      [CheckBox]             │   │
 * │  │    [Apply]  [Reset]  [Close]                  │   │
 * │  │    Status label (inline, red/green)           │   │
 * │  ├───────────────────────────────────────────────┤   │
 * │  │  Tab 2 — Calibration                         │   │
 * │  │    DataGridView — 10 rows × 2 cols            │   │
 * │  │    [Read Cal]  [Write Cal]  [Save to EEPROM]  │   │
 * │  │    Status label                               │   │
 * │  └───────────────────────────────────────────────┘   │
 * └─────────────────────────────────────────────────────┘
 */

using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using DesktopModbusController.Devices;
using DesktopModbusController.Modbus;

namespace DesktopModbusController.Forms
{
    public sealed partial class DeviceConfigForm : Form
    {
        private readonly WaveformDevice _device;
        private readonly DeviceManager  _manager;

        public DeviceConfigForm(WaveformDevice device, DeviceManager manager)
        {
            _device  = device  ?? throw new ArgumentNullException(nameof(device));
            _manager = manager ?? throw new ArgumentNullException(nameof(manager));

            InitializeComponent();            // Pre-populate calibration grid with 10 empty rows
            for (int i = 0; i < 10; i++)
                _dgvCal.Rows.Add("0", "0");            Text = $"Configure — {device.Name}  (addr {device.Address})";
            PopulateConfigFields();
        }

        // ── UI construction is in DeviceConfigForm.Designer.cs ───────────────

        // ── Configuration tab logic ───────────────────────────────────────────

        private void PopulateConfigFields()
        {
            _nudMinFreq.Value      = Math.Max(_nudMinFreq.Minimum,
                                     Math.Min(_nudMinFreq.Maximum, _device.MinFrequency_Hz));
            _nudMaxFreq.Value      = Math.Max(_nudMaxFreq.Minimum,
                                     Math.Min(_nudMaxFreq.Maximum, _device.MaxFrequency_Hz));
            _chkOutputEnable.Checked = _device.OutputEnabled;
        }

        private void BtnApply_Click(object sender, EventArgs e)
        {
            // Inline validation
            if (_nudMinFreq.Value >= _nudMaxFreq.Value)
            {
                SetCfgStatus("Min Frequency must be less than Max Frequency.", Color.Red);
                return;
            }

            _device.MinFrequency_Hz = (ushort)_nudMinFreq.Value;
            _device.MaxFrequency_Hz = (ushort)_nudMaxFreq.Value;
            _device.OutputEnabled   = _chkOutputEnable.Checked;

            _btnApply.Enabled = false;
            SetCfgStatus("Writing…", Color.Gray);

            ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.WriteConfig(_device);
                    _manager.ReadDeviceStatus(_device);
                    BeginInvoke(new Action(() =>
                    {
                        PopulateConfigFields();
                        SetCfgStatus("Applied successfully.", Color.Green);
                        _btnApply.Enabled = true;
                    }));
                }
                catch (ModbusException ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        SetCfgStatus($"Error: {ex.Message}", Color.Red);
                        _btnApply.Enabled = true;
                    }));
                }
            });
        }

        private void BtnReset_Click(object sender, EventArgs e)
        {
            _btnReset.Enabled = false;
            SetCfgStatus("Reading…", Color.Gray);
            ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.ReadDeviceStatus(_device);
                    BeginInvoke(new Action(() =>
                    {
                        PopulateConfigFields();
                        SetCfgStatus("Refreshed from device.", Color.Green);
                        _btnReset.Enabled = true;
                    }));
                }
                catch (ModbusException ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        SetCfgStatus($"Error: {ex.Message}", Color.Red);
                        _btnReset.Enabled = true;
                    }));
                }
            });
        }

        private void SetCfgStatus(string msg, Color color)
        {
            _lblCfgStatus.Text      = msg;
            _lblCfgStatus.ForeColor = color;
        }

        // ── Calibration tab logic ─────────────────────────────────────────────

        private void BtnReadCal_Click(object sender, EventArgs e)
        {
            SetCalButtons(false);
            SetCalStatus("Reading calibration data…", Color.Gray);
            ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.ReadCalibrationPoints(_device);
                    BeginInvoke(new Action(() =>
                    {
                        PopulateCalGrid();
                        SetCalStatus("Calibration data read successfully.", Color.Green);
                        SetCalButtons(true);
                    }));
                }
                catch (ModbusException ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        SetCalStatus($"Error: {ex.Message}", Color.Red);
                        SetCalButtons(true);
                    }));
                }
            });
        }

        private void BtnWriteCal_Click(object sender, EventArgs e)
        {
            if (!ReadCalGridToDevice(out string err))
            {
                SetCalStatus($"Validation error: {err}", Color.Red);
                return;
            }
            SetCalButtons(false);
            SetCalStatus("Writing calibration data…", Color.Gray);
            ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.WriteCalibrationPoints(_device);
                    BeginInvoke(new Action(() =>
                    {
                        SetCalStatus("Calibration data written (not yet saved to EEPROM).", Color.DarkOrange);
                        SetCalButtons(true);
                    }));
                }
                catch (ModbusException ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        SetCalStatus($"Error: {ex.Message}", Color.Red);
                        SetCalButtons(true);
                    }));
                }
            });
        }

        private void BtnSaveCal_Click(object sender, EventArgs e)
        {
            if (!ReadCalGridToDevice(out string err))
            {
                SetCalStatus($"Validation error: {err}", Color.Red);
                return;
            }
            SetCalButtons(false);
            SetCalStatus("Saving calibration to EEPROM…", Color.Gray);
            ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.SaveCalibration(_device);
                    BeginInvoke(new Action(() =>
                    {
                        SetCalStatus("Calibration saved to EEPROM.", Color.Green);
                        SetCalButtons(true);
                    }));
                }
                catch (ModbusException ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        SetCalStatus($"Error: {ex.Message}", Color.Red);
                        SetCalButtons(true);
                    }));
                }
            });
        }

        private void PopulateCalGrid()
        {
            for (int i = 0; i < 10; i++)
            {
                if (i < _device.CalibrationPoints.Count)
                {
                    _dgvCal.Rows[i].Cells[0].Value = _device.CalibrationPoints[i].Frequency_Hz.ToString();
                    _dgvCal.Rows[i].Cells[1].Value = _device.CalibrationPoints[i].Voltage_mV.ToString();
                }
                else
                {
                    _dgvCal.Rows[i].Cells[0].Value = "0";
                    _dgvCal.Rows[i].Cells[1].Value = "0";
                }
            }
        }

        /// <summary>Read and validate the grid; populate device.CalibrationPoints.</summary>
        private bool ReadCalGridToDevice(out string error)
        {
            _device.CalibrationPoints.Clear();
            for (int i = 0; i < 10; i++)
            {
                if (!ushort.TryParse(_dgvCal.Rows[i].Cells[0].Value?.ToString(), out ushort freq) ||
                    !ushort.TryParse(_dgvCal.Rows[i].Cells[1].Value?.ToString(), out ushort volt))
                {
                    error = $"Row {i + 1}: values must be integers 0–65535.";
                    return false;
                }
                _device.CalibrationPoints.Add(new CalibrationPoint
                {
                    Frequency_Hz = freq,
                    Voltage_mV   = volt
                });
            }
            error = null;
            return true;
        }

        private void SetCalStatus(string msg, Color color)
        {
            _lblCalStatus.Text      = msg;
            _lblCalStatus.ForeColor = color;
        }

        private void SetCalButtons(bool enabled)
        {
            _btnReadCal.Enabled  = enabled;
            _btnWriteCal.Enabled = enabled;
            _btnSaveCal.Enabled  = enabled;
        }
    }
}
