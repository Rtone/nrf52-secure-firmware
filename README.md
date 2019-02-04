# nRF52 Secure Firmware
---------


**nRF52-secure-firmware** is an entry point for Nordic development nRF52 BLE platforms and a part of a larger project for best practice recommended for BLE security.         
It focuses on maintaining different security parts of the nrf52 BLE firmware to foster good practice and awareness of security weaknesses and point to components that needfurther hardening.

All developments have been conducted on an nRF52 Nordic development kit platform: 
*	The currently supported Nordic board is: **nRF52840**
*	The currently supported hardware chip: **PCA10056** ARM Cortex-M4F CPU
*	The currently supported SDK version is: **14.2.0**
*	The currently supported Softdevice version is: **s140_5.0.0**

The project may need modifications to work with other versions or other boards.


## Supported Features

This current version of the firmware supports the following features :

- [x]	Support both *LE Secure Connections* and LE *Legacy paring* modes.
- [x]	Support *micro-ecc* and *cc310* cryptographic library backends. 
- [x]	Implementing *Numeric Comparaison* and *Passkey* association model.
- [x]	Changing MAC Address vendor with specific or random bytes generator. 
- [x]	Protecting read & write characteristics. 
- [x]	AES-128-ECB&CBC encryption and decryption of BLE data communication. 
- [x]	Real-time counter to measure elapsed time between tasks.


## TODO Feature List

- [ ]	implement hash functions. 
- [ ]	Securing secure *Device Firware Update* (DFU) mode. 
- [ ]	encryption layer for DFU mode.


## How to Import Project into Eclipse 

Installing Plug-in :

1. **GNU toolchain for ARM:** (gcc-arm-none-eabi-7-2017-q4-major) : run the script "arm-gcc-install.sh" in "tools/arm-gcc/"

2. **JLink** (for debugging in eclipse) : LINK BELOW ``https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack``

3. **nrfjprog :** LINK BELOW ``http://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52832`` in Download tab select ``nRF5x-Command-Line-Tools-XXX`` depends on your OS.

### Step 1
Help > Eclipse Marketplace. 
Search ``ARM`` and select ``GNU MCU Eclipse`` from the results. The required packages to install are:

GNU MCU C/C++ ARM Cross Compiler

GNU MCU C/C++ J-Link Debugging

GNU MCU C/C++ Packs

### Step 2

First we run the script ``nordic_setup_sdk.sh`` for downloading the SDK. The script is located in the main repository: (by default the SDK will be located in ``./libs/``)


### Step 3

In eclipse :

File > New Project > Makefile Project with Existing code 

Fill the Project Name with the name of your choose. 

![Alt text](./docs/img/import.png?raw=true "import")


For the Exiting Code Location browse into  ``./nrf52-secure-firmware/``.

For Toolchain choose ARM cross GCC

Now, we should see in Project Explorer the new project containing the Makefile, the linker file, the main file located in "source/", libs folder.

### Step 4
Configuration of eclipse:
 
Go to menu window > preferences > MCU > Global ARM toolchain Path > set the arm tool chain folder.
![Alt text](./docs/img/arm.png?raw=true "toolchain")

Go to menu window > preferences > MCU > Global SEGGER J-Link Path > set Executable to JLinkGDBServer ; set Folder "/usr/local/bin"
![Alt text](./docs/img/segger.png?raw=true "j-link")


In project properties window : Configure default C/C++ build command to ``make VERBOSE=1``.
![Alt text](./docs/img/build.png?raw=true "build")


### Step 5
One step left to build project successfully :

To use the SDK we first need to set the toolchain path in ``makefile.windows`` or ``makefile.posix`` depending on platform you are using. That is, the .posix should be edited if your are working on either Linux or OS X. These files are located in ``./libs/nRF5_SDK/nRF5_SDK_14.2.0_17b948a/components/toolchain/gcc/``

Change ``makefile.posix`` into :
	
	GNU_INSTALL_ROOT :=  «PATH to gcc-arm-none-eabi-4_9-2015q3 bins»
	GNU_VERSION := «Version of gcc-arm-eabi»
	GNU_PREFIX := arm-none-eabi
	
For example in my case:

	GNU_INSTALL_ROOT := /Users/ridaidil/Desktop/Stage-RTONE/nrf_dev/gcc-arm-none-eabi-7-2017-q4-major/bin/
	GNU_VERSION := 7.2.1
	GNU_PREFIX := arm-none-eabi	
	
After that we run the script ``build_all.sh`` located in ``./libs/nRF5_SDK/nRF5_SDK_14.2.0_17b948a/external/micro-ecc/`` to obtain the library ``Micro_ecc_lib_nrf52.a`` which used by the secure bootloader.

PS: in order to run the script ``build_all.sh`` without problems, run this command line 

```
$ sed -i 's/\r//g' build_all.sh 
```

The Project now is ready to be built.

## ``make flash`` from Eclipse

Right click in project menu > Build target > Create.

Fill ``Target name`` with ``flash``

![Alt text](./docs/img/flash1.png?raw=true "flash")

Make sure to add the PATH of ``nrfjprog`` program into ``PATH system variables``

In project properties window > build 

![Alt text](./docs/img/flash2.png?raw=true "flash")



## Debug from Eclipse 

Right click in project menu > Debug As > Debug Configuration.

Make sure to choose ``GDB SEGGER J-Link Debugging`` debugger type.

Fill in the blanks with same values in the picture below.  

![Alt text](./docs/img/debug.png?raw=true "debug")


## Contributions or Need Help 

You are welcome to contribute and suggest any improvements.
If you want to point to an issue, Please [file an issue](https://github.com/rtoneIotSecurity/nrf52-secure-firmware/issues).


If you have questions or need further guidance on using the tool, 
Please [file an issue](https://github.com/rtoneIotSecurity/nrf52-secure-firmware/issues).

 
## Direct contributions

Fork the repository, file a pull request and You are good to go ;)


## License

This project is licensed under The MIT License terms.

Copyright (c) 2019 Rtone IoT Security





