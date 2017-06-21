# COAP Based Cloud Application:
COAP Based Cloud Application which will send the integer data to the cloud through dw1000 radio and Nordic Host controller with the coap support in openthread stack.
The setup for this example is same as defined in the thread border router example.

Steps to be followed to test the COAP Based Cloud Application is defined as follows.
1. Register with cloud (things.io)
2. Create a new Project
3. Add the token in the CLI source code and recompile the code.
4. Create a widget on the web interface
5. Testing

Here will explain these steps in detail
## 1. Register with cloud (www.things.io)
For a detailed description of the sign-up procedure and an introduction to the web interface of thethings.io, complete the steps described in thethings.io Getting Started.
## 2. Create a new product:
        * Log in to the main control panel.
        * Go to the Things Manager page.
        * Create a new product. Select Raspberry Pi as Board
        * Choose JSON as the data type.
        * Activate the Thing by clicking Activate More Things.
        * After the Thing has been activated, copy the related Thing Token.

## 3. Add the token in the CLI source code and recompile the code.

**Note:** The Token must be copied into the CLI application source code. i.e., The value can be found in the header of the file coap_api.cpp represented as the CLOUD_URI_PATH define. This file located in openthread-master/src/core/api/ directory. Replace Thing Token ({THING_TOKEN} string) with the appropriate one, obtained in the process of Cloud setup.

```bash
 #define CLOUD_URI_PATH            "v2/things/{THING_TOKEN}"
```
**EX:**
```bash
 #define CLOUD_URI_PATH            "v2/things/DFdOKr5AHo_7Aj-L7UNnIO4BSunTvQeaJSgCWILuYA0"
```
 * After replacing the token, recompile the code with enabling coap support in the source code.
```bash
COAP=1 ENABLE_DW1000=1 make -f examples/Makefile-nrf52840-dw1000
```
 * Flash the CLI on one node and NCP on another node and connect to the Raspberry Pi as mentioned in the `Thread_Border_Router` setup.
 * Start the Thread Network as Mentioned in the `Thread Border Router Example`.
 * On cli node by using coapsend command Push the first integer value to the cloud
```bash
coapsend 45
```
## 4. Create a Widget on the web Interface to monitor the interger values
Go to the Dashboard page.

       * Optionally, remove all temporary created widgets by clicking Edit Dashboard.
       * Click Add Widget (+).
       * Fill in the name of the widget, for example Temperature, choose Thing Resource as a Data Source, and temp as a Resource.
       * Use Gauge as Widget Type and check the Realtime check box.
       * Optionally, you may also fill in the ranges ,units, for example Celsius.
       * You can observe the the Guage widget with Temparature value as 45.
## 5. Testing:
Now, from the CLI node by continously using coapsend command you can observe the changes on the web interface .
```bash
coapsend 50
coapsend 30
```
