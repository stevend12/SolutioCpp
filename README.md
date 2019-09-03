# SolutioCpp

This repository was started with the goal of creating a C++ framework for
applications of physics in medicine, particularly radiation therapy and medical
imaging. From this framework applications can be created that are
high-performance, inexpensive, and transparent (i.e. removing the "black box"
from medical software). The repository is a companion to the website
[Solutio in silico](http://www.solutioinsilico.com).

## Install
CMake is used to build the library and examples. The following packages are
required and need to be installed separately:

  * ITK (minimum install only)
  * FFTW

Some packages (fftw++) are included with the source.
  
## Examples
TODO

## Project Status

The framework is currently in the pre-release stage (i.e. v0.y.z). The plan is
for version 1.0.0 to at least include the following features:

  * Table lookup for NIST photon data attenuation coefficient data (NISTPAD)
  * Implementation of the TASMIP algorithm for imaging x-ray source spectra
  * Generation of primary-only CT projection data for various geometric objects (i.e. no Monte Carlo, only ray-tracing and exponential attenuation)
  * Filtered packprojection (FBP) reconstruction algorithm for helical and axial CT projection data
  * Data container for CT images with DICOM read/write functionality (via ITK)
  * Corrections-based dose calculation for linac photon beams (AAPM TG 71 formalism)
  * Brachytherapy dose calculation (AAPM TG 43 formalism)
  * Linear and logarithmic data interpolation functions for the above features

Although this list will likely increase during this intial stage. Currently I
(Steven) am loading source code and examples from my PhD work to provide a base
for the repository.

## Version Control
Version control for this repository follows the format of [Semantic Version](http://www.semver.org).
In short, the version number is represented as x.y.z, where:
  * x: Major version (for new, backwards-incompatible changes)
  * y: Minor version (for new, backwards-compatible changes)
  * z: Patch version (for backwards-compatible bug fixes)

## License

The framework will be distributed under the Apache 2.0 License, enabling all
users to develop their own applications for both personal and commerical use.
The main conditions are that you attach the Apache 2.0 license to any source
code you use, and that any changes you make are properly documented. Please read
the [LICENSE](./LICENSE) file for more information about the Apache 2.0 License.
 
