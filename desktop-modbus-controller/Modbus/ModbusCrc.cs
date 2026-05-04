using System;

namespace DesktopModbusController.Modbus
{
    /// <summary>CRC16 calculator for Modbus RTU (polynomial 0xA001, reflected 0x8005).</summary>
    internal static class ModbusCrc
    {
        private static readonly ushort[] Table = BuildTable();

        private static ushort[] BuildTable()
        {
            ushort[] t = new ushort[256];
            for (int i = 0; i < 256; i++)
            {
                ushort crc = (ushort)i;
                for (int b = 0; b < 8; b++)
                    crc = (crc & 1) != 0 ? (ushort)((crc >> 1) ^ 0xA001) : (ushort)(crc >> 1);
                t[i] = crc;
            }
            return t;
        }

        /// <summary>Compute Modbus CRC16 over <paramref name="length"/> bytes of <paramref name="data"/>.</summary>
        public static ushort Compute(byte[] data, int offset, int length)
        {
            ushort crc = 0xFFFF;
            for (int i = 0; i < length; i++)
                crc = (ushort)((crc >> 8) ^ Table[(crc ^ data[offset + i]) & 0xFF]);
            return crc;
        }

        /// <summary>Compute CRC over the entire array.</summary>
        public static ushort Compute(byte[] data)
            => Compute(data, 0, data.Length);
    }
}
