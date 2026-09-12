# Serial-Ethernet-FW

This project targets the [Serial-Ethernet-HW](https://github.com/TL-Embedded/Serial-Ethernet-HW) project.

The firmware is built around my [STM32X](https://github.com/Lambosaurus/STM32X) HAL.

# Function

This firmware is a simple remote serial port. It is intended to convert some TTL serial SCPI devices into network attached devices.

It provides the following key functionality: 
* DHCP support for automatic IP assignment
* MDNS implementation for device discovery
* A TCP socket to enable reading and writing to the Serial port

# MDNS

The device declares itself using MDNS. By default, it will appears as `ethernet-serial.local` on the network - avoiding the need for a device discovery process.

See [Config](#config) for setting the MDNS name.

# TCP/Serial

Open a TCP connection to port `5025`. Raw TCP messages can be used to read and write to the serial port. The serial port is configured at `9600 8N1`.

See [Config](#config) for setting the baud rate.

Data recieved via TCP will be emitted to the serial port as soon as recieved. 

To minimise the number of discrete TCP read/writes, data read on the serial port is chunked before sending TCP messages - using the following rules:
 1. Immedately send any buffered data on reception of a `\n` character.
 2. Send all data if `64` or more bytes are buffered.

> TODO: No timeout is used, but would be advisable for non SCPI applications.

# Config

The configuration is stored in the last flash page (0x0807800)

| Offset | Type   | Default           | Description |
| ------ | ------ | ----------------- | ----------- |
| 0      | u32    | 9600              | UART baud rate |
| 4      | u32    | 0                 | Reserved    |
| 8      | u8[64] | "serial-ethernet" | MDNS host name as a null terminated string |
| 72     | u32    | 0xAA0055AA        | Key. Must be this value or config will be defaulted |
