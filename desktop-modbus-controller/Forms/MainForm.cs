/*
 * MainForm.cs — Main application window
 *
 * DESIGNER LAYOUT (implemented manually below — no Designer.cs file):
 * ┌─────────────────────────────────────────────────────────────┐
 * │  [cboPort v] [Refresh]  Baud: [cbo v]  [Connect/Disconnect] │  ← pnlConnection (top, Height=40)
 * ├───────────────────────────────┬─────────────────────────────┤
 * │  ListView (devices)           │  Live Status panel           │
 * │  [Scan Bus] [Configure] [Tog] │  Labels: Freq / Voltage      │
 * │                               │  ProgressBar: voltage        │
 * ├───────────────────────────────┴─────────────────────────────┤
 * │  RichTextBox (log)                          [Clear Log]      │  ← pnlLog (bottom, Height=120)
 * └─────────────────────────────────────────────────────────────┘
 */

using System;
using System.Drawing;
using System.IO.Ports;
using System.Windows.Forms;
using DesktopModbusController.Devices;
using DesktopModbusController.Modbus;
using DesktopModbusController.Services;

namespace DesktopModbusController.Forms
{
    public sealed partial class MainForm : Form
    {
        // ── Business objects ──────────────────────────────────────────────────
        private ModbusClient    _client;
        private DeviceManager   _manager;
        private PollingService  _poller;

        public MainForm()
        {
            InitializeComponent();
            RefreshPortList();
            SetConnectedState(false);
        }

        // ── UI construction is in MainForm.Designer.cs ─────────────────────

        // ── Connection logic ──────────────────────────────────────────────────

        private void BtnConnect_Click(object sender, EventArgs e)
        {
            if (_client != null && _client.IsConnected)
                Disconnect();
            else
                Connect();
        }

        private void Connect()
        {
            try
            {
                string port = _cboPort.SelectedItem?.ToString();
                if (string.IsNullOrEmpty(port)) { AppendLog("Select a COM port first."); return; }
                int baud = int.Parse(_cboBaud.SelectedItem.ToString());

                _client  = new ModbusClient(port, baud);
                _manager = new DeviceManager(_client);
                _poller  = new PollingService(_manager, OnDevicePolled, AppendLog);

                _client.Connect();
                SetConnectedState(true);
                AppendLog($"Connected to {port} at {baud} baud.");
            }
            catch (Exception ex)
            {
                AppendLog($"[ERROR] Connect: {ex.Message}");
            }
        }

        private void Disconnect()
        {
            _poller?.Stop();
            _client?.Disconnect();
            _client?.Dispose();
            _client  = null;
            _manager = null;
            _poller  = null;
            _lvDevices.Items.Clear();
            SetConnectedState(false);
            AppendLog("Disconnected.");
        }

        // ── Bus scan ──────────────────────────────────────────────────────────

