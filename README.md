# Underwater Image and Video Processing

C++ implementation of underwater image and video enhancement using OpenCV 3.2.

<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/uw_enhancement.png"/>
</p>

## Table of Contents
- [Modules list](#modules-list)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Software details](#software-details)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)
- [Constributors](#constributors)

## Modules list
- [Color Correction](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/colorcorrection)

Color correction module using White World Assumption
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/unsupervised_color_correction.png"/>
</p>

- [Contrast Enhancement](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/contrastenhancement)

Contrast enhancement module using Histogram Stretching and Equalization using Rayleigh distribution 
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/integrated_color_model.png"/>
</p>

- [Dehazing](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/contrastenhancement)

Dehazing module using Bright Channel Prior
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/dehazing.png"/>
</p>

- [Illumination](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/illumination) 

Illumination correction module using a Homomorphic High-pass Filter
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/homomorphic_filter.png"/>
</p>

- [Fusion](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/fusion)

Image fusion module using a Multiscale approach
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/fusion.png"/>
</p>

- [Video Enhancement](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/videoenhancement)

Video enhancement module
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/white_world_assumption.png"/>
</p>

- [Evaluation Metrics](https://github.com/roboTech-github/Underwater-Image-Processing/tree/master/modules/evaluationmetrics)

Image quality metrics module (Contrast, Entropy, Features, etc)
<p align="center">
  <img src="https://github.com/roboTech-github/Underwater-Image-Processing/blob/master/images/rayleigh_equalization.png"/>
</p>

## Requirements

The current release has been developed and tested in Windows 10 64 bits

- [OpenCV 3.2](http://opencv.org) and extra modules (OpenCV contrib 3.2).
- [CMAKE 2.8](https://cmake.org/)

## Getting Started

To start using this project, proceed to the standard *clone* procedure:

```bash
git clone https://github.com/roboTech-github/Underwater-Image-Processing.git
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

**Underwater-Image-Processing** is a free and open source software licensed under the [GNU GPLv3.0 License](https://en.wikipedia.org/wiki/GNU_General_Public_License), unless otherwise specified in particular modules or libraries (see LICENSE and README.md).

Copyright (c) 2017-2018 Grupo de Investigación y Desarrollo en Mecatrónica (<mecatronica@usb.ve>).

## Author

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)

## Contributors

* **José Cappelletto** - [cappelletto](https://github.com/cappelletto)