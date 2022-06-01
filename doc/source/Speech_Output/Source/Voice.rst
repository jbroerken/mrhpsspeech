*****
Voice
*****
Voice speech output is handled by sending an audio buffer to an external 
source, which can then be used to perform speech output meant for the user. 
The service will also wait for a response from the external source which 
shows that speech output was completed.

Creating Audio
--------------
Received text strings are converted to audio by using the functionality of an 
external API provider like the Google Cloud API. The output string is given to 
the used provider, which is selected with the service configuration, and 
converted to a audio buffer for playback.

.. note:: 

    Strings are handled in the order in which they were received.

Sending Audio
-------------
Created audio for speech output is sent fully to the external source responsible 
for performing audio playback by using a local stream socket. 

.. note::

    The local stream message used to send voice audio to an 
    external source is **MRH_LS_M_AUDIO**.
    

.. note::

    The audio format of the created audio depends on the API provider and 
    service settings. 


Output Performed Response
-------------------------
The service expects a output performed response once the output was fully 
played back. This response shows the service that playback can now continue 
with new audio to output.

.. note::

    The local stream message expected from the external source to signal 
    playback finished is **MRH_LS_M_AUDIO_PLAYBACK_FINISHED**.
    

.. warning::

    No audio will be created or sent until the service receives the 
    output performed message for the current output.
