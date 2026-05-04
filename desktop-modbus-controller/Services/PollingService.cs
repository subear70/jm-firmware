using System;
using System.Threading;
using DesktopModbusController.Devices;
using DesktopModbusController.Modbus;

namespace DesktopModbusController.Services
{
    /// <summary>
    /// Background thread that periodically polls all online devices and
    /// invokes a callback for each update so the UI can refresh.
    /// </summary>
    public sealed class PollingService
    {
        private readonly DeviceManager        _manager;
        private readonly Action<WaveformDevice> _onDeviceUpdated;
        private readonly Action<string>         _onError;
        private readonly int                    _intervalMs;

        private Thread          _thread;
        private volatile bool   _running;

        /// <param name="manager">DeviceManager to use for status reads.</param>
        /// <param name="onDeviceUpdated">
        ///   Called on the polling thread after a successful device read.
        ///   The caller must marshal to the UI thread via Invoke().
        /// </param>
        /// <param name="onError">Called with a log message when a read fails.</param>
        /// <param name="intervalMs">Poll interval in milliseconds (default 1000).</param>
        public PollingService(DeviceManager          manager,
                              Action<WaveformDevice> onDeviceUpdated,
                              Action<string>         onError,
                              int                    intervalMs = 1000)
        {
            _manager         = manager         ?? throw new ArgumentNullException(nameof(manager));
            _onDeviceUpdated = onDeviceUpdated ?? throw new ArgumentNullException(nameof(onDeviceUpdated));
            _onError         = onError;
            _intervalMs      = intervalMs;
        }

        /// <summary>Start the polling thread.</summary>
        public void Start()
        {
            if (_running) return;
            _running = true;
            _thread  = new Thread(PollLoop)
            {
                IsBackground = true,
                Name         = "ModbusPollingThread"
            };
            _thread.Start();
        }

        /// <summary>Signal the polling thread to stop and wait for it to exit.</summary>
        public void Stop()
        {
            _running = false;
            _thread?.Join(2000);
            _thread = null;
        }

        private void PollLoop()
        {
            while (_running)
            {
                // Snapshot the device list to avoid holding a lock across I/O
                var devices = new System.Collections.Generic.List<WaveformDevice>(_manager.Devices);

                foreach (var device in devices)
                {
                    if (!_running) break;
                    if (device.Status == DeviceStatus.Unknown) continue;

                    try
                    {
                        _manager.ReadDeviceStatus(device);
                        _onDeviceUpdated?.Invoke(device);
                    }
                    catch (ModbusTimeoutException)
                    {
                        device.Status = DeviceStatus.Unknown;
                        _onError?.Invoke($"[POLL] Timeout — device {device.Address} lost.");
                        _onDeviceUpdated?.Invoke(device);
                    }
                    catch (ModbusException ex)
                    {
                        _onError?.Invoke($"[POLL] Device {device.Address}: {ex.Message}");
                    }
                    catch (Exception ex)
                    {
                        _onError?.Invoke($"[POLL] Unexpected error: {ex.Message}");
                    }
                }

                Thread.Sleep(_intervalMs);
            }
        }
    }
}
