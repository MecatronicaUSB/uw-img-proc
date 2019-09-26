# Project: uw-img-proc
# Module: dehazing

Module that dehazes underwater images using the Bright Channel Prior. The atmospheric light and transmittance are estimated using a rectified bright channel image, and the input image is restored using a hazy image formation model.
Current OpenCV 3.2 implementation does not support GPU acceleration.

## Getting Started

### Prerequisites

* OpenCV 3.2
* OpenCV extra modules (OpenCV contrib 3.2)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
cd modules/dehazing
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ dehazing input.jpg output.jpg -cuda=0 -time=1	-show=1
```
This will open 'input.jpg' apply the Bright Channel Prior algorithm and write it in 'output.jpg', while disabling GPU support, and showing total execution time as well ad the comparison of the original and dehazed images.

## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)