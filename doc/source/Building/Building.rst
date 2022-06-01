********
Building
********
The project files are already prepared and include everything except the 
required dependencies. This document lists both dependencies as well as the 
build process to create the library from source.

Supported Platforms
-------------------
The supported platforms for mrhpsspeech are UNIX-likes, primarily 
Debian-based GNU/Linux Distributions like the Rapsberry Pi OS.

Default Dependencies
--------------------
This project has the following default dependencies:

* mrhshared: https://github.com/jbroerken/mrhshared/
* libmrhbf: https://github.com/jbroerken/libmrhbf/
* libmrhevdata: https://github.com/jbroerken/libmrhevdata/
* libmrhpsb: https://github.com/jbroerken/libmrhpsb/
* libmrhls: https://github.com/jbroerken/libmrhls/

Optional Dependencies
---------------------
Optional dependencies are used for API providers to convert speech to text 
and text to speech:

.. note::

    The dependencies change depending on the API providers used.


.. toctree::
   :maxdepth: 1

   Dependencies/Google_Cloud_API


Build Tools
-----------
This release includes a CMake script (CMakeLists.txt) for a simplified build 
process. The minimal required version for CMake is 3.1.
Also needed is the GNU C++ Compiler. Full C++14 support is required.

Changing Options
----------------
Listed in the CMakeLists.txt file are options which define what type of API 
provider for audio processing are used. Also included is an option to define 
if the text string based communication should be included or not.

.. list-table::
    :header-rows: 1

    * - Option
      - Description
    * - USE_VOICE
      - Use voice input and output.
    * - USE_TEXT_STRING
      - Use text string based input and output.
    * - API_PROVIDER_GOOGLE_CLOUD_API
      - Use the Google Cloud API for speech processing.
      

Changing Pre-defined Settings
-----------------------------
Listed in the CMakeLists.txt file are preprocessor macros used to specify 
default file paths and other information. Change these to fit the requirements 
of the target use case.

.. list-table::
    :header-rows: 1

    * - Macro
      - Description
    * - MRH_SPEECH_SERVICE_THREAD_COUNT
      - The number of threads to use for callbacks.
    * - MRH_SPEECH_CONFIGURATION_PATH
      - The file path to the speech service configuration file.
    * - MRH_SPEECH_SERVICE_PRINT_INPUT
      - Print received speech input to console.
    * - MRH_SPEECH_SERVICE_PRINT_OUTPUT
      - Print received speech output to console.
    * - MRH_SPEECH_USE_VOICE
      - Enable voice based speech input and output. This is set by 
        the USE_VOICE option.
    * - MRH_API_PROVIDER_GOOGLE_CLOUD_API
      - Use the Google Cloud API for speech processing. This is set 
        by the API_PROVIDER_GOOGLE_CLOUD_API option.
    * - MRH_SPEECH_USE_TEXT_STRING
      - Use text string based input and output. This is set by the 
        USE_TEXT_STRING option.


Build Process
-------------
The build process should be relatively straightforward:

1. Aqquire dependencies for the type of build.
2. Move into the project "build" folder.
3. Check required settings for the wanted type of build.
4. Compile Makefiles with the included CMakeLists.txt.
5. Run "make" to compile the binary.
6. Install the compiled binary.
7. Acquire and install any required files from the mrh_os_files 
   (https://github.com/jbroerken/mrh_os_files/) repository.

Shell Commands
--------------
The following shell commands will create makefiles with the 
provided CMakeLists.txt using the default settings, compile 
the project with the created makefiles and install the binary:

.. code-block::

    cd <Project Root Folder>/build
    cmake ..
    make
    sudo make install
