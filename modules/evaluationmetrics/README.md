# Project: uw-img-proc
# Module: evaluationmetrics

Quality assessment module. Calculates several metrics that allow to evaluate the quality of the image. These are: Entropy, Contrast, Luminance, Normalized Neighborhood Function, Comprehensive Assessment Function, MSE, PSNR, Sharpness, Number of Features detected, Histogram (RGB and LAB).
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
cd modules/evaluationmetrics
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ evaluationmetrics proc.jpg orig.jpg -cuda=0 -time=0 -m=X -show=0
```
This will open 'proc.jpg' and 'orig.jpg' and calculate all the evaluation metrics available and save them in a csv file


## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)