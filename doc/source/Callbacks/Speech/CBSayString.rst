CBSayString
===========
The CBSayString callback is used retreive speech output to perform.

Action
------
The callback will read the output string received and store it for 
the current speech method to perform.

.. note::

    The output might be performed on a different speech method 
    if the speech method changes before output.


.. note::

    The output will only be performed after all speech output 
    before it has been performed.

Recieved Events
---------------
* MRH_EVENT_SAY_STRING_U

Returned Events
---------------
None.

Files
-----
The callback is implemented in the following files:

.. code-block:: c

    Callback/Speech/CBSayString.cpp
    Callback/Speech/CBSayString.h