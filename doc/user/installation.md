FMITerminalBlock Installation
=============================

Currently, FMITerminalBlock needs to be manually compiled. There is no binary release yet. Hence, the following sections describe the compilation procedures for Linux and Windows systems. FMITerminalBlock has the following dependencies.

* [CMake](https://cmake.org): FMITerminalBlock requires at least CMake version 3.1.0 to generate the build environment.
* C++ Toolchain: The C++ compiler must support C++11 standard and must be compatible with all other dependencies. Currently, FMITerminalBlock is tested with Microsoft Visual Studio 2015 compiler and GCC 5.4.
* [Boost Library 1.61.0](http://www.boost.org/doc/libs/1_61_0/): Boost provides several platform independent functions beyond the standard C++ library. The software is currently tested with version 1.61.0 only. Future Boost versions may also work but currently, no guarantee can be given. It is important that a binary release of Boost which is compiled with the target compiler of FMITerminalBlock is available. Further information on installing the Boost dependency is given below.
* [FMI++](https://sourceforge.net/projects/fmipp/): The FMI++ library handles several FMI-related aspects. It is assumed that the source code is available. It is not necessary to build FMI++ beforehand because the source code of FMI++ will be directly accessed. Since FMI++ is rapidly evolving, FMITerminalBlock is usually built and tested with the upstream version of FMI++.
* [Doxygen](http://www.stack.nl/~dimitri/doxygen/): Tool which is used to extract a source code documentation from annotated code. The Dogygen dependency is optional. If no installation is found, it is not possible to generate a compact source code documentation but FMITerminalBlock will still be compiled.
* [GIT](https://git-scm.com/): Version control system which is used to fetch and update the source code of FMI++ and FMITerminalBlock. In case GIT is not available, the source code can still be manually downloaded from the project websites.

## Step 0: Install the Toolchain
Please make sure that at least CMake, and a supported C++ toolchain is available. The CMake version must be compatible with the C++ toolchain. I.e. CMake must be able to generate the build environment for the particular toolchain. Old versions of CMake (such as 3.1.0) may not be able to generate more recent build environments such as Visual Studio 2015 projects. 

## Step 1: Install Boost
A Boost 1.61.0 installation needs to be available for the chosen compiler. For various Microsoft Visual Studio compilers up to MSVC 14 (Visual Studio 2015), binary packages are available at [https://sourceforge.net/projects/boost/files/boost-binaries/1.61.0/](https://sourceforge.net/projects/boost/files/boost-binaries/1.61.0/). It is usually sufficient to install (extract) the files to a common location such as ```C:\Program Files\boost\boost_1_61_0```.

For other compilers such as GCC, Boost has to be compiled manually. It is recommended to build both, the static and the dynamic library version. The following commands illustrate to build process on Windows (Using MinGW-64/Cygwin). Please refer to the [Boost documentation](http://www.boost.org/doc/libs/1_61_0/more/getting_started/unix-variants.html) for further details on compiling and installing Boost.
```
boost_1_61_0> bootstrap.bat
boost_1_61_0> b2.exe toolset=gcc --build-type=complete link=static,shared
boost_1_61_0> b2.exe toolset=gcc --build-type=complete link=static,shared --prefix="bin.install" install
```

In any case it is expected that the header and binary files are available in a common Boost installation directory. Within that directory, it is expected that all header files are located in the ```boost``` subdirectory and all binary files are located in a compiler specific ```lib*``` directory. The installation directory is also referred as Boost-root directory.

## Step 2: Download FMI++ and FMITerminalBlock Source Files
The [FMI++](https://sourceforge.net/p/fmipp/code/ci/master/tree/) as well as the [FMITerminalBlock](https://github.com/AIT-IES/FMITerminalBlock) source code are managed with GIT. To obtain the source code, both projects need to be cloned. The following commands download the source codes of both projects.
```
Development> git clone git://git.code.sf.net/p/fmipp/code fmipp
Development> git clone https://github.com/AIT-IES/FMITerminalBlock.git FMITerminalBlock
```

Alternatively, the source code may be directly downloaded on the project websites.

## Step 3: Configure and Generate the Build Environment

The build environment needs to be configured and generated via CMake. A GUI as well as a CUI version of CMake are available. It may be necessary to configure several options until the build environment can be generated successfully. First, chose the binary and source code directory of FMITerminalBlock. On using the CMake GUI, a wizard will guide the initial compiler setup. A first configuration run may be stared to initialize the default configuration options. Most likely, the initial configuration run will fail but most options will be set to the default values.

The configuration scripts need to locate the Boost directories. The Boost library subdirectories may either be added to the PATH environment variable or the CMake option BOOST_ROOT needs to be set to the Boost installation directory.

The source code location of FMI++ needs to be set in the CMake option fmipp_PROJECT_DIR. The default value is set to ```../fmipp```. As soon as FMI++ is found, FMI++ may be configured via the same CMake instance too. Nevertheless. The default options should be sufficient to configure and generate the build environment.

## Step 4: Compile the Project

The project is compiled by the generated build environment. For instance, for Microsoft Visual Studio, a solution file which covers all necessary targets is provided. Similarly, appropriate Makefiles and Eclipse projects may be generated as well. Use the default mechanism to build FMITerminalBlock and all test-cases or invoke the corresponding targets to execute the test cases and to build the Doxygen documentation.
