CBSpeechMethod
==============
The CBSpeechMethod callback is used return the currently used 
speech method.

Action
------
The callback checks for the current speech method and returns it.

.. note::

    Speech method identifiers returned depend on the service.

Recieved Events
---------------
* MRH_EVENT_LISTEN_GET_METHOD_U
* MRH_EVENT_SAY_GET_METHOD_U

Returned Events
---------------
* MRH_EVENT_LISTEN_GET_METHOD_S
* MRH_EVENT_SAY_GET_METHOD_S

Files
-----
The callback is implemented in the following files:

.. code-block:: c

    Callback/Speech/CBSpeechMethod.cpp
    Callback/Speech/CBSpeechMethod.h