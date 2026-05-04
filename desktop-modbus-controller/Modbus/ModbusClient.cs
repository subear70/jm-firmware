using System;
using System.IO.Ports;

namespace DesktopModbusController.Modbus
{
    /// <summary>
    /// Modbus RTU master — builds raw byte frames, sends them over a SerialPort,
    /// reads and validates responses.  Thread-safe via an internal lock.
    /// </summary>
    public sealed class ModbusClient : IDisposable
    {
        private readonly SerialPort _port;
        private readonly object     _lock = new object();
        private const int           ReadTimeoutMs = 500;

        /// <summary>Creates a client but does not open the port.</summary>
        /// <param name="portName">COM port name, e.g. "COM3".</param>
        /// <param name="baudRate">Baud rate — must match firmware (115200).</param>
        public ModbusClient(string portName, int baudRate)
        {
            _port = new SerialPort(portName, baudRate, Parity.None, 8, StopBits.One)
            {
                ReadTimeout  = ReadTimeoutMs,
                WriteTimeout = ReadTimeoutMs,
            };
        }

        /// <summary>Opens the serial port.</summary>
        public void Connect()
        {
            lock (_lock)
            {
                if (!_port.IsOpen)
                    _port.Open();
            }
        }

        /// <summary>Closes the serial port.</summary>
        public void Disconnect()
        {
            lock (_lock)
            {
                if (_port.IsOpen)
                    _port.Close();
            }
        }

        /// <summary>Whether the port is currently open.</summary>
        public bool IsConnected
        {
            get { lock (_lock) { return _port.IsOpen; } }
        }

        // ── FC03 — Read Holding Registers ────────────────────────────────────

        /// <summary>Read <paramref name="count"/> holding registers starting at <paramref name="startAddress"/>.</summary>
        /// <returns>Array of <paramref name="count"/> register values.</returns>
        public ushort[] ReadHoldingRegisters(byte deviceAddress, ushort startAddress, ushort count)
        {
            lock (_lock)
            {
                byte[] req = BuildReadRequest(deviceAddress, 0x03, startAddress, count);
                SendFrame(req);

                int expectedLen = 3 + count * 2 + 2;  // addr + fc + byteCount + data + CRC
                byte[] rsp = ReadResponse(expectedLen);
                ValidateReadResponse(rsp, deviceAddress, 0x03, expectedLen);

                return ExtractRegisters(rsp, count);
            }
        }

        // ── FC04 — Read Input Registers ──────────────────────────────────────

        /// <summary>Read <paramref name="count"/> input registers starting at <paramref name="startAddress"/>.</summary>
        public ushort[] ReadInputRegisters(byte deviceAddress, ushort startAddress, ushort count)
        {
            lock (_lock)
            {
                byte[] req = BuildReadRequest(deviceAddress, 0x04, startAddress, count);
                SendFrame(req);

                int expectedLen = 3 + count * 2 + 2;
                byte[] rsp = ReadResponse(expectedLen);
                ValidateReadResponse(rsp, deviceAddress, 0x04, expectedLen);

                return ExtractRegisters(rsp, count);
            }
        }

        // ── FC06 — Write Single Register ─────────────────────────────────────

        /// <summary>Write a single holding register.</summary>
        public void WriteSingleRegister(byte deviceAddress, ushort address, ushort value)
        {
            lock (_lock)
            {
                byte[] req = new byte[8];
                req[0] = deviceAddress;
                req[1] = 0x06;
                req[2] = (byte)(address >> 8);
                req[3] = (byte)(address & 0xFF);
                req[4] = (byte)(value   >> 8);
                req[5] = (byte)(value   & 0xFF);
                AppendCrc(req, 6);
                SendFrame(req);

                byte[] rsp = ReadResponse(8);
                ValidateEchoResponse(rsp, req, deviceAddress, 0x06);
            }
        }

        // ── FC16 — Write Multiple Registers ──────────────────────────────────

