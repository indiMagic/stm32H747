# STM32H747I-DISCO FreeRTOS starter (Cortex-M7)

Bare-metal CMake + FreeRTOS + STM32 HAL project for the **STM32H747I-DISCO** board, targeting the
**Cortex-M7** core. Builds with `arm-none-eabi-gcc`, flashes/debugs through the on-board ST-LINK
with OpenOCD, and is set up for VS Code (CMake Tools + Cortex-Debug).

## What it does
- 400 MHz core clock from the 25 MHz HSE (PLL1), SMPS direct supply, I/D cache on
- FreeRTOS kernel on SysTick, HAL tick on TIM6 (so the two never fight over SysTick)
- Three demo tasks: LED blinker (LD1 green), producer -> queue -> consumer (toggles LD2 orange)
- `printf()` over the ST-LINK virtual COM port (USART1, 115200 8N1)

## Toolchain (Ubuntu / Debian)
```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi gdb-multiarch openocd cmake ninja-build git
```
macOS: `brew install --cask gcc-arm-embedded && brew install openocd cmake ninja`.
Windows: install the Arm GNU Toolchain, OpenOCD (xPack) and CMake/Ninja, and put them on `PATH`.

VS Code extensions (also listed in `.vscode/extensions.json`): C/C++, CMake Tools, **Cortex-Debug**.

## Get the code
```bash
git clone <your-repo-url>
cd stm32H747/stm32h747i-disco-freertos
```
The third-party sources the build needs are committed under `third_party/` (trimmed copies of
FreeRTOS-Kernel, the STM32H7 HAL, the CMSIS device headers and CMSIS_5 core headers), so there are
no submodules to fetch.

## Build
```bash
cmake --preset debug
cmake --build --preset debug      # -> build/Debug/h747_disco_freertos.elf/.bin/.hex
```
In VS Code: `Ctrl+Shift+B` runs the default **Build (Debug)** task.

## Flash and debug
- Connect the board's **CN14 (ST-LINK USB)** to your PC.
- Flash: `cmake --build --preset debug --target flash`
- Debug: press **F5** with the "Debug CM7 (ST-LINK / OpenOCD)" configuration.
- Serial output: open the ST-LINK virtual COM port at 115200 baud.

## Layout
```
Core/Inc, Core/Src     application, FreeRTOSConfig.h, HAL config, IRQ handlers, retarget
STM32H747XIHx_FLASH_CM7.ld   linker script (CM7 in flash bank 1, data in AXI SRAM, stack in DTCM)
cmake/                 arm-none-eabi toolchain file
openocd/               OpenOCD config for the on-board ST-LINK (CM7 only)
.vscode/               build tasks, Cortex-Debug launch configs, settings
third_party/           FreeRTOS-Kernel, STM32H7 HAL, CMSIS device, CMSIS_5 (trimmed copies)
```

## Notes for this dual-core part
- This image runs on the **CM7** only. The CM4 is not started by this project; its flash bank
  (0x08100000) is reserved for a future CM4 image.
- If the CM4 boot option bytes are enabled on your board, make sure the CM4 does not fight over
  shared peripherals; use HSEM for cross-core sync when you add a CM4 project.
- DTCM (0x20000000) is **not DMA-accessible**. Place DMA buffers in `.dma_buffer` (SRAM1-3, 0x30000000),
  and handle D-cache coherency (clean/invalidate or use an MPU non-cacheable region).
- The clock setup assumes the board's SMPS direct-supply configuration (as in ST's examples for this board).
