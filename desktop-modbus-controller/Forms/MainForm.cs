using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using DesktopModbusController.Modbus;

namespace DesktopModbusController.Forms
{
    /// <summary>
    /// Simple Modbus RTU controller window.
    ///
    /// Layout:
    ///   COM port | port speed | device address     (top rows)
    ///   Register list rendered as labelled text boxes
    ///   [Connect] [Disconnect] [Read All] [Write All]
    /// </summary>
    public sealed class MainForm : Form
    {
        // The first 24 holding registers are contiguous. 0x0019 and 0x001A
        // are read/written separately because 0x0018 is the device address.
        private static readonly (ushort Address, string Name)[] RegisterDefs =
        {
            (0x0000, "Min Frequency (MHz)"),
            (0x0001, "Max Frequency (MHz)"),
            (0x0002, "Output Enable (0/1)"),
            (0x0003, "Sweep Rate (kHz)"),
            (0x0004, "Cal 1 Freq (MHz)"),
            (0x0005, "Cal 1 Voltage (mV)"),
            (0x0006, "Cal 2 Freq (MHz)"),
            (0x0007, "Cal 2 Voltage (mV)"),
            (0x0008, "Cal 3 Freq (MHz)"),
            (0x0009, "Cal 3 Voltage (mV)"),
            (0x000A, "Cal 4 Freq (MHz)"),
            (0x000B, "Cal 4 Voltage (mV)"),
            (0x000C, "Cal 5 Freq (MHz)"),
            (0x000D, "Cal 5 Voltage (mV)"),
            (0x000E, "Cal 6 Freq (MHz)"),
            (0x000F, "Cal 6 Voltage (mV)"),
            (0x0010, "Cal 7 Freq (MHz)"),
            (0x0011, "Cal 7 Voltage (mV)"),
            (0x0012, "Cal 8 Freq (MHz)"),
            (0x0013, "Cal 8 Voltage (mV)"),
            (0x0014, "Cal 9 Freq (MHz)"),
            (0x0015, "Cal 9 Voltage (mV)"),
            (0x0016, "Cal 10 Freq (MHz)"),
            (0x0017, "Cal 10 Voltage (mV)"),
            (ModbusRegisters.SweepPauseUs, "Sweep Pause (us)"),
            (ModbusRegisters.WaveformTriangle, "Triangle Wave (0/1)"),
        };

        private const int ContiguousHoldingRegisterCount = 24;
        private static readonly int[] BaudRates = { 9600, 19200, 38400, 57600, 115200 };

        private IModbusClient _client;

        // True while a Modbus transaction is running on a background thread.
        // Blocks re-entrancy and keeps the action buttons disabled.
        private bool _busy;

        private ComboBox _cboPort;
        private ComboBox _cboBaud;
        private NumericUpDown _numAddress;
        private NumericUpDown _numNewAddress;
        private Button _btnSetAddress;
        private Button _btnRefresh;
        private Button _btnConnect;
        private Button _btnDisconnect;
        private Button _btnReadAll;
        private Button _btnWriteAll;
        private Button _btnSaveCsv;
        private Button _btnLoadCsv;
        private TextBox _txtStatus;
        private readonly TextBox[] _regBoxes = new TextBox[RegisterDefs.Length];

        public MainForm()
        {
            BuildUi();
            RefreshPorts();
            UpdateControlStates();
        }

        // ── UI construction ─────────────────────────────────────────────────────

