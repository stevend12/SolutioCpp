# SolutioCpp

This C++ library is a collection of containers and algorithms for applications
of physics in medicine, particularly radiation therapy (RT) and medical imaging.
This library can be the foundation for building clinical, research, and
educational applications in these medical fields. The repository is a companion
to [Solutio in silico](solutioinsilico.com), a website designed to promote
science and technology concepts in medicine. More specific documentation for
this source code can be found on the website.

## Features
Key features of SolutioCpp include:
  * Basic physics equations, constants, and data relevant to medical physics
  * Containers to store common imaging/RT components (images, RT dose, etc.)
  * Read data from DICOM files and other vendor-specific file formats
  * Simulation of medical technologies (e.g. X-ray CT)
  * Algorithms from clinical guideline reports (AAPM TG 43, AAPM TG 71, etc.)

## Install
[CMake](cmake.org) is used to build the library and examples. The following packages are
required and need to be installed separately:
  * FFTW
  * DCMTK
  * ITK

One package is included with the source (fftw++).

## Use & Examples
It is highly recommended to use CMake to compile SolutioCpp with other programs.
The above packages will likely also need to be included in projects using SolutioCpp.
See the Examples folder for simple uses of the SolutioCpp library to build other programs.

## Project Status
The framework is currently in the pre-release stage (i.e. v0.y.z). The plan is
for version 1.0.0 to at least include the following features:
  * Read and lookup data for NIST photon attenuation coefficients (NISTX) and electron stopping power ratios (ESTAR)
  * Implementation of the TASMIP algorithm for imaging x-ray source spectra
  * Generation of primary-only CT projection data for various geometric objects (i.e. no Monte Carlo, only ray-tracing and exponential attenuation)
  * Filtered back-projection (FBP) reconstruction algorithm for helical and axial CT projection data
  * Reading medical images and RT entities from DICOM files
  * Corrections-based dose calculation for linac photon beams (AAPM TG 71 formalism)
  * Brachytherapy dose calculation (AAPM TG 43 formalism)
  * Linear and logarithmic data interpolation functions for the above features

Although this list may increase during this initial stage.

## Version Control
Version control for this repository follows the format of [Semantic Version](http://www.semver.org).
In short, the version number is represented as x.y.z, where:
  * x: Major version (for new, backwards-incompatible changes)
  * y: Minor version (for new, backwards-compatible changes)
  * z: Patch version (for backwards-compatible bug fixes)

## License
The framework will be distributed under the Apache 2.0 License, enabling all
users to develop their own applications for both personal and commercial use.
The main conditions are that you attach the Apache 2.0 license to any source
code you use, and that any changes you make are properly documented. Please read
the [LICENSE](./LICENSE) file for more information about the Apache 2.0 License.
