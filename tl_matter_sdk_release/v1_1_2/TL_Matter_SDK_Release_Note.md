## V1.1.2(PR)

### Version

-   SDK Version: tl_matter_sdk V1.1.2
-   Chip Version
    -   TLSR952x(A4), TL321x(A2)
-   Hardware EVK Version
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
-   Toolchain Version
    -   TLSR952x/TL321x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   Update BLE libs to release/V4.0.4.4;
-   Change the init clock-frequency of TL321x into 48MHz;
-   Optimize the stack size for the TL3218x;
-   Add VID and PID judgment when OTA validation;
-   Disable device rotation ID by default;

### Bug Fixes

-   Fix the macros in the watchdog intermediate driver layer;
-   Fix the factory data loss after retention wakeup;
-   Fix the button in light-switch-app cannot enter low power mode;
-   Fix the PM cannot wake up normally due to incorrect use of macro;
-   Fix the deep sleep stack corruption;
-   Fix the incorrect ISR number and GPIO interrupts;
-   Fix the PLIC exception with deep-sleep and reset PLIC during the recovery state after wakeup;
-   Fix the CLINT external interrupt is set during restoring peripheral from deep-sleep;
-   Fix the frame counter issue for ieee802154;

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x
    -   lighting-app(non-pm)
        -   Flash
            -   849.2KB
        -   RAM
            -   222.2KB
    -   light-switch-app(pm-retention)
        -   Flash
            -   807.7KB
        -   RAM
            -   176.5KB

-   TL321x
    -   lighting-app(non-pm)
        -   Flash
            -   806.1KB
        -   RAM
            -   109.4KB
    -   light-switch-app(pm-retention)
        -   Flash
            -   763.9KB
        -   RAM
            -   108.6KB

### 版本

-   SDK 版本: tl_matter_sdk V1.1.2
-   芯片版本
    -   TLSR952x(A4), TL321x(A2)
-   硬件评估板版本
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
-   工具链版本
    -   TLSR952x/TL321x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   更新 BLE libs 至 V4.0.4.4版本；
-   将 TL321x 的初始时钟频率更改为 48MHz；
-   优化 TL3218x 的任务栈大小；
-   在 OTA 验证时添加 VID 和 PID 判断；
-   默认禁用设备 rotation ID；

### Bug Fixes

-   修复 watchdog 中间驱动层的宏；
-   修复 PM 唤醒后 factory data 数据丢失问题；
-   修复 light-switch-app 在按下 button 后无法进入低功耗模式的问题；
-   修复相关宏使用错误导致 PM 无法正常唤醒的问题；
-   修复 deep sleep 下堆栈被异常覆盖的问题；
-   修复 ISR 编号错误及 GPIO 中断异常的问题；
-   修复 deep sleep 下 PLIC 异常，在唤醒后恢复状态期间里重置 PLIC；
-   修复 deep sleep 唤醒后恢复外设时 CLINT 外部中断被异常设置的问题；
-   修复 ieee802154 帧计数器的问题；

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x
    -   lighting-app(non-pm)
        -   Flash
            -   849.2KB
        -   RAM
            -   222.2KB
    -   light-switch-app(pm-retention)
        -   Flash
            -   807.7KB
        -   RAM
            -   176.5KB

-   TL321x
    -   lighting-app(non-pm)
        -   Flash
            -   806.1KB
        -   RAM
            -   109.4KB
    -   light-switch-app(pm-retention)
        -   Flash
            -   763.9KB
        -   RAM
            -   108.6KB

## V1.1.1(PR)

### Version

-   SDK Version: tl_matter_sdk V1.1.1
-   Chip Version
    -   TLSR952x(A4), TL321x(A2), TL721x(A2)
-   Hardware EVK Version
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
    -   TL721x: C1T315A20_V1.2
-   Toolchain Version
    -   TLSR952x/TL321x/TL721x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   Support the dual-mode switch of zigbee and matter;
-   Improve watchdog driver, works without application layer init;

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 891KB
    -   RAM:
        -   IRAM: 82KB, DRAM: 23KB

-   TL721x

    -   Flash：
        -   MCUBoot + Matter: 886KB
    -   RAM:
        -   IRAM: 110KB, DRAM: 35KB

### 版本

-   SDK 版本: tl_matter_sdk V1.1.1
-   芯片版本
    -   TLSR952x(A4), TL321x(A2), TL721x(A2)
-   硬件评估板版本
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
    -   TL721x: C1T315A20_V1.2
-   工具链版本
    -   TLSR952x/TL321x/TL721x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   支持 zigbee 和 matter 的双模切换；
-   优化看门狗驱动，无需应用层初始化即可工作；

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 891KB
    -   RAM:
        -   IRAM: 82KB, DRAM: 23KB

-   TL721x

    -   Flash：
        -   MCUBoot + Matter: 886KB
    -   RAM:
        -   IRAM: 110KB, DRAM: 35KB

## V1.1.0(PR)

### Version

-   SDK Version: tl_matter_sdk V1.1.0
-   Chip Version
    -   TLSR952x(A4), TL321x(A2), TL721x(A2)
-   Hardware EVK Version
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
    -   TL721x: C1T315A20_V1.2
-   Toolchain Version
    -   TLSR952x/TL321x/TL721x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   Optimize deepret mode to save power consumption;
-   Optimize RAM consumption;
-   Optimize ieee802154 driver;
-   Improve SDK stability;

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 891KB
    -   RAM:
        -   IRAM: 82KB, DRAM: 23KB

-   TL721x

    -   Flash：
        -   MCUBoot + Matter: 886KB
    -   RAM:
        -   IRAM: 110KB, DRAM: 35KB

### 版本

-   SDK 版本: tl_matter_sdk V1.1.0
-   芯片版本
    -   TLSR952x(A4), TL321x(A2), TL721x(A2)
-   硬件评估板版本
    -   TLSR952x: C1T266A20_V1.3
    -   TL321x: C1T335A20_V1.3
    -   TL721x: C1T315A20_V1.2
-   工具链版本
    -   TLSR952x/TL321x/TL721x: riscv64-elf-gcc ( IDE:
        zephyr-sdk-0.17.0_linux-x86_64_minimal )

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Features

-   优化 deepret 模式，节约 deepret 模式下功耗；
-   优化 ram 消耗；
-   优化 ieee802154 驱动；
-   提升 sdk 稳定性；

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 891KB
    -   RAM:
        -   IRAM: 82KB, DRAM: 23KB

-   TL721x

    -   Flash：
        -   MCUBoot + Matter: 886KB
    -   RAM:
        -   IRAM: 110KB, DRAM: 35KB