        private void BuildUi()
        {
            Version version = typeof(MainForm).Assembly.GetName().Version;
            Text = $"Modbus Controller v{version.Major}.{version.Minor}.{version.Build}";
            Font = new Font("Segoe UI", 9f);
            ClientSize = new Size(420, 712);
            MinimumSize = new Size(436, 592);
            StartPosition = FormStartPosition.CenterScreen;

            // Connection controls ---------------------------------------------
            var lblPort = new Label { Text = "COM Port:", Location = new Point(12, 15), AutoSize = true };
            _cboPort = new ComboBox
            {
                Location = new Point(90, 12),
                Width = 110,
                DropDownStyle = ComboBoxStyle.DropDownList,
            };
            _btnRefresh = new Button { Text = "↻", Location = new Point(206, 11), Width = 30 };
            _btnRefresh.Click += (s, e) => RefreshPorts();

            var lblBaud = new Label { Text = "Speed:", Location = new Point(12, 47), AutoSize = true };
            _cboBaud = new ComboBox
            {
                Location = new Point(90, 44),
                Width = 110,
                DropDownStyle = ComboBoxStyle.DropDownList,
            };
            foreach (int b in BaudRates)
                _cboBaud.Items.Add(b);
            _cboBaud.SelectedItem = 115200;

            var lblAddr = new Label { Text = "Address:", Location = new Point(246, 47), AutoSize = true };
            _numAddress = new NumericUpDown
            {
                Location = new Point(310, 44),
                Width = 60,
                Minimum = 1,
                Maximum = 247,
                Value = 1,
            };

            // Device-ID configuration (writes holding register 0x0018) --------
            var lblNewId = new Label { Text = "New Device ID:", Location = new Point(12, 79), AutoSize = true };
            _numNewAddress = new NumericUpDown
            {
                Location = new Point(110, 76),
                Width = 60,
                Minimum = 1,
                Maximum = 247,
                Value = 1,
            };
            _btnSetAddress = new Button
            {
                Text = "Set ID",
                Location = new Point(180, 74),
                Width = 90,
                Height = 26,
            };
            _btnSetAddress.Click += OnSetAddress;

            // Register list ---------------------------------------------------
            var regPanel = new Panel
            {
                Location = new Point(12, 112),
                Size = new Size(396, 420),
                AutoScroll = true,
                BorderStyle = BorderStyle.FixedSingle,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
            };

            int y = 8;
            for (int i = 0; i < RegisterDefs.Length; i++)
            {
                var def = RegisterDefs[i];
                var lbl = new Label
                {
                    Text = $"0x{def.Address:X4}  {def.Name}",
                    Location = new Point(8, y + 3),
                    AutoSize = true,
                };
                var box = new TextBox
                {
                    Location = new Point(250, y),
                    Width = 100,
                    Text = "0",
                    Anchor = AnchorStyles.Top | AnchorStyles.Right,
                };
                _regBoxes[i] = box;
                regPanel.Controls.Add(lbl);
                regPanel.Controls.Add(box);
                y += 28;
            }

            // Action buttons --------------------------------------------------
            _btnConnect = new Button { Text = "Connect", Size = new Size(90, 30) };
            _btnDisconnect = new Button { Text = "Disconnect", Size = new Size(90, 30) };
            _btnReadAll = new Button { Text = "Read All", Size = new Size(90, 30) };
            _btnWriteAll = new Button { Text = "Write All", Size = new Size(90, 30) };
            _btnSaveCsv = new Button { Text = "Save Device Settings", Size = new Size(140, 30) };
            _btnLoadCsv = new Button { Text = "Open Device Settings", Size = new Size(140, 30) };

            _btnConnect.Click += OnConnect;
            _btnDisconnect.Click += OnDisconnect;
            _btnReadAll.Click += OnReadAll;
            _btnWriteAll.Click += OnWriteAll;
            _btnSaveCsv.Click += OnSaveCsv;
            _btnLoadCsv.Click += OnLoadCsv;

            var buttonFlow = new FlowLayoutPanel
            {
                Location = new Point(12, 540),
                Size = new Size(396, 80),
                FlowDirection = FlowDirection.LeftToRight,
                Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
            };
            buttonFlow.Controls.Add(_btnConnect);
            buttonFlow.Controls.Add(_btnDisconnect);
            buttonFlow.Controls.Add(_btnReadAll);
            buttonFlow.Controls.Add(_btnWriteAll);
            buttonFlow.Controls.Add(_btnSaveCsv);
            buttonFlow.Controls.Add(_btnLoadCsv);

            _txtStatus = new TextBox
            {
                Text = "Disconnected",
                Location = new Point(12, 628),
                Size = new Size(396, 60),
                Multiline = true,
                ReadOnly = true,
                ScrollBars = ScrollBars.Vertical,
                BackColor = SystemColors.Window,
                ForeColor = Color.DimGray,
                Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
            };

            Controls.Add(lblPort);
            Controls.Add(_cboPort);
            Controls.Add(_btnRefresh);
            Controls.Add(lblBaud);
            Controls.Add(_cboBaud);
            Controls.Add(lblAddr);
            Controls.Add(_numAddress);
            Controls.Add(lblNewId);
            Controls.Add(_numNewAddress);
            Controls.Add(_btnSetAddress);
            Controls.Add(regPanel);
            Controls.Add(buttonFlow);
            Controls.Add(_txtStatus);
        }

