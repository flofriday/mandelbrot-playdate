# Mandelbrot Explorer
![Screenshot](screenshot.png)

A zoomable mandelbrot renderer for the playdate.

## Build for the Playdate

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<path to SDK>/C_API/buildsupport/arm.cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

This will create a mandelbrot.pdx executable which on macOS you can execute in 
the Simulator with `open mandelbrot.pdx`.

In the Simulator go to: Device -> Upload to Device.

Note: The simulator will crash upon loading the game as it is build for a 
differnt architecture. However you can still upload it to hte device.

## Build for the Simulator

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

This will create a mandelbrot.pdx executable which on macOS you can execute in 
the Simulator with `open mandelbrot.pdx`.

Note: Uploading this build to the playdate will not work because it was built
for the wrong architecture.