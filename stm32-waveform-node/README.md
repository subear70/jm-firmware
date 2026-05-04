# stm32-waveform-node — Firmware

STM32F446RE firmware for a Modbus RTU slave that drives a voltage-controlled
frequency jammer via DAC output. Settings and calibration data are persisted in
the MCU's internal Flash (emulated EEPROM, Sector 7).

---

## Hardware Required

| Item | Details |
|---|---|
| Board | Nucleo-64 STM32F446RE |
| RS485 transceiver | e.g. MAX485 / SP3485 — **production mode only** |
| USB cable | ST-Link (on-board, micro-USB) |
| PC | Windows / Linux / macOS |

### Pin Connections

| Signal | MCU Pin | Notes |
|---|---|---|
| USART2 TX | PA2 | RS485 data out |
| USART2 RX | PA3 | RS485 data in |
| RS485 DE/RE | PA1 | Direction control (HIGH = transmit) |
| DAC1 CH1 | PA4 | Analog voltage output to VCO |
| TIM2 | internal | Waveform step timer |
| TIM6 | internal | Modbus inter-frame gap timer |
| LED LD2 | PA5 | Onboard LED (startup blink) |

> **Solder bridges SB13/SB14:** On Nucleo-F446RE, PA2/PA3 are routed to the
> ST-Link VCP by default via SB13/SB14.
> - **Debug mode** (`DEBUG_VCP_MODE 1`) — leave bridges **intact**, no RS485
>   transceiver needed. The PC sees the board as a virtual COM port over USB.
> - **Production mode** (`DEBUG_VCP_MODE 0`) — **cut** SB13/SB14, connect the
>   RS485 transceiver to PA2/PA3/PA1.

---

## Prerequisites

1. Download and install **STM32CubeIDE** from
   https://www.st.com/en/development-tools/stm32cubeide.html  
   (free, includes GCC toolchain and ST-Link drivers).

---

## Step 1 — Create the CubeMX Project

STM32CubeIDE embeds CubeMX. Use it to generate the HAL skeleton that the
hand-written source files plug into.

1. Open STM32CubeIDE → **File → New → STM32 Project**.
2. In the board selector search for **NUCLEO-F446RE** and select it. Click **Next**.
3. Name the project **stm32-waveform-node**. Language: **C**.
4. Uncheck **"Use default location"** and browse to this directory
   (`stm32-waveform-node/`) so the `.ioc` file lands alongside the existing
   source folders. Click **Finish**.
5. When asked to initialise peripherals to default mode, click **No**
   (you will configure each peripheral manually in the steps below).

The `.ioc` configuration file opens in CubeMX view.

---

## Step 2 — Configure Peripherals in CubeMX

### 2.1 Clock Tree

1. Click the **Clock Configuration** tab.
2. Set **Input frequency** to **8 MHz** (Nucleo HSE crystal).
3. Select **HSE** as PLL input source.
4. Set PLL multipliers: **PLLM = 8, PLLN = 360, PLLP = 2** → SYSCLK = 180 MHz.
5. Select **PLLCLK** as the system clock source.
6. Set **APB1 Prescaler = /4** → APB1 = 45 MHz, TIM2/TIM6 clock = 90 MHz.
7. Set **APB2 Prescaler = /2** → APB2 = 90 MHz.
8. Click **Resolve Clock Issues** if any conflicts appear.

### 2.2 USART2 — Modbus RTU

1. **Pinout & Configuration → Connectivity → USART2**:
   - Mode: **Asynchronous**
   - Baud Rate: **115200**
   - Word Length: **8 Bits**, Parity: **None**, Stop Bits: **1**
2. **NVIC Settings**: enable **USART2 global interrupt**.

Pins auto-assigned: **PA2** (TX), **PA3** (RX).

### 2.3 TIM2 — Waveform Step Timer

1. **Timers → TIM2**:
   - Clock Source: **Internal Clock**
   - Prescaler: **8999**
   - Counter Period: **99**
   - auto-reload preload: **Enable**
