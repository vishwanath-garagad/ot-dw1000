[![OpenThread][ot-logo]][dw-repo]
# What is OpenThread?  

OpenThread is...
<a href="http://threadgroup.org/technology/ourtechnology#certifiedproducts">
<img src="https://cdn.rawgit.com/openthread/openthread/ab4c4e1e/doc/images/certified.svg" alt="Thread Certified Component" width="150px" align="right">
</a>

**...an open-source implementation of the [Thread](http://threadgroup.org/technology/ourtechnology) networking protocol.** Nest has released OpenThread to make the technology used in Nest products more broadly available to developers to accelerate the development of products for the connected home.

**...OS and platform agnostic**, with a narrow platform abstraction layer and a small memory footprint, making it highly portable.

**...a Thread Certified Component**, implementing all features defined in the [Thread 1.1.1 specification](http://threadgroup.org/technology/ourtechnology#specifications). This specification defines an IPv6-based reliable, secure and low-power wireless device-to-device communication protocol for home applications.

More information about Thread can be found on [threadgroup.org](http://threadgroup.org/).

[thread]: http://threadgroup.org/technology/ourtechnology
[dw-repo]: https://github.com/Decawave/ot-dw1000
[ot-logo]: DW1000/doc/images/openthread_logo.png
[nordic-img]: DW1000/doc/images/nordic.png
[evb1000-img]: DW1000/doc/images/evb1000.png
[evb-nordic-img]: DW1000/doc/images/evb-nordic.png


# OpenThread Features

OpenThread implements all features defined in the [Thread 1.1.1 specification](http://threadgroup.org/technology/ourtechnology#specifications), including all Thread networking layers (IPv6, 6LoWPAN, IEEE 802.15.4 with MAC security, Mesh Link Establishment, Mesh Routing) and device roles.

OpenThread supports both system-on-chip (SoC) and network co-processor (NCP) designs. Other features and enhancements include:

* Application support and services
    * IPv6 configuration and raw data interface
    * UDP sockets
    * CoAP client and server
    * DHCPv6 client and server
    * DNSv6 client
    * Command Line Interface (CLI)
* NCP support
    * Spinel - general purpose NCP protocol
    * `wpantund` - user-space NCP network interface driver/daemon
    * Sniffer support via NCP Spinel nodes

## Dependent tools to used for Nordic board
### SEGGER-JLink:

1.Download the suitable package from [SEGGER-JLink][SEGGER].

  [SEGGER]: https://www.segger.com/downloads/jlink
  
(or)

For Ubuntu 64-bit sytem download [SEGGER-JLink-Ubuntu-64bit][SEGGER-64bit].

  [SEGGER-64bit]: https://www.segger.com/downloads/jlink/JLink_Linux_V614h_x86_64.deb

2.Install the package by double clicking it

(or)

Install the package through command line
```bash
$ sudo dpkg -i JLink_Linux_V614h_x86_64.deb
```

# Get started with OpenThread

# Open Thread with UWB Radio Abstraction Layer on EVB1000 Radio and Nordic NRF52840 platform
## Hardware setup
The Connection details of the Hardware setup is explained in [Hardware_setup.md][HS].

[HS]: ./DW1000/doc/Hardware_setup.md

## Installing Tool chain and Building Openthread Applications.

1.To initialize the Environment Variables, installing Dependent Libraries (**First time**), setup(clone the OT repo and sym link creation for dw1000) and building the application.

Inside the cloned repo (ot-dw1000) run the script – `build_setup.sh INITIAL`
```bash
$ source ./build_setup.sh INITIAL
```
2.To initialize the Environment Variables, setup(clone the OT repo and sym link creation for dw1000) and building the application (**Next Time on Wards**).

Inside the cloned repo (ot-dw1000) run the script – `build_setup.sh Update`
```bash
$ source ./build_setup.sh UPDATE
```
3.To build the OT CLI and NCP application.

Inside the cloned repo (ot-dw1000) run the script – `build_setup.sh`

```bash
$ source ./build_setup.sh
```


## Flashing the binaries
Use Command line tools for flashing binary on to nrf52840.
In the previous steps `setup.sh` would have installed the command line tools `nrfjprog`.
```bash
$ cd $OT_ROOT/nrfjprog
$ sudo ./nrfjprog -f nrf52 --chiperase --program $OT_ROOT/output/bin/arm-none-eabi-ot-cli-ftd.hex
$ sudo ./nrfjprog -f nrf52 -r
```

## Running the CLI example

1. Prepare two boards with the flashed `CLI Example` (as shown above).
2. The CLI example uses UART connection. 
   To view raw UART output, start a terminal emulator like PuTTY and connect to the used COM port with the following UART settings:
    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - HW flow control: RTS/CTS
    
(or)

   Use Run the pyterm script provided in the `ot-dw1000/openthread-master/tools/pyterm` directory.
   On Linux system a port name should be called e.g. `/dev/ttyACM0` or `/dev/ttyACM1`.

3. Open a terminal connection on the first board and start a new Thread network.

 ```bash 
$ cd $OT_ROOT/tools/pyterm
$ sudo ./pyterm -p /dev/ttyACM0
  
 > channel 5
 Done
 > panid 0xdeca
 Done
 > ifconfig up
 Done
 > thread start
 Done
 ```

4. After a couple of seconds the node will become a Leader of the network.

 ```bash
 > state
 Leader
 ```

5. Open a terminal connection on the second board and attach a node to the network.

 ```bash
$ cd $OT_ROOT/tools/pyterm
$ sudo ./pyterm -p /dev/ttyACM1

 > channel 5
 Done
 > panid 0xdeca
 Done
 > ifconfig up
 Done
 > thread start
 Done
 ```

6. After a couple of seconds the second node will attach and become a Child.

 ```bash
 > state
 Child
 ```

7. List all IPv6 addresses of the first board.

 ```bash
 > ipaddr
 fdde:ad00:beef:0:0:ff:fe00:fc00
 fdde:ad00:beef:0:0:ff:fe00:9c00
 fdde:ad00:beef:0:4bcb:73a5:7c28:318e
 fe80:0:0:0:5c91:c61:b67c:271c
 ```

8. Choose one of them and send an ICMPv6 ping from the second board.

 ```bash
 > ping fdde:ad00:beef:0:0:ff:fe00:fc00
 16 bytes from fdde:ad00:beef:0:0:ff:fe00:fc00: icmp_seq=1 hlim=64 time=8ms
 ```

For a list of all available commands, visit [OpenThread CLI Reference README.md][CLI].

[CLI]: https://github.com/openthread/openthread/blob/master/src/cli/README.md

## Running the NCP example

The NCP example is explained in [NCP_Example][NCP].

[NCP]: ./DW1000/doc/NCP_Example.md

## Thread Border Router Example
The Thread Border Router example is explained in [Thread_Border_Router][TBR].

[TBR]: ./DW1000/doc/Thread_Border_Router.md


### Known Issues
* Observed that Discover Command displays network info all the channels instead of displaying for active channel.

### Known Limitations 
ActiveScan have a limitation of displaying only one node information. 
* During ActiveScan the node will broadcast a message to all the nodes and wait for responses. Since DW1000 doesn't support     CSMA/CA if all the responses are arriving in same time, only one node data will be successfully received by the scan initiator node. 
* A PC can support only two nodes while testing Open Thread CLI and NCP Application. While testing open-thread CLI and NCP application with multiple nodes we need to use multiple PCs for pyterm terminal.

## Documentation
* Userguide is available in DW1000/doc/PP_DecaWave_MAC_ReleaseNotes.pdf
* ReleaseNote is available in DW1000/doc/PP_DecaWave_MAC_UserGuide.pdf
