using System;
using System.Collections.Generic;

namespace DesktopModbusController.Devices
{
    /// <summary>One frequency-to-voltage calibration point.</summary>
    public class CalibrationPoint
    {
        /// <summary>Jammer output frequency at this DAC voltage (Hz).</summary>
        public ushort Frequency_Hz  { get; set; }

        /// <summary>Required DAC output voltage to reach Frequency_Hz (mV).</summary>
        public ushort Voltage_mV    { get; set; }
    }

    /// <summary>Device connection / configuration state.</summary>
    public enum DeviceStatus
    {
        Unknown,
        Online,
        Error,
        ConfigInvalid,
        CalInvalid
    }

    /// <summary>Model for one Modbus RTU slave (STM32 waveform node).</summary>
    public class WaveformDevice
    {
        /// <summary>Modbus slave address (1–247).</summary>
        public byte Address { get; }

        /// <summary>Human-readable display name.</summary>
        public string Name { get; set; }

        /// <summary>Minimum sweep frequency (Hz).</summary>
        public ushort MinFrequency_Hz { get; set; }

        /// <summary>Maximum sweep frequency (Hz).</summary>
        public ushort MaxFrequency_Hz { get; set; }

        /// <summary>Whether the waveform output is currently enabled.</summary>
        public bool OutputEnabled { get; set; }

        /// <summary>Last known connection / error state.</summary>
        public DeviceStatus Status { get; set; }

        /// <summary>Current sweep frequency read from the device (Hz).</summary>
        public ushort CurrentFrequency_Hz { get; set; }

        /// <summary>Current DAC output voltage read from the device (mV).</summary>
        public ushort CurrentVoltage_mV { get; set; }

        /// <summary>10-entry frequency/voltage calibration table.</summary>
        public List<CalibrationPoint> CalibrationPoints { get; set; }
            = new List<CalibrationPoint>();

        /// <summary>Timestamp of the last successful communication.</summary>
        public DateTime LastSeen { get; set; }

        public WaveformDevice(byte address)
        {
            Address = address;
            Name    = $"Device {address}";
            Status  = DeviceStatus.Unknown;
        }

        public override string ToString() => $"[{Address}] {Name} — {Status}";
    }
}
