# Underwater Image Processing

Implementation of Underwater Image/Video Enhancement. Using OpenCV 3.2.

**uw-img-proc** is a free and open source software licensed under the [GNU GPLv3.0 License](https://en.wikipedia.org/wiki/GNU_General_Public_License), unless otherwise specified in particular modules or libraries (see LICENSE and README.md).

## Table of Contents
- [Modules list](#modules-list)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Software details](#software-details)
- [Contributing](#contributing)
- [License](#license)

## Modules list
- [colorcorrection](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/colorcorrection) Color correction module using 3 versions of the Gray World Assumption
- [contrastenhancement](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/contrastenhancement) Contrast enhancement module using histogram stretching/equalization
- [evaluationmetrics](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/evaluationmetrics) Image Quality Metrics module
- [fusion](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/fusion) Image warping and stitching
- [illumination](https://github.com/MecatronicaUSB/uw-img-proc/tree/master/modules/illumination) Illumination correction module using a homomorphic filter
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

Summary of contributing guidelines (based on those of OpenCV project):

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