2. **NVIC Settings**: enable **TIM2 global interrupt**.

### 2.4 TIM6 — Modbus Inter-Frame Gap Timer

1. **Timers → TIM6**:
   - Clock Source: **Internal Clock**
   - Prescaler: **89**
   - Counter Period: **499**
   - auto-reload preload: **Enable**
2. **NVIC Settings**: enable **TIM6 and DAC global interrupt**.

### 2.5 DAC1 — Analog Output

1. **Analog → DAC1**:
   - OUT1 Configuration: **Connected to external pin only**
   - Output Buffer: **Enable**
   - Trigger: **None** (software trigger, driven from TIM2 ISR)

Pin auto-assigned: **PA4** (DAC_OUT1).

### 2.6 GPIO — RS485 Direction Control

1. Click pin **PA1** on the chip diagram → set to **GPIO_Output**.
   - User Label: `RS485_DE_RE`
   - Default output level: **Low**

Pin **PA5** (LED LD2) is already configured as GPIO_Output by the board
default when using the NUCLEO-F446RE target.

---

## Step 3 — Generate HAL Code

1. Click **Project → Generate Code** (or press `Alt+K`).
2. STM32CubeIDE regenerates `Src/main.c`, `stm32f4xx_it.c`, and the
   `Drivers/` folder. **Do not edit any auto-generated file** — all custom
   code goes inside `USER CODE BEGIN / END` guards.

---

## Step 4 — Add the Include Path

All headers live in `Inc/` and all `.c` files live in `Src/`. CubeIDE already
registers `Src/` as a source folder, so only the include path needs adding:

1. Right-click project → **Properties → C/C++ Build → Settings →
   MCU GCC Compiler → Include paths**.
2. Add:
   - `${workspace_loc:/${ProjName}/Inc}`

---

## Step 5 — Integrate into main.c

Open the CubeMX-generated `Src/main.c` and insert code from
`Src/main_integration.c` into the matching `USER CODE BEGIN / END` blocks:

| Section in main.c | What to add |
|---|---|
| `USER CODE BEGIN Includes` | `#include` directives for all modules |
| `USER CODE BEGIN PV` | `static uint8_t s_uart_rx_byte;` |
| `USER CODE BEGIN 2` | `Modbus_Init(...)`, `Waveform_Init(...)`, `EEPROM_Init()`, EEPROM load, boot LED blink, arm UART RX |
| `USER CODE BEGIN WHILE` | `Modbus_Process();` |

Then add the three HAL callback overrides **outside** `main()`, anywhere in
`Src/main.c` (or in `stm32f4xx_it.c`) inside `USER CODE` guards:

- `HAL_UART_RxCpltCallback`
- `HAL_UART_TxCpltCallback`
- `HAL_TIM_PeriodElapsedCallback`

All three are provided verbatim in `Src/main_integration.c`.

---

## Step 6 — Linker Script (Flash Sector 7 already protected)

`STM32F446RETX_FLASH.ld` is already configured with `LENGTH = 384K`,
reserving Flash Sector 7 (0x08060000 – 0x0807FFFF, 128 KB) exclusively for
emulated EEPROM. Do not change this value back to 512K.

---

## Step 7 — Select Communication Mode

Open `Inc/config.h` and set `DEBUG_VCP_MODE`:

```c
#define DEBUG_VCP_MODE    1    // 1 = ST-Link VCP (debug), 0 = RS485 (production)
```

| | `DEBUG_VCP_MODE 1` — Debug | `DEBUG_VCP_MODE 0` — Production |
|---|---|---|
| Physical path | USART2 → ST-Link chip → USB VCP | USART2 → RS485 transceiver → bus |
| Solder bridges SB13/SB14 | **Intact** (factory default) | **Cut** |
| RS485 transceiver | Not needed | Required (PA2 TX, PA3 RX, PA1 DE/RE) |
| PC connection | Virtual COM port over USB | USB-to-RS485 adapter |
| Multi-node bus | No (point-to-point only) | Yes (up to 32 nodes) |
| DE/RE toggling | Skipped automatically | Active |