        // ── Event handlers ───────────────────────────────────────────────────────

        private void RefreshPorts()
        {
            string current = _cboPort.SelectedItem as string;
            _cboPort.Items.Clear();
            foreach (string name in SerialPort.GetPortNames())
                _cboPort.Items.Add(name);

            if (current != null && _cboPort.Items.Contains(current))
                _cboPort.SelectedItem = current;
            else if (_cboPort.Items.Count > 0)
                _cboPort.SelectedIndex = 0;
        }

        private void OnConnect(object sender, EventArgs e)
        {
            try
            {
                if (!(_cboPort.SelectedItem is string portName))
                {
                    SetStatus("Select a COM port first.", true);
                    return;
                }
                int baud = (int)_cboBaud.SelectedItem;
                _client = new ModbusClient(portName, baud);

                _client.Connect();
                SetStatus("Connected.", false);
            }
            catch (Exception ex)
            {
                _client?.Dispose();
                _client = null;
                ShowError("Connect failed: " + ex.Message);
            }
            UpdateControlStates();
        }

        private void OnDisconnect(object sender, EventArgs e)
        {
            try
            {
                _client?.Disconnect();
                _client?.Dispose();
                _client = null;
                SetStatus("Disconnected.", false);
            }
            catch (Exception ex)
            {
                ShowError("Disconnect failed: " + ex.Message);
            }
            UpdateControlStates();
        }

        private void OnReadAll(object sender, EventArgs e)
        {
            if (_client == null) return;
            try
            {
                byte addr = (byte)_numAddress.Value;
                ushort[] values = _client.ReadHoldingRegisters(
                    addr, RegisterDefs[0].Address, ContiguousHoldingRegisterCount);

                for (int i = 0; i < ContiguousHoldingRegisterCount; i++)
                    _regBoxes[i].Text = values[i].ToString(CultureInfo.InvariantCulture);
                for (int i = ContiguousHoldingRegisterCount; i < RegisterDefs.Length; i++)
                {
                    ushort value = _client.ReadHoldingRegisters(addr, RegisterDefs[i].Address, 1)[0];
                    if (!IsRegisterValueValid(RegisterDefs[i].Address, value))
                        throw new InvalidDataException($"Device returned invalid {RegisterDefs[i].Name} value {value}.");
                    _regBoxes[i].Text = value.ToString(CultureInfo.InvariantCulture);
                }

                SetStatus($"Read {RegisterDefs.Length} holding registers.", false);
            }
            catch (Exception ex)
            {
                ShowError("Read failed: " + ex.Message);
            }
        }

