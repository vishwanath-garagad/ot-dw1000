# Working with Openthread NCP Appliction:
NCP is a low-power Wireless Network Co-Processor, can be a Border Router to connect the thread network with the external network with Wpantund Interface Running on the host (Linux PC).

## Running NCP with Wpantund
1. Flash the NCP binary file on to the Nordic board and connect it to Linux PC via USB port.
2. Open a Terminal and Run the following command
```bash
sudo /usr/local/sbin/wpantund -o NCPSocketName /dev/ttyACM0 -o WPANInterfaceName wpan0
```
The above command will run the Wpantund Interface on the Linux PC with name wpan0 and initializes the NCP.

3. To configure the NCP ,wpantund provides “wpanctl” command line tool. To use it Open an another terminal and run the following command
```bash
sudo wpanctl -I wpan0
```
A command line to control/configure NCP appears as  `wpanctl:wpan0`
```bash
wpanctl:wpan0>
```
4.Now by following NCP commands , we can configure the NCP.  `help` command will list the available commands.
```bash
Wpanctl:wpan0>help
Commands:
   join                          Join a WPAN.
   form                          Form a new WPAN.
   attach                        Attach/resume a previously commissioned network
   reset                         Reset the NCP
   begin-low-power               Enter low-power mode
   leave                         Abandon the currently connected WPAN.
   poll                          Poll the parent immediately to see if there is IP traffic
   config-gateway                Configure gateway
   add-route                     Add external route prefix
   remove-route                  Remove external route prefix
   list                          List available interfaces.
   status                        Retrieve the status of the interface.
   permit-join                   Permit other devices to join the current network.
   scan                          Scan for nearby networks.
   mfg                           Execute manufacturing command.
   getprop                       Get a property.
   setprop                       Set a property.
   begin-net-wake                Initiate a network wakeup
   host-did-wake                 Perform any host-wakeup related tasks
   pcap                          Start a packet capture
   cd                            Change current interface (command mode)
   quit                          Terminate command line mode.
   help                          Display this help.
   clear                         Clear shell.
wpanctl:wpan0>
```
5. `status` command will display the status of the NCP.
```bash
wpanctl:wpan0>status
wpan0 => [
        "NCP:State" => "offline"
        "Daemon:Enabled" => true
        "NCP:Version" => "OPENTHREAD/0.01.00; NRF52840; Mar 27 2017 10:06:23"
        "Daemon:Version" => "0.08.00 (/f911961-dirty; Mar 31 2017 11:32:13)"
        "Config:NCP:DriverName" => "spinel"
        "NCP:HardwareAddress" => [B8DD443A6E973DEB]
]
```
## Making NCP as end device and CLI node as Leader:
1. Start a CLI node in a seperate serial terminal (with suitable parameters,let's assume PANID=0xdeca, Channel 5 ), so it will become Leader of the newly formed network.

2. To Join NCP Node to the above network , use `scan` and `Join` commands. On wpanctl
```bash
wpanctl:wpan0> scan
   | Joinable | NetworkName        | PAN ID | Ch | XPanID           | HWAddr           | RSSI
---+----------+--------------------+--------+----+------------------+------------------+------
 1 |       NO | "OpenThread"       | 0xDECA | 5 | DEAD00BEEF00CAFE | AA9D0AFEC741A253 |  -67
```
3. Use `join` command with Network Id the with option
```bash
wpanctl:wpan0> join 1
Joining "OpenThread" DEAD00BEEF00CAFE as node type "end-device"
Successfully Joined!
```
4. Check the `status`.
```bash
wpanctl:wpan0> status
wpan0 => [
        "NCP:State" => "associated"
        "Daemon:Enabled" => true
        "NCP:Version" => "OPENTHREAD/0.01.00; none; May 12 2017 11:47:38"
        "Daemon:Version" => "0.08.00d (/f911961-dirty; May  5 2017 10:52:26)"
        "Config:NCP:DriverName" => "spinel"
        "NCP:HardwareAddress" => [18B4300000000002]
        "NCP:Channel" => 5
        "Network:NodeType" => "end-device"
        "Network:Name" => "Open Thread"
        "Network:XPANID" => 0x64941B1C5D9C8F10
        "Network:PANID" => 0xDECA
        "IPv6:LinkLocalAddress" => "fe80::24b1:b3df:dc48:9585"
        "IPv6:MeshLocalAddress" => "fd64:941b:1c5d:0:7c34:ca08:8028:352c"
        "IPv6:MeshLocalPrefix" => "fd64:941b:1c5d::/64"
        "com.nestlabs.internal:Network:AllowingJoin" => false
]
wpanctl:wpan0>
```
5. It is Possible to ping with the MeshLocalAddress of NCP from the CLI node using ping command.
On cli run this command
```bash
ping fd64:941b:1c5d:0:7c34:ca08:8028:352c
 16 bytes from fd64:941b:1c5d:0:7c34:ca08:8028:352c icmp_seq=1 hlim=64 time=121ms
```
6. Also from the Linux terminal , it is possible to ping with the above IP address and the IP addresses available on the CLI node.

On Linux Terminal (use ping6 )
```bash
ping6 fd64:941b:1c5d:0:7c34:ca08:8028:352c
PING fd64:941b:1c5d:0:7c34:ca08:8028:352c(fd64:941b:1c5d:0:7c34:ca08:8028:352c) 56 data bytes 
64 bytes from fd64:941b:1c5d:0:7c34:ca08:8028:352c: icmp_seq=1 ttl=64 time=0.015 ms
64 bytes from fd64:941b:1c5d:0:7c34:ca08:8028:352c: icmp_seq=1 ttl=64 time=0.015 ms
64 bytes from fd64:941b:1c5d:0:7c34:ca08:8028:352c: icmp_seq=1 ttl=64 time=0.015 ms
.
.
```
## Making NCP as Leader  and CLI node as end-device/Child:
1. Start a NCP Node by flashing the NCP binary onto one Nordic board.
2. Open the wpanctl to confiure the NCP.
3. By using `form` command , NCP can create a Network as a Leader
```bash
wpanctl:wpan0> form Thread_Name
Forming WPAN "Thread_Name" as node type "router"
Successfully formed!
```
4. Now check the `status`.
```bash
wpanctl:wpan0> status
wpan0 => [
        "NCP:State" => "associated"
        "Daemon:Enabled" => true
        "NCP:Version" => "OPENTHREAD/0.01.00; none; May 12 2017 11:47:38"
        "Daemon:Version" => "0.08.00d (/f911961-dirty; May  5 2017 10:52:26)"
        "Config:NCP:DriverName" => "spinel"
        "NCP:HardwareAddress" => [18B4300000000002]
        "NCP:Channel" => 19
        "Network:NodeType" => "leader"
        "Network:Name" => "Thread_Name"
        "Network:XPANID" => 0x64941B1C5D9C8F10
        "Network:PANID" => 0x803B
        "IPv6:LinkLocalAddress" => "fe80::24b1:b3df:dc48:9585"
        "IPv6:MeshLocalAddress" => "fd64:941b:1c5d:0:7c34:ca08:8028:352c"
        "IPv6:MeshLocalPrefix" => "fd64:941b:1c5d::/64"
        "com.nestlabs.internal:Network:AllowingJoin" => false
]
wpanctl:wpan0>
```
5. `getprop` will list all the parameters of the NCP Node.
```bash
wpanctl:wpan0>getprop
```
6. Now Run a CLI node in second board with the same parameters observed on the NCP (Panid , channel , Masterkey).
7. Soon , CLI node will join the Network as a child. It can be ensured by using `state` command and `parent` commands on the CLI Node.
8. It is Possible to ping with the MeshLocalAddress of NCP from the CLI node using `ping` command.
 On CLI Node  run this command

```bash
 Ping fd64:941b:1c5d:0:7c34:ca08:8028:352c
   > 16 bytes from fd64:941b:1c5d:0:7c34:ca08:8028:352c icmp_seq=1 hlim=64 time=121ms
```
