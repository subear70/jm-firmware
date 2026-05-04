using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace DesktopModbusController.Forms
{
    partial class DeviceConfigForm
    {
        private IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
                components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.tabs = new System.Windows.Forms.TabControl();
            this.tabCfg = new System.Windows.Forms.TabPage();
            this.tblCfg = new System.Windows.Forms.TableLayoutPanel();
            this.lblMinFreq = new System.Windows.Forms.Label();
            this._nudMinFreq = new System.Windows.Forms.NumericUpDown();
            this.lblMaxFreq = new System.Windows.Forms.Label();
            this._nudMaxFreq = new System.Windows.Forms.NumericUpDown();
            this.lblOutEnable = new System.Windows.Forms.Label();
            this._chkOutputEnable = new System.Windows.Forms.CheckBox();
            this._lblCfgStatus = new System.Windows.Forms.Label();
            this.btnPanelCfg = new System.Windows.Forms.FlowLayoutPanel();
            this._btnApply = new System.Windows.Forms.Button();
            this._btnReset = new System.Windows.Forms.Button();
            this._btnClose = new System.Windows.Forms.Button();
            this.tabCal = new System.Windows.Forms.TabPage();
            this._dgvCal = new System.Windows.Forms.DataGridView();
            this.Frequency_Hz = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Voltage_mV = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.btnPanelCal = new System.Windows.Forms.FlowLayoutPanel();
            this._btnReadCal = new System.Windows.Forms.Button();
            this._btnWriteCal = new System.Windows.Forms.Button();
            this._btnSaveCal = new System.Windows.Forms.Button();
            this._lblCalStatus = new System.Windows.Forms.Label();
            this.tabs.SuspendLayout();
            this.tabCfg.SuspendLayout();
            this.tblCfg.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._nudMinFreq)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._nudMaxFreq)).BeginInit();
            this.btnPanelCfg.SuspendLayout();
            this.tabCal.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._dgvCal)).BeginInit();
            this.btnPanelCal.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabs
            // 
            this.tabs.Controls.Add(this.tabCfg);
            this.tabs.Controls.Add(this.tabCal);
            this.tabs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabs.Location = new System.Drawing.Point(0, 0);
            this.tabs.Name = "tabs";
            this.tabs.SelectedIndex = 0;
            this.tabs.Size = new System.Drawing.Size(422, 433);
            this.tabs.TabIndex = 0;
            // 
            // tabCfg
            // 
            this.tabCfg.Controls.Add(this.tblCfg);
            this.tabCfg.Location = new System.Drawing.Point(4, 25);
            this.tabCfg.Name = "tabCfg";
            this.tabCfg.Size = new System.Drawing.Size(414, 404);
            this.tabCfg.TabIndex = 0;
            this.tabCfg.Text = "Configuration";
            // 
            // tblCfg
            // 
            this.tblCfg.ColumnCount = 2;
            this.tblCfg.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 160F));
            this.tblCfg.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tblCfg.Controls.Add(this.lblMinFreq, 0, 0);
            this.tblCfg.Controls.Add(this._nudMinFreq, 1, 0);
            this.tblCfg.Controls.Add(this.lblMaxFreq, 0, 1);
            this.tblCfg.Controls.Add(this._nudMaxFreq, 1, 1);
            this.tblCfg.Controls.Add(this.lblOutEnable, 0, 2);
            this.tblCfg.Controls.Add(this._chkOutputEnable, 1, 2);
            this.tblCfg.Controls.Add(this._lblCfgStatus, 0, 3);
            this.tblCfg.Controls.Add(this.btnPanelCfg, 0, 4);
            this.tblCfg.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tblCfg.Location = new System.Drawing.Point(0, 0);
            this.tblCfg.Name = "tblCfg";
            this.tblCfg.Padding = new System.Windows.Forms.Padding(8);
            this.tblCfg.RowCount = 5;
            this.tblCfg.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tblCfg.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tblCfg.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tblCfg.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tblCfg.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tblCfg.Size = new System.Drawing.Size(414, 404);
            this.tblCfg.TabIndex = 0;
            // 
            // lblMinFreq
            // 
            this.lblMinFreq.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblMinFreq.AutoSize = true;
            this.lblMinFreq.Location = new System.Drawing.Point(11, 10);
            this.lblMinFreq.Name = "lblMinFreq";
            this.lblMinFreq.Size = new System.Drawing.Size(125, 16);
            this.lblMinFreq.TabIndex = 0;
            this.lblMinFreq.Text = "Min Frequency (Hz):";
            // 
            // _nudMinFreq
            // 
            this._nudMinFreq.Location = new System.Drawing.Point(171, 11);
            this._nudMinFreq.Maximum = new decimal(new int[] {
            999,
            0,
            0,
            0});
            this._nudMinFreq.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this._nudMinFreq.Name = "_nudMinFreq";
            this._nudMinFreq.Size = new System.Drawing.Size(100, 22);
            this._nudMinFreq.TabIndex = 1;
            this._nudMinFreq.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // lblMaxFreq
            // 
            this.lblMaxFreq.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblMaxFreq.AutoSize = true;
            this.lblMaxFreq.Location = new System.Drawing.Point(11, 30);
            this.lblMaxFreq.Name = "lblMaxFreq";
            this.lblMaxFreq.Size = new System.Drawing.Size(129, 16);
            this.lblMaxFreq.TabIndex = 2;
            this.lblMaxFreq.Text = "Max Frequency (Hz):";
            // 
            // _nudMaxFreq
            // 
            this._nudMaxFreq.Location = new System.Drawing.Point(171, 31);
            this._nudMaxFreq.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this._nudMaxFreq.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this._nudMaxFreq.Name = "_nudMaxFreq";
            this._nudMaxFreq.Size = new System.Drawing.Size(100, 22);
            this._nudMaxFreq.TabIndex = 3;
            this._nudMaxFreq.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            // 
            // lblOutEnable
            // 
            this.lblOutEnable.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblOutEnable.AutoSize = true;
            this.lblOutEnable.Location = new System.Drawing.Point(11, 50);
            this.lblOutEnable.Name = "lblOutEnable";
            this.lblOutEnable.Size = new System.Drawing.Size(94, 16);
            this.lblOutEnable.TabIndex = 4;
            this.lblOutEnable.Text = "Output Enable:";
            // 
            // _chkOutputEnable
            // 
            this._chkOutputEnable.AutoSize = true;
            this._chkOutputEnable.Location = new System.Drawing.Point(171, 51);
            this._chkOutputEnable.Name = "_chkOutputEnable";
            this._chkOutputEnable.Size = new System.Drawing.Size(80, 14);
            this._chkOutputEnable.TabIndex = 5;
            this._chkOutputEnable.Text = "Enabled";
            // 
            // _lblCfgStatus
            // 
            this._lblCfgStatus.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this._lblCfgStatus.AutoSize = true;
            this.tblCfg.SetColumnSpan(this._lblCfgStatus, 2);
            this._lblCfgStatus.ForeColor = System.Drawing.Color.Gray;
            this._lblCfgStatus.Location = new System.Drawing.Point(11, 70);
            this._lblCfgStatus.Name = "_lblCfgStatus";
            this._lblCfgStatus.Size = new System.Drawing.Size(48, 16);
            this._lblCfgStatus.TabIndex = 6;
            this._lblCfgStatus.Text = "Ready";
            // 
            // btnPanelCfg
            // 
            this.btnPanelCfg.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.btnPanelCfg.AutoSize = true;
            this.tblCfg.SetColumnSpan(this.btnPanelCfg, 2);
            this.btnPanelCfg.Controls.Add(this._btnApply);
            this.btnPanelCfg.Controls.Add(this._btnReset);
            this.btnPanelCfg.Controls.Add(this._btnClose);
            this.btnPanelCfg.Location = new System.Drawing.Point(11, 227);
            this.btnPanelCfg.Name = "btnPanelCfg";
            this.btnPanelCfg.Size = new System.Drawing.Size(228, 29);
            this.btnPanelCfg.TabIndex = 7;
            // 
            // _btnApply
            // 
            this._btnApply.Location = new System.Drawing.Point(3, 3);
            this._btnApply.Name = "_btnApply";
            this._btnApply.Size = new System.Drawing.Size(70, 23);
            this._btnApply.TabIndex = 0;
            this._btnApply.Text = "Apply";
            // 
            // _btnReset
            // 
            this._btnReset.Location = new System.Drawing.Point(79, 3);
            this._btnReset.Name = "_btnReset";
            this._btnReset.Size = new System.Drawing.Size(70, 23);
            this._btnReset.TabIndex = 1;
            this._btnReset.Text = "Reset";
            // 
            // _btnClose
            // 
            this._btnClose.DialogResult = System.Windows.Forms.DialogResult.OK;
            this._btnClose.Location = new System.Drawing.Point(155, 3);
            this._btnClose.Name = "_btnClose";
            this._btnClose.Size = new System.Drawing.Size(70, 23);
            this._btnClose.TabIndex = 2;
            this._btnClose.Text = "Close";
            // 
            // tabCal
            // 
            this.tabCal.Controls.Add(this._dgvCal);
            this.tabCal.Controls.Add(this.btnPanelCal);
            this.tabCal.Controls.Add(this._lblCalStatus);
            this.tabCal.Location = new System.Drawing.Point(4, 25);
            this.tabCal.Name = "tabCal";
            this.tabCal.Size = new System.Drawing.Size(192, 71);
            this.tabCal.TabIndex = 1;
            this.tabCal.Text = "Calibration";
            // 
            // _dgvCal
            // 
            this._dgvCal.AllowUserToAddRows = false;
            this._dgvCal.AllowUserToDeleteRows = false;
            this._dgvCal.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this._dgvCal.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this._dgvCal.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Frequency_Hz,
            this.Voltage_mV});
            this._dgvCal.Dock = System.Windows.Forms.DockStyle.Fill;
            this._dgvCal.Location = new System.Drawing.Point(0, 0);
            this._dgvCal.Name = "_dgvCal";
            this._dgvCal.RowHeadersWidth = 51;
            this._dgvCal.Size = new System.Drawing.Size(192, 13);
            this._dgvCal.TabIndex = 0;
            // 
            // Frequency_Hz
            // 
            this.Frequency_Hz.HeaderText = "Frequency (Hz)";
            this.Frequency_Hz.MinimumWidth = 6;
            this.Frequency_Hz.Name = "Frequency_Hz";
            // 
            // Voltage_mV
            // 
            this.Voltage_mV.HeaderText = "Voltage (mV)";
            this.Voltage_mV.MinimumWidth = 6;
            this.Voltage_mV.Name = "Voltage_mV";
            // 
            // btnPanelCal
            // 
            this.btnPanelCal.Controls.Add(this._btnReadCal);
            this.btnPanelCal.Controls.Add(this._btnWriteCal);
            this.btnPanelCal.Controls.Add(this._btnSaveCal);
            this.btnPanelCal.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.btnPanelCal.Location = new System.Drawing.Point(0, 13);
            this.btnPanelCal.Name = "btnPanelCal";
            this.btnPanelCal.Size = new System.Drawing.Size(192, 36);
            this.btnPanelCal.TabIndex = 1;
            // 
            // _btnReadCal
            // 
            this._btnReadCal.Location = new System.Drawing.Point(3, 3);
            this._btnReadCal.Name = "_btnReadCal";
            this._btnReadCal.Size = new System.Drawing.Size(90, 23);
            this._btnReadCal.TabIndex = 0;
            this._btnReadCal.Text = "Read Cal";
            // 
            // _btnWriteCal
            // 
            this._btnWriteCal.Location = new System.Drawing.Point(99, 3);
            this._btnWriteCal.Name = "_btnWriteCal";
            this._btnWriteCal.Size = new System.Drawing.Size(90, 23);
            this._btnWriteCal.TabIndex = 1;
            this._btnWriteCal.Text = "Write Cal";
            // 
            // _btnSaveCal
            // 
            this._btnSaveCal.Location = new System.Drawing.Point(3, 32);
            this._btnSaveCal.Name = "_btnSaveCal";
            this._btnSaveCal.Size = new System.Drawing.Size(110, 23);
            this._btnSaveCal.TabIndex = 2;
            this._btnSaveCal.Text = "Save to EEPROM";
            // 
            // _lblCalStatus
            // 
            this._lblCalStatus.Dock = System.Windows.Forms.DockStyle.Bottom;
            this._lblCalStatus.ForeColor = System.Drawing.Color.Gray;
            this._lblCalStatus.Location = new System.Drawing.Point(0, 49);
            this._lblCalStatus.Name = "_lblCalStatus";
            this._lblCalStatus.Size = new System.Drawing.Size(192, 22);
            this._lblCalStatus.TabIndex = 2;
            this._lblCalStatus.Text = "Ready";
            // 
            // DeviceConfigForm
            // 
            this.AcceptButton = this._btnClose;
            this.ClientSize = new System.Drawing.Size(422, 433);
            this.Controls.Add(this.tabs);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimumSize = new System.Drawing.Size(400, 440);
            this.Name = "DeviceConfigForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Configure Device";
            this.tabs.ResumeLayout(false);
            this.tabCfg.ResumeLayout(false);
            this.tblCfg.ResumeLayout(false);
            this.tblCfg.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this._nudMinFreq)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._nudMaxFreq)).EndInit();
            this.btnPanelCfg.ResumeLayout(false);
            this.tabCal.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this._dgvCal)).EndInit();
            this.btnPanelCal.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        // ── Field declarations ────────────────────────────────────────────────
        private NumericUpDown _nudMinFreq;
        private NumericUpDown _nudMaxFreq;
        private CheckBox      _chkOutputEnable;
        private Button        _btnApply;
        private Button        _btnReset;
        private Label         _lblCfgStatus;
        private DataGridView  _dgvCal;
        private Button        _btnReadCal;
        private Button        _btnWriteCal;
        private Button        _btnSaveCal;
        private Label         _lblCalStatus;
        private Button        _btnClose;
        private TabControl tabs;
        private TabPage tabCfg;
        private TableLayoutPanel tblCfg;
        private Label lblMinFreq;
        private Label lblMaxFreq;
        private Label lblOutEnable;
        private FlowLayoutPanel btnPanelCfg;
        private TabPage tabCal;
        private DataGridViewTextBoxColumn Frequency_Hz;
        private DataGridViewTextBoxColumn Voltage_mV;
        private FlowLayoutPanel btnPanelCal;
    }
}
