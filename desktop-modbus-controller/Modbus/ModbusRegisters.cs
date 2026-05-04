namespace DesktopModbusController.Modbus
{
    /// <summary>
    /// Modbus RTU register address constants.
    /// Must stay in sync with the STM32 firmware register map.
    /// </summary>
    public static class ModbusRegisters
    {
        // ── Holding registers (FC03 / FC06 / FC16) ───────────────────────────
        /// <summary>Minimum sweep frequency (Hz). R/W.</summary>
        public const ushort MinFrequency      = 0x0000;

        /// <summary>Maximum sweep frequency (Hz). R/W.</summary>
        public const ushort MaxFrequency      = 0x0001;

        /// <summary>Output enable: 0 = off, 1 = on. R/W.</summary>
        public const ushort OutputEnable      = 0x0002;

        /// <summary>Write any value to trigger EEPROM save of calibration data. W only.</summary>
        public const ushort SaveCalibration   = 0x0003;

        /// <summary>
        /// Base address of the 20-register calibration block.
        /// Layout: [freq0, v0, freq1, v1, ..., freq9, v9]
        /// </summary>
        public const ushort CalibrationBase   = 0x0004;

        /// <summary>Total number of registers in the calibration block (10 freq/voltage pairs).</summary>
        public const ushort CalibrationCount  = 20;

        // ── Input registers (FC04) ────────────────────────────────────────────
        /// <summary>Device status bit-flags. R only.</summary>
        public const ushort DeviceStatus      = 0x0000;

        /// <summary>Current output frequency (Hz). R only.</summary>
        public const ushort CurrentFrequency  = 0x0001;

        /// <summary>Current DAC output voltage (mV). R only.</summary>
        public const ushort CurrentVoltage    = 0x0002;

        // ── Status bit masks ─────────────────────────────────────────────────
        public const ushort StatusRunning     = 0x0001;
        public const ushort StatusConfigValid = 0x0002;
        public const ushort StatusFreqError   = 0x0004;
        public const ushort StatusCalInvalid  = 0x0008;
    }
}
