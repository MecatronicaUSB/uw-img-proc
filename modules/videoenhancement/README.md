# Project: uw-img-proc
# Module: videoenhancement

Underwater video enhancement module that applies a filter to all the frames of a video and saves the enhanced version in "avi" format. 
Current OpenCV 3.2 implementation does not support GPU acceleration.

## Getting Started

### Prerequisites

* OpenCV 3.2
* OpenCV extra modules (OpenCV contrib 3.2)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uw-img-proc.git
cd modules/videoenhancement
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

```
$ videoenhancement v1.mp4 -cuda=0 -time=1 -comp=1 -m=E
```
This will open 'v1.mp4' enhance the video using histogram equalization and write it in 'v1_E.avi', while disabling GPU support, showing total execution time and save the comparison of the original and the enhanced videos.

## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen

## Contributing

See contributing guidelines for base project **uw-img-proc**

## Versioning

Github

## Authors and Contributors

* **Geraldine Barreto** - [geraldinebc](https://github.com/geraldinebc)