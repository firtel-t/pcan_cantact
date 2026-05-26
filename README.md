# !ACHTUNG! READ FIRST [COPYRIGHT ISSUE](https://github.com/moonglow/pcan_cantact/issues/18)

## XCAN firmware for STM32F042 / STM32G431 based boards

![build workflow](https://github.com/moonglow/pcan_cantact/actions/workflows/firmware_build.yml/badge.svg)

## Target Hardware

### STM32F042 (bxCAN)

* [CANtact](https://github.com/linklayer/cantact-hw) - opensource USB-CAN adapter project `make cantact_16`
* [CANable](https://canable.io/) - opensource USB-CAN adapter based on CANtact project `make canable`
* [Entreé](https://github.com/tuna-f1sh/entree) - opensource USB-C CAN adapter based on CANable project `make entree`
* [Ollie](https://github.com/slimelec/ollie-hw) - opensource USB-CAN adapter with isolated USB `make ollie`
* Any other STM32F042 based boards with external or internal OSC.

### STM32G431 (FDCAN, Classic CAN only - CAN FD not supported)

* [MKS CANable V2.0](https://github.com/makerbase-mks/CANable-MKS) - USB-CAN adapter with STM32G431 `make mks_canable2`
* Any other STM32G431 based boards with FDCAN + USB.

## Toolchain

* GNU Arm Embedded Toolchain

## Build

```bash
# Build all F042 targets
make

# Build specific F042 target
make canable

# Build G431 target (MKS CANable V2.0)
make mks_canable2
```

## Flash

Simply upload the `.bin` file via WebUSB DFU — no special tools required.

* https://devanlai.github.io/webdfu/dfu-util/

Hold the BOOT button while connecting USB to enter DFU mode.

License
----

WTFPL
