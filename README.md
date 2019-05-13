# Gaus ESP32 FreeRTOS Demo

A simple ESP32 Demo intended to run on a Huzzah32 board.

The project is licenced with MIT licence except subprojects, which have relevant licence files included

While the source size is quite large, this is because we've grabbed a copy of:
- libcurl
- libjansson
- libgaus

If you're coming here with an interest to integrate the
[gaus c reference lib](https://github.com/gaus-fleet-lab/reference-c-lib) with your own code you should start by
checking out: (gaus_demo.c)[https://github.com/gaus-fleet-lab/esp32-freertos-demo/blob/master/main/gaus_demo.c].  You
can also pretty much ignore the files under `components` as they're just external libs copied into this project,
`libgaus` plus it's two dependencies: `libcurl`, and `libjansson`.

# Building

In order to build you'll need the esp32 tools as described here: [Esp32 Getting Started](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)  It's suggested that you use the same version as we use in CI.  You can check out the (CI Dockerfile)[https://github.com/gaus-fleet-lab/esp32-freertos/blob/master/Dockerfile] for hints.

Once you have all of the tools installed you'll need to:
- Install `automake`
- Install `libtool`
- Save copy of ca-cert for your guas instance to `main/ca_cert.pem`
- From project root directory run `make menuconfig` and set correct settings in "Gaus Demo Configuration".
  - WiFi SSID
  - WiFi Password
  - Gaus server url
  - Gaus Product Access
  - Gaus Product Secret
  - Gaus Device Id
- From project root directory run:
  - `make`
  - `make flash` (To flash)
  - `make monitor` (To see log messages from the device)

# Program flow

Currently the program does the following:
- Initialize libgaus
- Print some nice information
- Sleep a while
- Restart
