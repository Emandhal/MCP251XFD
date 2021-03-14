# MCP251XFD
MCP251XFD is an fully hardware independant **Driver** primarily aimed at embedded world

# Presentation
This driver only takes care of configuration and check of the internal registers and the formatting of the communication with the device. That means it does not directly take care of the physical communication, there is functions interfaces to do that. By doing this, the driver can control a MCP2517FD trough a I2C to SPI converter without any change, the transformation will be done in the interface functions.
Each driver’s functions need a device structure that indicate with which device he must threat and communicate. Each device can have its own configuration.
The driver can detect which one of the MCP2517FD or the MCP2518FD is connected.

# Usage

## Installation

### Get the sources
Get and add the 3 following files to your project
```
MCP251XFD.c
MCP251XFD.h
Conf_MCP251XFD_Template.h
ErrorsDef.h
```
Copy or rename the file `Conf_MCP251XFD_Template.h` to `Conf_MCP251XFD.h`... Et voila!

## Others directories

### Tests\ directory
The **Tests** folder contains an example of use on the _SAMV71 Xplained Ultra board_
See the `Main_Synchronous.c` for the hardware setup.

### Tests\UnitTest\ directory
_The **Tests\UnitTest** folder contains unit test for Visual Studio 2017 and are not required._

# Configuration
To set up one or more devices in the project, you must:
* Configure the driver (`Conf_MCP251XFD.h`) which will be the same for all devices but modify only its behavior in the project
* Create and define the configuration of as many device structures as there are devices to use
* Create and define controller and CAN controller configuration for each device. Multiple devices can share the same configuration
* Initialize the device with the configuration structure previously defined

** All is explained in the MCP251XFD driver library guide**

# Driver usage

** All is explained in the MCP251XFD driver library guide**