        /// <summary>Write multiple consecutive holding registers.</summary>
        public void WriteMultipleRegisters(byte deviceAddress, ushort startAddress, ushort[] values)
        {
            if (values == null || values.Length == 0)
                throw new ArgumentException("values must not be empty.", nameof(values));

            lock (_lock)
            {
                int dataBytes = values.Length * 2;
                byte[] req = new byte[9 + dataBytes];
                req[0] = deviceAddress;
                req[1] = 0x10;
                req[2] = (byte)(startAddress   >> 8);
                req[3] = (byte)(startAddress   & 0xFF);
                req[4] = (byte)(values.Length  >> 8);
                req[5] = (byte)(values.Length  & 0xFF);
                req[6] = (byte)dataBytes;
                for (int i = 0; i < values.Length; i++)
                {
                    req[7 + i * 2]     = (byte)(values[i] >> 8);
                    req[7 + i * 2 + 1] = (byte)(values[i] & 0xFF);
                }
                AppendCrc(req, 7 + dataBytes);
                SendFrame(req);

                byte[] rsp = ReadResponse(8);
                ValidateEchoResponse(rsp, req, deviceAddress, 0x10);
            }
        }

        // ── Frame building helpers ────────────────────────────────────────────

        private static byte[] BuildReadRequest(byte addr, byte fc,
                                               ushort startReg, ushort count)
        {
            byte[] req = new byte[8];
            req[0] = addr;
            req[1] = fc;
            req[2] = (byte)(startReg >> 8);
            req[3] = (byte)(startReg & 0xFF);
            req[4] = (byte)(count    >> 8);
            req[5] = (byte)(count    & 0xFF);
            AppendCrc(req, 6);
            return req;
        }

        private static void AppendCrc(byte[] buf, int dataLen)
        {
            ushort crc = ModbusCrc.Compute(buf, 0, dataLen);
            buf[dataLen]     = (byte)(crc & 0xFF);
            buf[dataLen + 1] = (byte)(crc >> 8);
        }

        private void SendFrame(byte[] frame)
        {
            _port.DiscardInBuffer();
            _port.Write(frame, 0, frame.Length);
        }

        private byte[] ReadResponse(int expectedBytes)
        {
            byte[] buf = new byte[expectedBytes];
            int received = 0;
            DateTime deadline = DateTime.UtcNow.AddMilliseconds(ReadTimeoutMs);
            while (received < expectedBytes)
            {
                if (DateTime.UtcNow > deadline)
                    throw new ModbusTimeoutException(buf[0]);
                int n = _port.Read(buf, received, expectedBytes - received);
                received += n;
            }
            return buf;
        }

        private static void ValidateReadResponse(byte[] rsp, byte addr, byte fc, int expectedLen)
        {
            CheckErrorResponse(rsp, addr, fc);

            if (rsp.Length < expectedLen)
                throw new ModbusException($"Response too short: got {rsp.Length}, expected {expectedLen}.");

            ushort recvCrc = (ushort)(rsp[rsp.Length - 2] | (rsp[rsp.Length - 1] << 8));
            ushort calcCrc = ModbusCrc.Compute(rsp, 0, rsp.Length - 2);
            if (recvCrc != calcCrc) throw new ModbusCrcException();
        }

        private static void ValidateEchoResponse(byte[] rsp, byte[] req,
                                                 byte addr, byte fc)
        {
            CheckErrorResponse(rsp, addr, fc);

            ushort recvCrc = (ushort)(rsp[rsp.Length - 2] | (rsp[rsp.Length - 1] << 8));
            ushort calcCrc = ModbusCrc.Compute(rsp, 0, rsp.Length - 2);
            if (recvCrc != calcCrc) throw new ModbusCrcException();
        }

        private static void CheckErrorResponse(byte[] rsp, byte addr, byte fc)
        {
            if (rsp.Length >= 3 && rsp[1] == (fc | 0x80))
                throw new ModbusException(
                    $"Device 0x{addr:X2} returned exception code 0x{rsp[2]:X2}.",
                    rsp[2]);
        }

        private static ushort[] ExtractRegisters(byte[] rsp, ushort count)
        {
            var result = new ushort[count];
            for (int i = 0; i < count; i++)
                result[i] = (ushort)((rsp[3 + i * 2] << 8) | rsp[4 + i * 2]);
            return result;
        }

        public void Dispose()
        {
            _port?.Close();
            _port?.Dispose();
        }
    }
}