        private void OnWriteAll(object sender, EventArgs e)
        {
            if (_client == null) return;

            var values = new ushort[_regBoxes.Length];
            for (int i = 0; i < _regBoxes.Length; i++)
            {
                if (!ushort.TryParse(_regBoxes[i].Text.Trim(),
                        NumberStyles.Integer, CultureInfo.InvariantCulture, out values[i]))
                {
                    ShowError($"Invalid value for {RegisterDefs[i].Name} (0–65535).");
                    _regBoxes[i].Focus();
                    _regBoxes[i].SelectAll();
                    return;
                }
                if (!IsRegisterValueValid(RegisterDefs[i].Address, values[i]))
                {
                    string allowed = RegisterDefs[i].Address == ModbusRegisters.SweepPauseUs
                        ? $"0–{ModbusRegisters.MaxSweepPauseUs} us"
                        : "0 or 1";
                    ShowError($"{RegisterDefs[i].Name} must be {allowed}.");
                    _regBoxes[i].Focus();
                    _regBoxes[i].SelectAll();
                    return;
                }
            }

            try
            {
                byte addr = (byte)_numAddress.Value;
                var bulkValues = new ushort[ContiguousHoldingRegisterCount];
                Array.Copy(values, bulkValues, ContiguousHoldingRegisterCount);
                _client.WriteMultipleRegisters(addr, RegisterDefs[0].Address, bulkValues);
                for (int i = ContiguousHoldingRegisterCount; i < values.Length; i++)
                {
                    try
                    {
                        _client.WriteSingleRegister(addr, RegisterDefs[i].Address, values[i]);
                    }
                    catch (Exception ex)
                    {
                        ShowError($"Registers 0x0000–0x0017 were written, but {RegisterDefs[i].Name} could not be updated: {ex.Message}");
                        return;
                    }
                }
                SetStatus($"Wrote {values.Length} holding registers.", false);
            }
            catch (Exception ex)
            {
                ShowError("Write failed: " + ex.Message);
            }
        }

        /// <summary>
        /// Writes the persisted device-address register (0x0018) on the slave.
        /// The slave stores the new address to EEPROM and answers this request
        /// from its current address, so the target address is updated to the new
        /// value afterwards for subsequent transactions.
        /// </summary>
        private async void OnSetAddress(object sender, EventArgs e)
        {
            if (_client == null || _busy) return;

            byte currentAddr = (byte)_numAddress.Value;
            byte newAddr = (byte)_numNewAddress.Value;

            if (newAddr == currentAddr)
            {
                SetStatus($"Device ID already {newAddr}.", false);
                return;
            }

            SetBusy(true);
            SetStatus("Setting device ID\u2026", false);
            try
            {
                IModbusClient client = _client;
                await Task.Run(() => client.WriteSingleRegister(
                    currentAddr, ModbusRegisters.DeviceAddress, newAddr));
                _numAddress.Value = newAddr;
                SetStatus($"Device ID changed {currentAddr} \u2192 {newAddr}. Target address updated.", false);
            }
            catch (Exception ex)
            {
                ShowError("Set device ID failed: " + ex.Message);
            }
            finally
            {
                SetBusy(false);
            }
        }

        // ── CSV save / load ──────────────────────────────────────────────────

        /// <summary>Saves the register values currently shown in the UI to a CSV
        /// file (columns: Address, Name, Value). Works offline — no device
        /// connection required.</summary>
        private void OnSaveCsv(object sender, EventArgs e)
        {
            using (var dlg = new SaveFileDialog
            {
                Filter = "CSV files (*.csv)|*.csv|All files (*.*)|*.*",
                FileName = "registers.csv",
                Title = "Save register values",
            })
            {
                if (dlg.ShowDialog(this) != DialogResult.OK) return;

                try
                {
                    var sb = new StringBuilder();
                    sb.AppendLine("Address,Name,Value");
                    int saved = 0;
                    for (int i = 0; i < RegisterDefs.Length; i++)
                    {
                        // Output Enable is volatile runtime state, not a stored
                        // setting — leave it out of the device-settings file.
                        if (RegisterDefs[i].Address == ModbusRegisters.OutputEnable) continue;

                        string val = _regBoxes[i].Text.Trim();
                        sb.AppendLine($"0x{RegisterDefs[i].Address:X4},{RegisterDefs[i].Name},{val}");
                        saved++;
                    }
                    File.WriteAllText(dlg.FileName, sb.ToString());
                    SetStatus($"Saved {saved} registers to {Path.GetFileName(dlg.FileName)}.", false);
                }
                catch (Exception ex)
                {
                    ShowError("Save CSV failed: " + ex.Message);
                }
            }
        }

