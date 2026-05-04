using System;
using System.Collections.Generic;
using DesktopModbusController.Modbus;

namespace DesktopModbusController.Devices
{
    /// <summary>
    /// Coordinates all read/write operations for devices on the RS485 bus.
    /// All methods are synchronous and blocking — call from a background thread.
    /// </summary>
    public class DeviceManager
    {
        private readonly ModbusClient          _client;
        private readonly List<WaveformDevice>  _devices = new List<WaveformDevice>();

        /// <summary>All devices discovered by the last bus scan.</summary>
        public IReadOnlyList<WaveformDevice> Devices => _devices;

        public DeviceManager(ModbusClient client)
        {
            _client = client ?? throw new ArgumentNullException(nameof(client));
        }

        // ── Bus scan ─────────────────────────────────────────────────────────

        /// <summary>
        /// Probe each Modbus address from <paramref name="fromAddress"/> to
        /// <paramref name="toAddress"/> by reading 1 holding register.
        /// Devices that respond are added to <see cref="Devices"/>.
        /// </summary>
        /// <param name="onAddressScanned">
        /// Called after each address attempt with the address just scanned.
        /// Useful for updating a progress indicator.
        /// </param>
        public void ScanBus(byte fromAddress, byte toAddress,
                            Action<byte> onAddressScanned = null)
        {
            _devices.Clear();
            for (byte addr = fromAddress; addr <= toAddress; addr++)
            {
                try
                {
                    // Probe: read 1 holding register — fast timeout
                    _client.ReadHoldingRegisters(addr, ModbusRegisters.MinFrequency, 1);
                    var device = new WaveformDevice(addr) { Status = DeviceStatus.Online };
                    ReadDeviceStatus(device);
                    _devices.Add(device);
                }
                catch (ModbusTimeoutException)
                {
                    // No device at this address — continue
                }
                catch (ModbusException ex)
                {
                    // Device responded with an error — still exists on the bus
                    var device = new WaveformDevice(addr) { Status = DeviceStatus.Error };
                    _ = ex; // log if needed
                    _devices.Add(device);
                }

                onAddressScanned?.Invoke(addr);
            }
        }

        // ── Status read ──────────────────────────────────────────────────────

        /// <summary>
        /// Read holding registers (config) and input registers (live status)
        /// from the device and update its model properties.
        /// </summary>
        /// <exception cref="ModbusException">On communication error.</exception>
        public void ReadDeviceStatus(WaveformDevice device)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));

            // Read config block (3 holding registers: MinFreq, MaxFreq, OutputEnable)
            ushort[] cfg = _client.ReadHoldingRegisters(
                device.Address, ModbusRegisters.MinFrequency, 3);
            device.MinFrequency_Hz = cfg[0];
            device.MaxFrequency_Hz = cfg[1];
            device.OutputEnabled   = cfg[2] != 0;

            // Read live status (3 input registers: Status, CurrentFreq, CurrentVoltage)
            ushort[] inp = _client.ReadInputRegisters(
                device.Address, ModbusRegisters.DeviceStatus, 3);
            ushort statusFlags = inp[0];
            device.CurrentFrequency_Hz = inp[1];
            device.CurrentVoltage_mV   = inp[2];

            // Map status flags to DeviceStatus enum
            if ((statusFlags & ModbusRegisters.StatusCalInvalid) != 0)
                device.Status = DeviceStatus.CalInvalid;
            else if ((statusFlags & ModbusRegisters.StatusFreqError) != 0)
                device.Status = DeviceStatus.ConfigInvalid;
            else
                device.Status = DeviceStatus.Online;

            device.LastSeen = DateTime.Now;
        }

        // ── Config write ─────────────────────────────────────────────────────

        /// <summary>
        /// Write MinFrequency, MaxFrequency and OutputEnable to the device using FC16.
        /// </summary>
        /// <exception cref="ArgumentException">If MinFrequency >= MaxFrequency.</exception>
        /// <exception cref="ModbusException">On communication error.</exception>
        public void WriteConfig(WaveformDevice device)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));
            if (device.MinFrequency_Hz >= device.MaxFrequency_Hz)
                throw new ArgumentException(
                    "MinFrequency must be less than MaxFrequency.", nameof(device));

            ushort[] values =
            {
                device.MinFrequency_Hz,
                device.MaxFrequency_Hz,
                (ushort)(device.OutputEnabled ? 1 : 0)
            };
            _client.WriteMultipleRegisters(device.Address, ModbusRegisters.MinFrequency, values);
        }

        /// <summary>Set or clear the output enable flag on the device.</summary>
        public void SetOutputEnable(WaveformDevice device, bool enable)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));
            _client.WriteSingleRegister(device.Address, ModbusRegisters.OutputEnable,
                                        (ushort)(enable ? 1 : 0));
            device.OutputEnabled = enable;
        }

        // ── Calibration ──────────────────────────────────────────────────────

        /// <summary>
        /// Read the 20 calibration registers from the device and populate
        /// <see cref="WaveformDevice.CalibrationPoints"/>.
        /// </summary>
        public void ReadCalibrationPoints(WaveformDevice device)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));

            ushort[] regs = _client.ReadHoldingRegisters(
                device.Address,
                ModbusRegisters.CalibrationBase,
                ModbusRegisters.CalibrationCount);

            device.CalibrationPoints.Clear();
            for (int i = 0; i < 10; i++)
            {
                device.CalibrationPoints.Add(new CalibrationPoint
                {
                    Frequency_Hz = regs[i * 2],
                    Voltage_mV   = regs[i * 2 + 1]
                });
            }
        }

        /// <summary>
        /// Write <see cref="WaveformDevice.CalibrationPoints"/> to the device
        /// using FC16 (does NOT trigger EEPROM save — call <see cref="SaveCalibration"/> for that).
        /// </summary>
        public void WriteCalibrationPoints(WaveformDevice device)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));

            ushort[] regs = new ushort[ModbusRegisters.CalibrationCount];
            int count = Math.Min(device.CalibrationPoints.Count, 10);
            for (int i = 0; i < count; i++)
            {
                regs[i * 2]     = device.CalibrationPoints[i].Frequency_Hz;
                regs[i * 2 + 1] = device.CalibrationPoints[i].Voltage_mV;
            }
            _client.WriteMultipleRegisters(device.Address,
                                           ModbusRegisters.CalibrationBase, regs);
        }

        /// <summary>
        /// Write the calibration points to the device and then trigger
        /// the firmware to save them to EEPROM by writing the SaveCalibration register.
        /// </summary>
        public void SaveCalibration(WaveformDevice device)
        {
            if (device == null) throw new ArgumentNullException(nameof(device));
            WriteCalibrationPoints(device);
            _client.WriteSingleRegister(device.Address, ModbusRegisters.SaveCalibration, 1);
        }
    }
}
