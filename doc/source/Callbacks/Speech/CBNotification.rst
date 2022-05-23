CBNotification
==============
The CBNotification callback is used to send a notification to 
a remote user device.

Action
------
The callback is receives a notification string from either a user 
application or user application service and sends it to another 
user device with the help of an external process.

The callback will return a result event to the user application 
which sent the request. A application service will not receive a 
result event.

.. note::

    The default service has no functionality implemented. It will 
    return a not implemented result.

Recieved Events
---------------
* MRH_EVENT_SAY_NOTIFICATION_APP_U
* MRH_EVENT_SAY_NOTIFICATION_SERVICE_U

Returned Events
---------------
* MRH_EVENT_SAY_NOTIFICATION_APP_S

Files
-----
The callback is implemented in the following files:

.. code-block:: c

    Callback/Speech/CBNotification.cpp
    Callback/Speech/CBNotification.h