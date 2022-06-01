***********
Text String
***********
Text speech output is handled by sending a text string buffer to an external 
source. The full text string is sent as speech output.

Sending Text
------------
Text is sent to an external source by using a local stream socket. 
This allows the speech service to send text to both local input  
or a remote server.

.. note::

    The local stream message for sending speech output to the 
    external source is **MRH_LS_M_STRING**.
    

.. note::

    Text strings are sent in the UTF-8 format.


Each sent string is considered to be fully complete. Each text string received 
from a user application by a say string event will be sent immediatly.

