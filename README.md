# mrhpsspeech

<p align="center">
<img width="100%" height="100%" src="/doc/source/banner.png">
</p>


## About

The mrhpsspeech project provides the default speech platform service for the MRH platform.
It manages both text and voice based input and output.


## Requirements

#### Compilation

This project is built using CMake. You can find CMake here:

https://cmake.org/

#### Library Dependencies

This project requires other libraries and headers to function:

Dependency | Source
---------- | ------
libmrhbf |  https://github.com/jbroerken/libmrhbf/
libmrhcevs | https://github.com/jbroerken/libmrhcevs/
libmrhev | https://github.com/jbroerken/libmrhev/
libmrhpsb | https://github.com/jbroerken/libmrhpsb/
mrhshared | https://github.com/jbroerken/mrhshared/
Sodium | https://libsodium.gitbook.io/doc/
PortAudio | http://files.portaudio.com/download.html
libsamplerate: http://libsndfile.github.io/libsamplerate/index.html
PocketSphinx | https://cmusphinx.github.io/wiki/download/
google-cloud-cpp | https://github.com/googleapis/google-cloud-cpp

For more information about the requirements, check the "Building" section found in the documentation.


## Documentation

All documentation is build with sphinx-doc using the Read The Docs theme.
To build the documentation, grab the requirements for it:

#### sphinx-doc
https://www.sphinx-doc.org/en/master/

#### Read The Docs Theme
https://sphinx-rtd-theme.readthedocs.io/en/stable/

## Licence

This project is licenced under the GNU General Public 2 licence. 
Please read the included LICENCE.txt for the exact terms.


## Directories

This project supplies multiple directories for the development of said project. 
Their names and descriptions are as follows:

Directory | Description
--------- | -----------
bin | Contains the built project executables.
build | CMake build directory.
doc | Documentation files.
src | Project source code.
tools | Project support tools.
