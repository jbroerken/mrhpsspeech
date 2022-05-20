************
Service Info
************
mrhpsspeech is the default speech input and output service used 
by the MRH platform. This service only implements default listen 
and say events.

Service Identifiers
-------------------
The service uses the following identifiers:

.. list-table::
    :header-rows: 1

    * - Identifier
      - Value
    * - Supplier ID
      - MRH (0x4d524800)
    * - Binary ID
      - TALK (0x54414c4b)
    * - Version
      - 1


Features
--------
The following features are provided by the service:
  
.. list-table::
    :header-rows: 1

    * - Feature
      - Description
    * - Handle audio speech input
      - The service processes audio buffer speech 
        input to send as events to user applications.
    * - Handle audio speech output
      - The service creates audio buffer speech output 
        from received user application events.
    * - Handle text speech input
      - The service processes text string speech 
        input to send as events to user applications.
    * - Handle text speech output
      - The service creates text string speech output 
        from received user application events.
    * - List speech sources
      - The service gives information about the used 
        output sources for speech input and output.

  
Events
------
The following events are handled by the service:

* MRH_EVENT_PS_RESET_REQUEST_U
* MRH_EVENT_LISTEN_AVAIL_U
* MRH_EVENT_LISTEN_CUSTOM_COMMAND_U
* MRH_EVENT_LISTEN_GET_METHOD_U
* MRH_EVENT_SAY_AVAIL_U
* MRH_EVENT_SAY_CUSTOM_COMMAND_U
* MRH_EVENT_SAY_STRING_U
* MRH_EVENT_SAY_GET_METHOD_U
* MRH_EVENT_SAY_NOTIFICATION_APP_U
* MRH_EVENT_SAY_NOTIFICATION_SERVICE_U

.. note::
    
    Custom command events will return the event MRH_EVENT_NOT_IMPLEMENTED_S!
    
.. note:: 

    MRH_EVENT_LISTEN_STRING_S is sent without being requested.

