## V1.1.0(PR)

### Version

-   SDK Version: telink_zigbee_matter_dual_mode_sdk V1.1.0
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

-   Support TLSR952x(4MB) and TL321x(2MB);
-   Support dual-mode switch for Zigbee and Matter;
-   Support customized mode;
-   Support LZMA firmware compression;
-   Support kDnsstimer to protect from init to dnss;
-   Support group control of up to 8 per fabric;
-   Support deferred storage attributes for provider;
-   Support ota events status storage;

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### Flash

-   TLSR952x
    -   GD25LQ32E
-   TL321x
    -   P25Q16SU

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 808KB
    -   RAM:
        -   IRAM: 83KB, DRAM: 25KB

### 版本

-   SDK 版本: telink_zigbee_matter_dual_mode_sdk V1.1.0
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

-   支持 TLSR952x（4MB）和 TL321x（2MB）；
-   支持 Zigbee 和 Matter 双模切换；
-   支持客制化模式；
-   支持 LZMA 固件压缩；
-   支持 kDnstimer 从 init 到 dnss 的保护；
-   支持每个 fabric 高达 8 个组的组控；
-   支持延迟属性存储；
-   支持 ota 事件状态存储；

### Bug Fixes

-   N/A

### BREAKING CHANGES

-   N/A

### Note

-   N/A

### Flash

-   TLSR952x
    -   GD25LQ32E
-   TL321x
    -   P25Q16SU

### CodeSize

-   TLSR952x

    -   Flash
        -   MCUBoot + Matter: 850KB
    -   RAM
        -   IRAM: 101KB, DRAM: 110KB

-   TL321x

    -   Flash：
        -   MCUBoot + Matter: 808KB
    -   RAM:
        -   IRAM: 83KB, DRAM: 25KB
