DESCRIPTION
------------
seviri_native_util is a C library that provides functionality to read, write, and pre-process SEVIRI image data in the Native SEVIRI Level 1.5 format distributed by U-MARF and the HRIT format from the MSG dissemination service (EUMETCast and direct).  It reads the level 1.5 files into a data structure, including the U-MARF header, the level 1.5 header, the image data (in 10 bit pixel counts), and the level 1.5 trailer.  Files with specially selected rectangular regions relative to the entire disk are fully supported.

The user may select any subset of channels to read and may select a rectangular region to read in either in pixel coordinates (relative to an entire SEVIRI disk) or in latitude and longitude.  Rectangular selection is supported for both full disk files or files with a previously selected region.

Once read in, the data can then be accessed, changed (subsetted, calibrated, etc.) and/or written to a new file.  In addition, the data may be preprocessed to obtain, radiance, reflectance, bidirectional reflectance factor, and/or brightness temperature, for any set of channels, along with pixel time, latitude, longitude, solar zenith angle, solar azimuth angle, sensor viewing zenith angle, and sensor viewing azimuth angle.


INSTALLATION
------------
The source maybe obtained by git cloning from github.com with:

git clone https://github.com/gmcgarragh/seviri_native_util.git

The main code is C89 compliant and is dependent only on the C standard library.  As such, it should compile with any modern C compiler and probably with most C++ compilers.  The build system requires GNU Make, with which the following steps will compile the code:

1) Copy make.inc.example to make.inc.
2) Edit make.inc according to the comments within.
3) Run make.

The default setup in make.inc.example is for GCC and should work on any Linux or MacOS systems with programming tools installed and probably most modern UNIX systems.

For other platforms/environments, such as MS Windows Visual C++, it is up to user to set up the build within that platform/environment.

After the build the relevant header and library file will be located in the same directory as the source.  It is up to the user to move these to or link to these from other locations.

In addition to the C interface seviri_native_util has a Fortran and Python interfaces that provide access to the most important functionality.  If these interfaces are desired uncomment the indicated lines in make.inc and adjust the Fortran compiler and options as desired.  The default setup is for GFortran.


USAGE
-----
seviri_native_util is meant to be used by a larger program requiring access to SEVIRI Level 1.5.  It is not a standalone program and as such must be linked with other programs.

Working example programs for C, Fortran, and Python are included with the distribution: 'example_c', 'example_f90', and 'example_py', respectively.  These are fully commented in the code and should be enough to get started using seviri_native_util.  These programs take a single argument, a Native or HRIT file name of the forms

MSG*-SEVI-MSG15-*

or

H-000-MSG*__-MSG*________-_________-EPI______-*-__

respectively, read sub-images of reflectance in two bands and brightness temperature in two bands, and print the values for the central pixel.


CONTACT
-------
For questions, comments, or bug reports contact Greg McGarragh at mcgarragh@atm.ox.ac.uk.

Bug reports are greatly appreciated!  If you would like to report a bug please include sample code that reproduces the bug along with the inputs and any relevant outputs.


REFERENCES
----------
GIESKE_A_S_M:
	A.S.M. Gieske, J. Hendrikse, V. Retsios , B. van Leeuwen, B.H.P. Maathuis , M. Romaguera, J.A. Sobrino, W.J. Timmermans, and Z. Su. Processing of MSG-1 SEVIRI data in the thermal infrared - Algorithm development with the use of the SPARC2004 data set.

PDF_MSG_SEVIRI_RAD2REFL:
	Conversion from radiances to reflectances for SEVIRI warm channels, Issue v4, 25 October 2012, EUMETSAT, Doc. No. EUM/MET/TEN/12/0332

PDF_CGMS_LRIT_HRIT_2_6:
	LRIT/HRIT global specification, Issue 2.6, 12 August 1999, CGMS, Doc. No. CGMS 03

PDF_EFFECT_RAD_TO_BRIGHTNESS-1:
	The Conversion from Effective Radiances to Equivalent Brightness Temperatures, Issue v1, 5 October 2012, EUMETSAT, Doc. No. EUM/MET/TEN/11/0569

PDF_TEN_05105_MSG_IMG_DATA:
	MSG level 1.5 image data format description, Issue v7, 4 December 2013, EUMETSAT, Doc. No. EUM/MSG/ICD/105

ROSENFELD_D_2004:
	D. Rosenfeld, I. Lensky, J. Kerkmann, S. Tjemkes Y. Govaerts, and H.P. Roesli. Applications of METEOSAT Second Generation (MSG) - Conversion from counts to radiances and from radiances to brightness temperatures and reflectances.  Version 1.1, 30 June 2004
