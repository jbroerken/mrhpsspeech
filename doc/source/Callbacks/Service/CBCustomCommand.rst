CBCustomCommand
===============
The CBCustomCommand callback is used to react to recieved custom 
command events. Only application custom events are handled.

Action
------
The callback will create a not implemented response event, which 
will include the custom command event type as its value. The event 
is then added to the events to send to the user package.

Recieved Events
---------------
* MRH_EVENT_LISTEN_CUSTOM_COMMAND_U
* MRH_EVENT_SAY_CUSTOM_COMMAND_U

Returned Events
---------------
* MRH_EVENT_NOT_IMPLEMENTED_S

Files
-----
The callback is implemented in the following files:

.. code-block:: c

    Callback/Service/CBCustomCommand.cpp
    Callback/Service/CBCustomCommand.h