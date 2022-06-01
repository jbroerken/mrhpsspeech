*****
Voice
*****
Voice speech input is handled by receiving an audio buffer from an external 
source, which is then collected until all audio was received. The full audio 
is then transcribed to text with the help of an external speech to text API 
provider.

Receiving Audio
---------------
Audio is received by an external source by using a local stream socket. 
This allows the speech service to work with many different solutions to 
audio input, like multiple microphones or a intercom style system for 
example.

.. note::

    The local stream message used to receive voice audio from an 
    external source is **MRH_LS_M_AUDIO**.
    

.. note::

    Audio is required to be in **mono channel** format and a sample rate (KHz) matching 
    the configuration of the speech service.


Voice audio is collected and considered in progress until no following audio was 
received for the timeout specified by the service configuration. All collected 
audio will at this point be considered to be the full voice input to convert to 
text.

.. note::

    Received audio buffers are sorted in the order in which they were received.


Converting Audio
----------------
A fully received voice audio buffer is transcribed to text by using the functionality 
of an external API provider like the Google Cloud API. Audio is given to the used 
provider, which is selected with the service configuration, as a full buffer.

.. note:: 

     Transcription will not start until a full audio buffer is available.
     

The result of the voice audio transcription will then be used to create a listen 
string event to send to the current running user application. 

.. warning::

    String which exceed the event buffer limit will be shortened to fit!


Start / Stop Recording
----------------------
The speech service sends information to the connected audio source to 
inform when to start and stop recording. This information can be used 
to make sure that recording only happens if no speech output is meant 
to be performed.

.. note::

    The local stream messages sent to the external source to start 
    and stop recording are **MRH_LS_M_AUDIO_START_RECORDING** and 
    **MRH_LS_M_AUDIO_STOP_RECORDING**.