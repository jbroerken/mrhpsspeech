*********
Callbacks
*********
The service uses callbacks to react to recieved events. All callbacks are 
given to libmrhpsb and executed on a callback thread. Service and speech 
callbacks handle a specific event. 

Service Callbacks
-----------------
.. toctree::
  :maxdepth: 1

  Service Availability <Service/CBAvail>
  Custom Command <Service/CBCustomCommand>
  
  
Speech Callbacks
-----------------
.. toctree::
  :maxdepth: 1

  Perform Output <Speech/CBSayString>
  Speech Method <Speech/CBSpeechMethod>
  Send Notitication <Speech/CBNotification>
