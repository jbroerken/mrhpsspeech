******************************
Google Cloud API Configuration
******************************
The Google Cloud API uses the Google Cloud to convert speech to text and text 
to speech. Supported are settings for both the language to use and the gender 
for the speech output.

Google Cloud API Block
----------------------
The Google Cloud API block stores the following values:

.. list-table::
    :header-rows: 1

    * - Key
      - Description
    * - LanguageCode
      - The language code to use for transcribing text and audio.
        See https://cloud.google.com/speech-to-text/docs/languages 
        for the BCP-47 codes.
    * - VoiceGender
      - The gender of the speaking voice to use for the google text 
        to speech synthesizer. 0 for female, >= 1 for male.
        
Example
-------
The following example shows default Google Cloud API settings found in the 
speech service configuration file:

.. code-block:: c

    <Google Cloud API>{
        <LanguageCode><en>
        <VoiceGender><0>
    }
    
