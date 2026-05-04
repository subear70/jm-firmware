using System;

namespace DesktopModbusController.Modbus
{
    /// <summary>Base class for all Modbus communication errors.</summary>
    public class ModbusException : Exception
    {
        /// <summary>Modbus exception code returned by the device (0x01–0x04), or 0 if N/A.</summary>
        public byte ExceptionCode { get; }

        public ModbusException(string message, byte exceptionCode = 0)
            : base(message)
        {
            ExceptionCode = exceptionCode;
        }
    }

    /// <summary>Thrown when a response CRC does not match the computed CRC.</summary>
    public class ModbusCrcException : ModbusException
    {
        public ModbusCrcException()
            : base("Modbus CRC mismatch — possible bus noise or framing error.") { }
    }

    /// <summary>Thrown when no response is received within the configured timeout.</summary>
    public class ModbusTimeoutException : ModbusException
    {
        public ModbusTimeoutException(byte deviceAddress)
            : base($"No response from device 0x{deviceAddress:X2} within timeout.") { }
    }
}
