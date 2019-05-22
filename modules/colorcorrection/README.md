# Project: uw-img-proc
# Module: colorcorrection

Module that balances the color of underwater images using the Gray World Assumption applied in the chromatic components of Ruderman's laB color space. The input image is transformed into the laB color space, adjusted using Gray World, and then converted back to RGB color space.
Current OpenCV 3.2 implementation uses GPU acceleration for channel split/merge through CUDA library.

The provided **cmake** file autodetects if CUDA is present, and enables GPU support.

## Getting Started

### Prerequisites

* OpenCV 3.2
* CUDA 8.0 (for GPU support)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
cd modules/colorcorrection
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ colorcorrection input.jpg output.jpg -cuda=0 -time=1 -show=1
```
This will open 'input.jpg' correct the color and write it in 'output.jpg', while disabling GPU support, and showing total execution time as well as the comparison of the original and the color corrected images

## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)