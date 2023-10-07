# Mandelbrot Playdate
![Screenshot](screenshot.png)

## Build for the Simulator

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

This will create a mandelbrot.pdx executable which on macOS you can execute in 
the Simulator with `open mandelbrot.pdx`.

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

Note: The simulator might not run the game or display a different build as this 
binary is not compatible with it.