The Modbus framing, CRC, and register logic are **identical** in both modes —
only the physical layer changes.

---

## Step 8 — Set the Device Modbus Address

Open `Inc/config.h` and change:

```c
#define MODBUS_DEVICE_ADDRESS    1U   // Set a unique address (1–247) per node
```

Each node on the same RS485 bus must have a different address.

---

## Step 9 — Build the Project

1. In STM32CubeIDE: **Project → Build All** (`Ctrl+B`).
2. The **Console** panel shows GCC output. A successful build ends with:
   ```
   Finished building: stm32-waveform-node.elf
   ```
3. Check **Memory Usage** — target has 384 KB usable Flash / 128 KB RAM.

### Common build errors

| Error | Fix |
|---|---|
| `undefined reference to HAL_DAC_...` | Ensure DAC is enabled in CubeMX and code regenerated |
| `undefined reference to Modbus_...` | Source folders not added to build (Step 4) |
| `multiple definition of HAL_TIM_PeriodElapsedCallback` | Remove duplicate weak definition in `stm32f4xx_it.c` |

---

## Step 10 — Flash the Firmware

### Via ST-Link (on-board debugger)

1. Connect the Nucleo board to the PC via the micro-USB cable (CN1 — ST-Link port).
2. In STM32CubeIDE: **Run → Run** (`Ctrl+F11`) to flash and run, or
   **Run → Debug** (`F11`) to flash and halt at `main()` for debugging.
3. The onboard LED (PA5 / LD2) blinks **3 times** on successful startup.

### Via STM32CubeProgrammer (standalone)

1. Open STM32CubeProgrammer → connect via **ST-LINK** interface.
2. **Open File** → select `Debug/stm32-waveform-node.elf` (or `.hex`).
3. Click **Download**.

---

## Step 11 — Debug Tips

- Use **Live Expressions** in CubeIDE to watch `volatile` variables such as
  `s_current_freq_hz` and `s_current_volt_mv` in real time without halting.
- If `HAL_TIM_PeriodElapsedCallback` is defined in both `Src/main.c` and
  `Src/stm32f4xx_it.c`, delete the one in `stm32f4xx_it.c`.
- To verify Modbus frames, connect a logic analyser to PA2/PA3 and decode as
  UART at 115200 baud.

---

## Step 12 — Verify with the Desktop Application

1. Connect the RS485 bus between the Nucleo board and your USB-to-RS485 adapter.
2. Open the **desktop-modbus-controller** WinForms application.
3. Select the correct COM port and baud rate (115200), click **Connect**.
4. Click **Scan Bus** — the device should appear at the address set in `config.h`.
5. Select the device and click **Configure** to set sweep frequencies and upload
   calibration data.

---

## Project File Structure Reference

```
stm32-waveform-node/
├── stm32-waveform-node.ioc    ← CubeMX project (created in Step 1)
├── Inc/
│   ├── config.h               ← compile-time constants + mode switch
│   ├── modbus_rtu.h           ← Modbus RTU slave API
│   ├── modbus_registers.h     ← register address constants
│   ├── waveform.h             ← VCO sweep API
│   └── eeprom.h               ← Flash-emulated EEPROM API
├── Src/
│   ├── main.c                 ← CubeMX generated + USER CODE blocks
│   ├── main_integration.c     ← reference: code to paste into main.c
│   ├── modbus_rtu.c           ← RTU framing, CRC, FC dispatcher
│   ├── modbus_registers.c     ← register read/write handlers
│   ├── waveform.c             ← DAC output, calibration interpolation
│   ├── eeprom.c               ← Flash-emulated EEPROM (Sector 7)
│   ├── syscalls.c             ← CubeMX generated
│   └── sysmem.c               ← CubeMX generated
├── Drivers/                   ← STM32 HAL — auto-generated, do not edit
├── Startup/                   ← startup_stm32f446retx.s — do not edit
└── STM32F446RETX_FLASH.ld     ← linker script (LENGTH = 384K, Sector 7 reserved)
```
