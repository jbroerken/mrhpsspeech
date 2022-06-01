**************
Speech Methods
**************
mrhpsspeech uses two types of speech methods: Voice 
audio and text strings.

Both speech methods rely on external sources for both speech input 
and output. A speech method is usable if a external source connects 
to the local socket used for the methods data transmission. 

The method chosen by the service depends on the the types of sources 
connected. Voice will always be chosen as the default if no text 
source is connected. Once a text source is connected it will be used 
as the method for speech input and output. Disconnecting the text 
source will cause the service to revert to using the voice source 
(if available).

.. note::

    Only one speech method can be used at a time.
