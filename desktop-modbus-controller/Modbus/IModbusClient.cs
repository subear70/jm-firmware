using System;

namespace DesktopModbusController.Modbus
{
    /// <summary>
    /// Abstraction over a Modbus RTU master so that the real serial-port client
    /// (<see cref="ModbusClient"/>) and the simulated demo client
    /// (<see cref="DemoModbusClient"/>) can be used interchangeably.
    /// </summary>
    public interface IModbusClient : IDisposable
    {
        /// <summary>Open the underlying transport.</summary>
        void Connect();

        /// <summary>Close the underlying transport.</summary>
        void Disconnect();

        /// <summary>Whether the transport is currently open.</summary>
        bool IsConnected { get; }

        /// <summary>FC03 — read holding registers.</summary>
        ushort[] ReadHoldingRegisters(byte deviceAddress, ushort startAddress, ushort count);

        /// <summary>FC04 — read input registers.</summary>
        ushort[] ReadInputRegisters(byte deviceAddress, ushort startAddress, ushort count);

        /// <summary>FC06 — write a single holding register.</summary>
        void WriteSingleRegister(byte deviceAddress, ushort address, ushort value);

        /// <summary>FC16 — write multiple consecutive holding registers.</summary>
        void WriteMultipleRegisters(byte deviceAddress, ushort startAddress, ushort[] values);
    }
}
