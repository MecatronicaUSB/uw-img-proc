# Project: uw-img-proc
# Module: contrastenhancement

Module that enhances the contrast of images using histogram adjustment approaches such as histogram stretching and equalization.
Current OpenCV 3.2 implementation does not support GPU acceleration.

## Getting Started

### Prerequisites

* OpenCV 3.2

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
cd modules/contrastenhancement
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ contrastenhancement input.jpg output.jpg -m=I -cuda=0 -time=1 -show=1
```
This will open 'input.jpg' enhance the contrast using the Integrated Color Model model (histogram stretching) and write it in 'output.jpg', while disabling GPU support, and showing total execution time as well as the comparison of the original and the color corrected images.

## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)