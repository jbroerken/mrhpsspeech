************
Speech Input
************
mrhpsspeech receives speech input from a user to send it as a input string event to the 
currently running user application. 

The service uses to types of sources for speech input: Voice based input and text string 
based input. Voice based input handles audio buffers while text based input handles simple 
UTF-8 strings.

.. note:: 

    Both voice and text based input receive their audio and text strings from 
    external sources.


.. toctree::
   :maxdepth: 1

   Source/Voice
   Source/Text_String
