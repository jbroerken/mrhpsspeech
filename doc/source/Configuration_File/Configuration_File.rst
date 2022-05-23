******************
Configuration File
******************
The mrhpsspeech service loads a configuration file which contains information about the 
voice and text string speech methods as well as the configuration settings for the used 
API provider speech methods.

The configuration file uses the MRH Block File format.

File Structure
--------------
The block file stores general service information, the voice audio settings, the text string 
settings and the API provider settings in individual blocks. The general service settings are 
found in the **Service** block, the voice audio settings in the **Voice** block and the 
the text string settings in the **TextString** block.

Individual API provider settings for text to speech and speech to text are also found in their 
own individual blocks, which should be named after the API itself.

.. note:: 

    Specific API provider settings are not listed here. Please check the 
    provided configuration files for explanations on API provider settings.


Service Block
-------------
The Service block stores the following values:

.. list-table::
    :header-rows: 1

    * - Key
      - Description
    * - MethodWaitMS
      - The time to wait before processing a method in milliseconds.
        
Voice Block
-----------
The Voice block stores the following values:

.. list-table::
    :header-rows: 1

    * - Key
      - Description
    * - SocketPath
      - The full path to the socket file to exchange voice 
        data on.
    * - RecordingKHz
      - The KHz frequency to use when recording using a signed 
        PCM 16-bit mono format.
    * - PlaybackKHz
      - The KHz frequency to use for playback using a signed PCM 
        16-bit mono format.
    * - RecordingTimeoutS
      - The timeout until recorded audio is transcribed.
    * - APIProvider
      - The speech to text and text to speech API provider used.
        
TextString Block
----------------
The TextString block stores the following values:

.. list-table::
    :header-rows: 1

    * - Key
      - Description
    * - SocketPath
      - The full path to the socket file to exchange text string 
        data on.
    * - RecieveTimeoutS
      - The time in seconds until a text string communication is 
        considered finished.

Example
-------
The following example shows a user service configuration file with 
default locations and names:

.. code-block:: c

    <MRHBF_1>
    
    <Service>{
        <MethodWaitMS><100>
    }

    <Voice>{
        <SocketPath></tmp/mrh/mrhpsspeech_voice.sock>
        <RecordingKHz><44100>
        <PlaybackKHz><44100>
        <RecordingTimeoutS><3>
        <APIProvider><0>
    }

    <TextString>{
        <SocketPath></tmp/mrh/mrhpsspeech_text.sock>
        <RecieveTimeoutS><30>
    }
    
