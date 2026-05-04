using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace DesktopModbusController.Forms
{
    partial class MainForm
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
            this._statusStrip = new System.Windows.Forms.StatusStrip();
            this._tsLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this._tsProgress = new System.Windows.Forms.ToolStripProgressBar();
            this.pnlConn = new System.Windows.Forms.Panel();
            this.flowConn = new System.Windows.Forms.FlowLayoutPanel();
            this.lblPort = new System.Windows.Forms.Label();
            this._cboPort = new System.Windows.Forms.ComboBox();
            this._btnRefreshPorts = new System.Windows.Forms.Button();
            this.lblBaud = new System.Windows.Forms.Label();
            this._cboBaud = new System.Windows.Forms.ComboBox();
            this.lblSep = new System.Windows.Forms.Label();
            this._btnConnect = new System.Windows.Forms.Button();
            this._lblConnStatus = new System.Windows.Forms.Label();
            this.pnlLog = new System.Windows.Forms.Panel();
            this._rtbLog = new System.Windows.Forms.RichTextBox();
            this._btnClearLog = new System.Windows.Forms.Button();
            this._split = new System.Windows.Forms.SplitContainer();
            this._lvDevices = new System.Windows.Forms.ListView();
            this.colAddr = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colStatus = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colMinHz = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colMaxHz = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colOutput = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colFreqHz = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colVoltMv = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnPanelDevices = new System.Windows.Forms.FlowLayoutPanel();
            this._btnScan = new System.Windows.Forms.Button();
            this._btnConfigure = new System.Windows.Forms.Button();
            this._btnToggleOutput = new System.Windows.Forms.Button();
            this._pbVoltage = new System.Windows.Forms.ProgressBar();
            this._lblLiveVolt = new System.Windows.Forms.Label();
            this._lblLiveFreq = new System.Windows.Forms.Label();
            this._lblLiveTitle = new System.Windows.Forms.Label();
            this._statusStrip.SuspendLayout();
            this.pnlConn.SuspendLayout();
            this.flowConn.SuspendLayout();
            this.pnlLog.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._split)).BeginInit();
            this._split.Panel1.SuspendLayout();
            this._split.Panel2.SuspendLayout();
            this._split.SuspendLayout();
            this.btnPanelDevices.SuspendLayout();
            this.SuspendLayout();
            // 
            // _statusStrip
            // 
            this._statusStrip.ImageScalingSize = new System.Drawing.Size(20, 20);
            this._statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this._tsLabel,
            this._tsProgress});
            this._statusStrip.Location = new System.Drawing.Point(0, 699);
            this._statusStrip.Name = "_statusStrip";
            this._statusStrip.Size = new System.Drawing.Size(1111, 26);
            this._statusStrip.TabIndex = 3;
            // 
            // _tsLabel
            // 
            this._tsLabel.Name = "_tsLabel";
            this._tsLabel.Size = new System.Drawing.Size(1096, 20);
            this._tsLabel.Spring = true;
            this._tsLabel.Text = "Ready";
            this._tsLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // _tsProgress
            // 
            this._tsProgress.Name = "_tsProgress";
            this._tsProgress.Size = new System.Drawing.Size(100, 18);
            this._tsProgress.Visible = false;
            // 
            // pnlConn
            // 
            this.pnlConn.Controls.Add(this.flowConn);
            this.pnlConn.Dock = System.Windows.Forms.DockStyle.Top;
            this.pnlConn.Location = new System.Drawing.Point(0, 0);
            this.pnlConn.Name = "pnlConn";
            this.pnlConn.Padding = new System.Windows.Forms.Padding(4);
            this.pnlConn.Size = new System.Drawing.Size(1111, 44);
            this.pnlConn.TabIndex = 2;
            // 
            // flowConn
            // 
            this.flowConn.AutoSize = true;
            this.flowConn.Controls.Add(this.lblPort);
            this.flowConn.Controls.Add(this._cboPort);
            this.flowConn.Controls.Add(this._btnRefreshPorts);
            this.flowConn.Controls.Add(this.lblBaud);
            this.flowConn.Controls.Add(this._cboBaud);
            this.flowConn.Controls.Add(this.lblSep);
            this.flowConn.Controls.Add(this._btnConnect);
            this.flowConn.Controls.Add(this._lblConnStatus);
            this.flowConn.Dock = System.Windows.Forms.DockStyle.Fill;
            this.flowConn.Location = new System.Drawing.Point(4, 4);
            this.flowConn.Name = "flowConn";
            this.flowConn.Size = new System.Drawing.Size(1103, 36);
            this.flowConn.TabIndex = 0;
            this.flowConn.WrapContents = false;
            // 
            // lblPort
            // 
            this.lblPort.AutoSize = true;
            this.lblPort.Location = new System.Drawing.Point(3, 0);
            this.lblPort.Name = "lblPort";
            this.lblPort.Padding = new System.Windows.Forms.Padding(0, 6, 0, 0);
            this.lblPort.Size = new System.Drawing.Size(34, 22);
            this.lblPort.TabIndex = 0;
            this.lblPort.Text = "Port:";
            // 
            // _cboPort
            // 
            this._cboPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this._cboPort.Location = new System.Drawing.Point(43, 3);
            this._cboPort.Name = "_cboPort";
            this._cboPort.Size = new System.Drawing.Size(90, 24);
            this._cboPort.TabIndex = 1;
            // 
            // _btnRefreshPorts
            // 
            this._btnRefreshPorts.Location = new System.Drawing.Point(139, 3);
            this._btnRefreshPorts.Name = "_btnRefreshPorts";
            this._btnRefreshPorts.Size = new System.Drawing.Size(30, 23);
            this._btnRefreshPorts.TabIndex = 2;
            this._btnRefreshPorts.Text = "↻";
            // 
            // lblBaud
            // 
            this.lblBaud.AutoSize = true;
            this.lblBaud.Location = new System.Drawing.Point(175, 0);
            this.lblBaud.Name = "lblBaud";
            this.lblBaud.Padding = new System.Windows.Forms.Padding(0, 6, 0, 0);
            this.lblBaud.Size = new System.Drawing.Size(48, 22);
            this.lblBaud.TabIndex = 3;
            this.lblBaud.Text = "  Baud:";
            // 
            // _cboBaud
            // 
            this._cboBaud.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this._cboBaud.Items.AddRange(new object[] {
            "115200",
            "9600"});
            this._cboBaud.Location = new System.Drawing.Point(229, 3);
            this._cboBaud.Name = "_cboBaud";
            this._cboBaud.Size = new System.Drawing.Size(80, 24);
            this._cboBaud.TabIndex = 4;
            // 
            // lblSep
            // 
            this.lblSep.AutoSize = true;
            this.lblSep.Location = new System.Drawing.Point(315, 0);
            this.lblSep.Name = "lblSep";
            this.lblSep.Size = new System.Drawing.Size(13, 16);
            this.lblSep.TabIndex = 5;
            this.lblSep.Text = "  ";
            // 
            // _btnConnect
            // 
            this._btnConnect.Location = new System.Drawing.Point(334, 3);
            this._btnConnect.Name = "_btnConnect";
            this._btnConnect.Size = new System.Drawing.Size(90, 23);
            this._btnConnect.TabIndex = 6;
            this._btnConnect.Text = "Connect";
            // 
            // _lblConnStatus
            // 
            this._lblConnStatus.AutoSize = true;
            this._lblConnStatus.ForeColor = System.Drawing.Color.Red;
            this._lblConnStatus.Location = new System.Drawing.Point(430, 0);
            this._lblConnStatus.Name = "_lblConnStatus";
            this._lblConnStatus.Size = new System.Drawing.Size(90, 16);
            this._lblConnStatus.TabIndex = 7;
            this._lblConnStatus.Text = "Disconnected";
            this._lblConnStatus.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // pnlLog
            // 
            this.pnlLog.Controls.Add(this._rtbLog);
            this.pnlLog.Controls.Add(this._btnClearLog);
            this.pnlLog.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.pnlLog.Location = new System.Drawing.Point(0, 569);
            this.pnlLog.Name = "pnlLog";
            this.pnlLog.Size = new System.Drawing.Size(1111, 130);
            this.pnlLog.TabIndex = 1;
            // 
            // _rtbLog
            // 
            this._rtbLog.BackColor = System.Drawing.Color.Black;
            this._rtbLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this._rtbLog.Font = new System.Drawing.Font("Consolas", 8F);
            this._rtbLog.ForeColor = System.Drawing.Color.Lime;
            this._rtbLog.Location = new System.Drawing.Point(0, 0);
            this._rtbLog.Name = "_rtbLog";
            this._rtbLog.ReadOnly = true;
            this._rtbLog.Size = new System.Drawing.Size(1031, 130);
            this._rtbLog.TabIndex = 0;
            this._rtbLog.Text = "";
            // 
            // _btnClearLog
            // 
            this._btnClearLog.Dock = System.Windows.Forms.DockStyle.Right;
            this._btnClearLog.Location = new System.Drawing.Point(1031, 0);
            this._btnClearLog.Name = "_btnClearLog";
            this._btnClearLog.Size = new System.Drawing.Size(80, 130);
            this._btnClearLog.TabIndex = 1;
            this._btnClearLog.Text = "Clear Log";
            // 
            // _split
            // 
            this._split.Dock = System.Windows.Forms.DockStyle.Fill;
            this._split.Location = new System.Drawing.Point(0, 44);
            this._split.Name = "_split";
            // 
            // _split.Panel1
            // 
            this._split.Panel1.Controls.Add(this._lvDevices);
            this._split.Panel1.Controls.Add(this.btnPanelDevices);
            // 
            // _split.Panel2
            // 
            this._split.Panel2.Controls.Add(this._pbVoltage);
            this._split.Panel2.Controls.Add(this._lblLiveVolt);
            this._split.Panel2.Controls.Add(this._lblLiveFreq);
            this._split.Panel2.Controls.Add(this._lblLiveTitle);
            this._split.Size = new System.Drawing.Size(1111, 525);
            this._split.SplitterDistance = 370;
            this._split.TabIndex = 0;
            // 
            // _lvDevices
            // 
            this._lvDevices.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colAddr,
            this.colName,
            this.colStatus,
            this.colMinHz,
            this.colMaxHz,
            this.colOutput,
            this.colFreqHz,
            this.colVoltMv});
            this._lvDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this._lvDevices.FullRowSelect = true;
            this._lvDevices.GridLines = true;
            this._lvDevices.HideSelection = false;
            this._lvDevices.Location = new System.Drawing.Point(0, 0);
            this._lvDevices.MultiSelect = false;
            this._lvDevices.Name = "_lvDevices";
            this._lvDevices.Size = new System.Drawing.Size(370, 489);
            this._lvDevices.TabIndex = 0;
            this._lvDevices.UseCompatibleStateImageBehavior = false;
            this._lvDevices.View = System.Windows.Forms.View.Details;
            // 
            // colAddr
            // 
            this.colAddr.Text = "Addr";
            this.colAddr.Width = 45;
            // 
            // colName
            // 
            this.colName.Text = "Name";
            this.colName.Width = 100;
            // 
            // colStatus
            // 
            this.colStatus.Text = "Status";
            this.colStatus.Width = 90;
            // 
            // colMinHz
            // 
            this.colMinHz.Text = "Min Hz";
            this.colMinHz.Width = 65;
            // 
            // colMaxHz
            // 
            this.colMaxHz.Text = "Max Hz";
            this.colMaxHz.Width = 65;
            // 
            // colOutput
            // 
            this.colOutput.Text = "Output";
            this.colOutput.Width = 55;
            // 
            // colFreqHz
            // 
            this.colFreqHz.Text = "Freq (Hz)";
            this.colFreqHz.Width = 75;
            // 
            // colVoltMv
            // 
            this.colVoltMv.Text = "Volt (mV)";
            this.colVoltMv.Width = 75;
            // 
            // btnPanelDevices
            // 
            this.btnPanelDevices.Controls.Add(this._btnScan);
            this.btnPanelDevices.Controls.Add(this._btnConfigure);
            this.btnPanelDevices.Controls.Add(this._btnToggleOutput);
            this.btnPanelDevices.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.btnPanelDevices.Location = new System.Drawing.Point(0, 489);
            this.btnPanelDevices.Name = "btnPanelDevices";
            this.btnPanelDevices.Size = new System.Drawing.Size(370, 36);
            this.btnPanelDevices.TabIndex = 1;
            // 
            // _btnScan
            // 
            this._btnScan.Enabled = false;
            this._btnScan.Location = new System.Drawing.Point(3, 3);
            this._btnScan.Name = "_btnScan";
            this._btnScan.Size = new System.Drawing.Size(90, 23);
            this._btnScan.TabIndex = 0;
            this._btnScan.Text = "Scan Bus";
            // 
            // _btnConfigure
            // 
            this._btnConfigure.Enabled = false;
            this._btnConfigure.Location = new System.Drawing.Point(99, 3);
            this._btnConfigure.Name = "_btnConfigure";
            this._btnConfigure.Size = new System.Drawing.Size(90, 23);
            this._btnConfigure.TabIndex = 1;
            this._btnConfigure.Text = "Configure…";
            // 
            // _btnToggleOutput
            // 
            this._btnToggleOutput.Enabled = false;
            this._btnToggleOutput.Location = new System.Drawing.Point(195, 3);
            this._btnToggleOutput.Name = "_btnToggleOutput";
            this._btnToggleOutput.Size = new System.Drawing.Size(100, 23);
            this._btnToggleOutput.TabIndex = 2;
            this._btnToggleOutput.Text = "Toggle Output";
            // 
            // _pbVoltage
            // 
            this._pbVoltage.Dock = System.Windows.Forms.DockStyle.Top;
            this._pbVoltage.Location = new System.Drawing.Point(0, 64);
            this._pbVoltage.Maximum = 3300;
            this._pbVoltage.Name = "_pbVoltage";
            this._pbVoltage.Size = new System.Drawing.Size(737, 18);
            this._pbVoltage.TabIndex = 0;
            // 
            // _lblLiveVolt
            // 
            this._lblLiveVolt.Dock = System.Windows.Forms.DockStyle.Top;
            this._lblLiveVolt.Location = new System.Drawing.Point(0, 42);
            this._lblLiveVolt.Name = "_lblLiveVolt";
            this._lblLiveVolt.Size = new System.Drawing.Size(737, 22);
            this._lblLiveVolt.TabIndex = 1;
            this._lblLiveVolt.Text = "Voltage:   —";
            // 
            // _lblLiveFreq
            // 
            this._lblLiveFreq.Dock = System.Windows.Forms.DockStyle.Top;
            this._lblLiveFreq.Location = new System.Drawing.Point(0, 20);
            this._lblLiveFreq.Name = "_lblLiveFreq";
            this._lblLiveFreq.Size = new System.Drawing.Size(737, 22);
            this._lblLiveFreq.TabIndex = 2;
            this._lblLiveFreq.Text = "Frequency: —";
            // 
            // _lblLiveTitle
            // 
            this._lblLiveTitle.Dock = System.Windows.Forms.DockStyle.Top;
            this._lblLiveTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Bold);
            this._lblLiveTitle.Location = new System.Drawing.Point(0, 0);
            this._lblLiveTitle.Name = "_lblLiveTitle";
            this._lblLiveTitle.Size = new System.Drawing.Size(737, 20);
            this._lblLiveTitle.TabIndex = 3;
            this._lblLiveTitle.Text = "Live Status";
            // 
            // MainForm
            // 
            this.ClientSize = new System.Drawing.Size(1111, 725);
            this.Controls.Add(this._split);
            this.Controls.Add(this.pnlLog);
            this.Controls.Add(this.pnlConn);
            this.Controls.Add(this._statusStrip);
            this.MinimumSize = new System.Drawing.Size(750, 500);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Jammer Modbus Controller";
            this._statusStrip.ResumeLayout(false);
            this._statusStrip.PerformLayout();
            this.pnlConn.ResumeLayout(false);
            this.pnlConn.PerformLayout();
            this.flowConn.ResumeLayout(false);
            this.flowConn.PerformLayout();
            this.pnlLog.ResumeLayout(false);
            this._split.Panel1.ResumeLayout(false);
            this._split.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this._split)).EndInit();
            this._split.ResumeLayout(false);
            this.btnPanelDevices.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        // ── Field declarations ────────────────────────────────────────────────
        private ComboBox             _cboPort;
        private Button               _btnRefreshPorts;
        private ComboBox             _cboBaud;
        private Button               _btnConnect;
        private Label                _lblConnStatus;
        private ListView             _lvDevices;
        private Button               _btnScan;
        private Button               _btnConfigure;
        private Button               _btnToggleOutput;
        private Label                _lblLiveTitle;
        private Label                _lblLiveFreq;
        private Label                _lblLiveVolt;
        private ProgressBar          _pbVoltage;
        private RichTextBox          _rtbLog;
        private Button               _btnClearLog;
        private StatusStrip          _statusStrip;
        private ToolStripStatusLabel _tsLabel;
        private ToolStripProgressBar _tsProgress;
        private SplitContainer       _split;
        private Panel pnlConn;
        private FlowLayoutPanel flowConn;
        private Label lblPort;
        private Label lblBaud;
        private Label lblSep;
        private Panel pnlLog;
        private ColumnHeader colAddr;
        private ColumnHeader colName;
        private ColumnHeader colStatus;
        private ColumnHeader colMinHz;
        private ColumnHeader colMaxHz;
        private ColumnHeader colOutput;
        private ColumnHeader colFreqHz;
        private ColumnHeader colVoltMv;
        private FlowLayoutPanel btnPanelDevices;
    }
}