        /// <summary>Loads register values from a CSV file into the UI boxes,
        /// matching each row to a register by address. Does not write to the
        /// device — press Write All afterwards to send them.</summary>
        private void OnLoadCsv(object sender, EventArgs e)
        {
            using (var dlg = new OpenFileDialog
            {
                Filter = "CSV files (*.csv)|*.csv|All files (*.*)|*.*",
                Title = "Load register values",
            })
            {
                if (dlg.ShowDialog(this) != DialogResult.OK) return;

                try
                {
                    var indexByAddress = new Dictionary<ushort, int>();
                    for (int i = 0; i < RegisterDefs.Length; i++)
                        indexByAddress[RegisterDefs[i].Address] = i;

                    int applied = 0;
                    foreach (string raw in File.ReadAllLines(dlg.FileName))
                    {
                        string line = raw.Trim();
                        if (line.Length == 0) continue;

                        string[] parts = line.Split(',');
                        if (parts.Length < 2) continue;

                        string addrText = parts[0].Trim();
                        if (addrText.Equals("Address", StringComparison.OrdinalIgnoreCase)) continue; // header

                        if (!TryParseAddress(addrText, out ushort address)) continue;

                        // Value is the last field, so a comma inside the name column is harmless.
                        string valText = parts[parts.Length - 1].Trim();
                        if (!ushort.TryParse(valText, NumberStyles.Integer,
                                CultureInfo.InvariantCulture, out ushort value))
                            continue;

                        if (!indexByAddress.TryGetValue(address, out int idx)) continue;
                        if (!IsRegisterValueValid(address, value)) continue;

                        _regBoxes[idx].Text = value.ToString(CultureInfo.InvariantCulture);
                        applied++;
                    }
                    SetStatus($"Loaded {applied} register value(s) from {Path.GetFileName(dlg.FileName)}.", false);
                }
                catch (Exception ex)
                {
                    ShowError("Load CSV failed: " + ex.Message);
                }
            }
        }

        /// <summary>Parses a register address in either hex ("0x0004") or decimal form.</summary>
        private static bool TryParseAddress(string text, out ushort address)
        {
            text = text.Trim();
            if (text.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
                return ushort.TryParse(text.Substring(2), NumberStyles.HexNumber,
                    CultureInfo.InvariantCulture, out address);
            return ushort.TryParse(text, NumberStyles.Integer,
                CultureInfo.InvariantCulture, out address);
        }

        private static bool IsRegisterValueValid(ushort address, ushort value)
        {
            if (address == ModbusRegisters.SweepPauseUs)
                return value <= ModbusRegisters.MaxSweepPauseUs;
            if (address == ModbusRegisters.WaveformTriangle)
                return value <= 1U;
            return true;
        }

        // ── Helpers ────────────────────────────────────────────────────────────────

        private void UpdateControlStates()
        {
            bool connected = _client != null && _client.IsConnected;
            _btnConnect.Enabled = !connected && !_busy;
            _btnDisconnect.Enabled = connected && !_busy;
            _btnReadAll.Enabled = connected && !_busy;
            _btnWriteAll.Enabled = connected && !_busy;
            _btnSetAddress.Enabled = connected && !_busy;
            _cboPort.Enabled = !connected && !_busy;
            _cboBaud.Enabled = !connected && !_busy;
            _btnRefresh.Enabled = !connected && !_busy;
        }

        /// <summary>Marks a background Modbus transaction as in progress and
        /// refreshes control enablement so the UI stays responsive but blocks
        /// re-entrant requests.</summary>
        private void SetBusy(bool busy)
        {
            _busy = busy;
            UseWaitCursor = busy;
            UpdateControlStates();
        }

        private void SetStatus(string message, bool isError)
        {
            _txtStatus.Text = message;
            _txtStatus.ForeColor = isError ? Color.Firebrick : Color.DimGray;
        }

        /// <summary>Shows an error both in the status line and as a modal dialog
        /// so the user is clearly made aware of the failure.</summary>
        private void ShowError(string message)
        {
            SetStatus(message, true);
            MessageBox.Show(this, message, "Modbus Error",
                MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            _client?.Dispose();
            base.OnFormClosing(e);
        }
    }
}
