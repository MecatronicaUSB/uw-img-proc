# Project: uw-img-proc
# Module: fusion

Underwater image enhancement module that uses two different techniques for enhancing the input image and the associated weight maps that evaluate several image qualities such as global and local contrast, saliency and exposedness. Then it fuses the contribution of the enhanced versions using a multiscale approach.

The provided **cmake** file autodetects if CUDA is present, and enables GPU support.

## Getting Started

### Prerequisites

* OpenCV 3.2
* opencv-contrib 3.2
* CUDA 8.0 (for GPU support)
DA 8.0 (for GPU support)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
cd modules/fusion
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ fusion img1.jpg img2.jpg -cuda=0 -time=1 -show=1
```
This will open 'img1.jpg' enhance the image and write it in 'img2.jpg', while disabling GPU support, and showing total execution time as well as the comparison of the original and the enhanced images

## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)