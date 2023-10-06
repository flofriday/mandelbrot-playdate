# Mandelbrot Playdate

## Build it yourself

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<path to SDK>/C_API/buildsupport/arm.cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

This will create a mandelbrot.pdx executable which on macOS you can execute in 
the Simulator with `open mandelbrot.pdx`.