        private void BtnScan_Click(object sender, EventArgs e)
        {
            _btnScan.Enabled = false;
            _tsProgress.Visible = true;
            _tsProgress.Minimum = 1;
            _tsProgress.Maximum = 247;
            _lvDevices.Items.Clear();

            System.Threading.ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.ScanBus(1, 32, addr =>
                    {
                        BeginInvoke(new Action(() =>
                        {
                            _tsLabel.Text       = $"Scanning address {addr}…";
                            _tsProgress.Value   = addr;
                        }));
                    });

                    BeginInvoke(new Action(() =>
                    {
                        PopulateDeviceList();
                        _poller.Start();
                        _tsLabel.Text       = $"Scan complete — {_manager.Devices.Count} device(s) found.";
                        _tsProgress.Visible = false;
                        _btnScan.Enabled    = true;
                    }));
                }
                catch (Exception ex)
                {
                    BeginInvoke(new Action(() =>
                    {
                        AppendLog($"[ERROR] Scan failed: {ex.Message}");
                        _tsProgress.Visible = false;
                        _btnScan.Enabled    = true;
                    }));
                }
            });
        }

        // ── Configure / toggle ────────────────────────────────────────────────

        private void BtnConfigure_Click(object sender, EventArgs e)
        {
            var device = GetSelectedDevice();
            if (device == null) return;
            using (var dlg = new DeviceConfigForm(device, _manager))
                dlg.ShowDialog(this);
            RefreshDeviceRow(device);
        }

        private void BtnToggleOutput_Click(object sender, EventArgs e)
        {
            var device = GetSelectedDevice();
            if (device == null) return;
            System.Threading.ThreadPool.QueueUserWorkItem(_ =>
            {
                try
                {
                    _manager.SetOutputEnable(device, !device.OutputEnabled);
                    BeginInvoke(new Action(() => RefreshDeviceRow(device)));
                }
                catch (Exception ex)
                {
                    BeginInvoke(new Action(() => AppendLog($"[ERROR] Toggle: {ex.Message}")));
                }
            });
        }

        // ── Polling callback ──────────────────────────────────────────────────

        private void OnDevicePolled(WaveformDevice device)
        {
            if (IsDisposed || !IsHandleCreated) return;
            BeginInvoke(new Action(() =>
            {
                RefreshDeviceRow(device);

                var selected = GetSelectedDevice();
                if (selected != null && selected.Address == device.Address)
                {
                    _lblLiveFreq.Text = $"Frequency: {device.CurrentFrequency_Hz} Hz";
                    _lblLiveVolt.Text = $"Voltage:   {device.CurrentVoltage_mV} mV";
                    _pbVoltage.Value  = Math.Min(device.CurrentVoltage_mV, (ushort)3300);
                }
            }));
        }

        // ── ListView helpers ──────────────────────────────────────────────────

        private void PopulateDeviceList()
        {
            _lvDevices.Items.Clear();
            foreach (var d in _manager.Devices)
                _lvDevices.Items.Add(DeviceToListViewItem(d));
        }

        private void RefreshDeviceRow(WaveformDevice device)
        {
            foreach (ListViewItem item in _lvDevices.Items)
            {
                if (item.Tag is WaveformDevice d && d.Address == device.Address)
                {
                    UpdateListViewItem(item, device);
                    return;
                }
            }
            // Device not yet in list — add it
            _lvDevices.Items.Add(DeviceToListViewItem(device));
        }

        private static ListViewItem DeviceToListViewItem(WaveformDevice d)
        {
            var item = new ListViewItem(d.Address.ToString()) { Tag = d };
            UpdateListViewItem(item, d);
            return item;
        }

        private static void UpdateListViewItem(ListViewItem item, WaveformDevice d)
        {
            while (item.SubItems.Count < 8) item.SubItems.Add("");
            item.SubItems[0].Text = d.Address.ToString();
            item.SubItems[1].Text = d.Name;
            item.SubItems[2].Text = d.Status.ToString();
            item.SubItems[3].Text = d.MinFrequency_Hz.ToString();
            item.SubItems[4].Text = d.MaxFrequency_Hz.ToString();
            item.SubItems[5].Text = d.OutputEnabled ? "ON" : "OFF";
            item.SubItems[6].Text = d.CurrentFrequency_Hz.ToString();
            item.SubItems[7].Text = d.CurrentVoltage_mV.ToString();
            item.ForeColor = d.Status == DeviceStatus.Online ? SystemColors.WindowText : Color.Red;
            item.Tag = d;
        }

        private WaveformDevice GetSelectedDevice()
        {
            if (_lvDevices.SelectedItems.Count == 0) return null;
            return _lvDevices.SelectedItems[0].Tag as WaveformDevice;
        }

        private void LvDevices_SelectedIndexChanged(object sender, EventArgs e)
        {
            var device = GetSelectedDevice();
            bool hasSel = device != null;
            _btnConfigure.Enabled    = hasSel;
            _btnToggleOutput.Enabled = hasSel;

            if (device != null)
            {
                _lblLiveFreq.Text = $"Frequency: {device.CurrentFrequency_Hz} Hz";
                _lblLiveVolt.Text = $"Voltage:   {device.CurrentVoltage_mV} mV";
                _pbVoltage.Value  = Math.Min(device.CurrentVoltage_mV, (ushort)3300);
            }
        }

        // ── Utilities ─────────────────────────────────────────────────────────

        private void BtnRefreshPorts_Click(object sender, EventArgs e) => RefreshPortList();

        private void BtnClearLog_Click(object sender, EventArgs e) => _rtbLog.Clear();

        private void RefreshPortList()
        {
            string current = _cboPort.SelectedItem?.ToString();
            _cboPort.Items.Clear();
            _cboPort.Items.AddRange(SerialPort.GetPortNames());
            if (current != null && _cboPort.Items.Contains(current))
                _cboPort.SelectedItem = current;
            else if (_cboPort.Items.Count > 0)
                _cboPort.SelectedIndex = 0;
        }

        private void SetConnectedState(bool connected)
        {
            _btnConnect.Text          = connected ? "Disconnect" : "Connect";
            _lblConnStatus.Text       = connected ? "Connected"  : "Disconnected";
            _lblConnStatus.ForeColor  = connected ? Color.Green  : Color.Red;
            _btnScan.Enabled          = connected;
            _cboPort.Enabled          = !connected;
            _cboBaud.Enabled          = !connected;
            _btnRefreshPorts.Enabled  = !connected;
        }

        private void AppendLog(string message)
        {
            if (InvokeRequired) { BeginInvoke(new Action<string>(AppendLog), message); return; }
            _rtbLog.AppendText($"[{DateTime.Now:HH:mm:ss.fff}] {message}{Environment.NewLine}");
            _rtbLog.ScrollToCaret();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            // MinSize and SplitterDistance must be set after the form is laid out.
            _split.Panel1MinSize = 350;
            _split.Panel2MinSize = 160;
            int desired = 560;
            int max = _split.Width - _split.Panel2MinSize - _split.SplitterWidth;
            _split.SplitterDistance = Math.Max(_split.Panel1MinSize, Math.Min(desired, max));
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            _poller?.Stop();
            _client?.Disconnect();
            _client?.Dispose();
        }
    }
}
