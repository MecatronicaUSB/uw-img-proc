# Underwater Video and Image Processing

C++ implementation of underwater image and video enhancement using OpenCV 3.2.

**uw-img-proc** is a free and open source software licensed under the [GNU GPLv3.0 License](https://en.wikipedia.org/wiki/GNU_General_Public_License), unless otherwise specified in particular modules or libraries (see LICENSE and README.md).

## Table of Contents
- [Modules list](#modules-list)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Software details](#software-details)
- [Contributing](#contributing)
- [License](#license)

## Modules list
- [colorcorrection](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/colorcorrection) Color correction module using the Gray World Assumption in different color spaces
- [contrastenhancement](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/contrastenhancement) Contrast enhancement module using histogram stretching and equalization
- [evaluationmetrics](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/evaluationmetrics) Image quality metrics module
- [fusion](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/fusion) Image fusion module using a multiscale approach
- [illumination](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/illumination) Illumination correction module using a homomorphic high-pass filter
- [videoenhancement](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/videoenhancement) Video enhancement module

Each module contains information describing its usage, with a useful README file.

## Requirements

The current release has been developed and tested in Windows 10 64 bits

- [OpenCV 3.2](http://opencv.org) and extra modules (OpenCV contrib 3.2).
- [cmake 2.8](https://cmake.org/) - cmake making it happen again

## Getting Started

To start using this project, proceed to the standard *clone* procedure:

```bash
cd <some_directory>
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
```

## Software Details

- Implementation done in C++.

## Contributing

Summary of contributing guidelines:

* One pull request per issue;
* Choose the right base branch;
* Include tests and documentation;
* Use small datasets for testing purposes;
* Follow always the same coding style guide. If possible, apply code formating with any IDE.

## License

Copyright (c) 2017-2018 Grupo de Investigación y Desarrollo en Mecatrónica (<mecatronica@usb.ve>).
Released under the [GNU GPLv3.0 License](LICENSE). 

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)
* **José Cappelletto** - [cappelletto](https://github.com/cappelletto)