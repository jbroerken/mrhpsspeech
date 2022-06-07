/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef APIProvider_h
#define APIProvider_h

// C / C++

// External

// Project


//*************************************************************************************
// API Provider
//*************************************************************************************

// Google Cloud API
#ifndef MRH_API_PROVIDER_GOOGLE_CLOUD_API
    #define MRH_API_PROVIDER_GOOGLE_CLOUD_API 1
#endif

// Enumeration
// @NOTE: Keep #define excluded in list for switch cases
typedef enum
{
    GOOGLE_CLOUD_API = 0,
        
    API_PROVIDER_MAX = GOOGLE_CLOUD_API,
    
    API_PROVIDER_COUNT = API_PROVIDER_MAX + 1

}APIProvider;


#endif /* APIProvider_h */
