***********
Text String
***********
Text speech input is handled by receiving a text string buffer from an external 
source. The full text string is then treated as speech input.

Receiving Text
--------------
Text is received by an external source by using a local stream socket. 
This allows the speech service to receive text both from local input  
or from a remote server.

.. note::

    The local stream message for receiving speech input from the 
    external source is **MRH_LS_M_STRING**.
    

.. note::

    Text strings are required to be in UTF-8 format.


Each received string is considered to be fully complete. This string will then 
be used to create a listen string event to send to the current running user 
application. 

.. warning::

    String which exceed the event buffer limit will be shortened to fit!

