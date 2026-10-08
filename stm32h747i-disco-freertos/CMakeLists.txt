cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake)

project(h747_disco_freertos C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TP ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(FREERTOS ${TP}/FreeRTOS-Kernel)
set(HAL      ${TP}/stm32h7xx_hal_driver)
set(DEVICE   ${TP}/cmsis_device_h7)
set(CMSIS    ${TP}/CMSIS_5/CMSIS/Core)

# ---- CPU flags: Cortex-M7 with double-precision FPU (CM7 core of the H747) ----
set(CPU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)

add_executable(${PROJECT_NAME}.elf
    # Application
    Core/Src/main.c
    Core/Src/stm32h7xx_it.c
    Core/Src/stm32h7xx_hal_timebase_tim.c
    Core/Src/syscalls.c

    # Startup + system
    ${DEVICE}/Source/Templates/gcc/startup_stm32h747xx.s
    ${DEVICE}/Source/Templates/system_stm32h7xx.c

    # FreeRTOS kernel
    ${FREERTOS}/tasks.c
    ${FREERTOS}/queue.c
    ${FREERTOS}/list.c
    ${FREERTOS}/timers.c
    ${FREERTOS}/event_groups.c
    ${FREERTOS}/stream_buffer.c
    ${FREERTOS}/portable/GCC/ARM_CM7/r0p1/port.c
    ${FREERTOS}/portable/MemMang/heap_4.c

    # STM32 HAL (only what is needed; add more as you use peripherals)
    ${HAL}/Src/stm32h7xx_hal.c
    ${HAL}/Src/stm32h7xx_hal_cortex.c
    ${HAL}/Src/stm32h7xx_hal_rcc.c
    ${HAL}/Src/stm32h7xx_hal_rcc_ex.c
    ${HAL}/Src/stm32h7xx_hal_pwr.c
    ${HAL}/Src/stm32h7xx_hal_pwr_ex.c
    ${HAL}/Src/stm32h7xx_hal_gpio.c
    ${HAL}/Src/stm32h7xx_hal_dma.c
    ${HAL}/Src/stm32h7xx_hal_dma_ex.c
    ${HAL}/Src/stm32h7xx_hal_flash.c
    ${HAL}/Src/stm32h7xx_hal_flash_ex.c
    ${HAL}/Src/stm32h7xx_hal_hsem.c
    ${HAL}/Src/stm32h7xx_hal_exti.c
    ${HAL}/Src/stm32h7xx_hal_tim.c
    ${HAL}/Src/stm32h7xx_hal_tim_ex.c
    ${HAL}/Src/stm32h7xx_hal_uart.c
    ${HAL}/Src/stm32h7xx_hal_uart_ex.c
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    ${FREERTOS}/include
    ${FREERTOS}/portable/GCC/ARM_CM7/r0p1
    ${HAL}/Inc
    ${DEVICE}/Include
    ${CMSIS}/Include
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    STM32H747xx
    CORE_CM7
    USE_HAL_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    -Wall -Wextra
    -ffunction-sections -fdata-sections
    -fno-common
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-O2 -g>
)

# Vendor sources are noisy at -Wextra; keep warnings for our own code only
set_source_files_properties(
    ${HAL}/Src/stm32h7xx_hal.c
    PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter"
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32H747XIHx_FLASH_CM7.ld
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
    -u _printf_float
)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE c m nosys)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
)

# ---- Convenience targets (flash with OpenOCD) ----
add_custom_target(flash
    COMMAND openocd -f ${CMAKE_CURRENT_SOURCE_DIR}/openocd/stm32h747i-disco-cm7.cfg
            -c "program $<TARGET_FILE:${PROJECT_NAME}.elf> verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing CM7 image via ST-LINK/OpenOCD"
)
