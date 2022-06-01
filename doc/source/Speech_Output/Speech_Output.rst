*************
Speech Output
*************
mrhpsspeech sends speech output for a user by receiving a output string event from the 
currently running user application. 

The service uses to types of sources for speech output: Voice based output and text string 
based output. Voice based output handles audio buffers while text based output handles simple 
UTF-8 strings.

.. note:: 

    Both voice and text based output send their audio and text strings to  
    external sources.


.. toctree::
   :maxdepth: 1

   Source/Voice
   Source/Text_String
