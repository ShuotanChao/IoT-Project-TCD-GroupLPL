# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/borjagq/esp/esp-idf/components/bootloader/subproject"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/tmp"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/src"
  "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/borjagq/Library/CloudStorage/OneDrive-TrinityCollegeDublin/InternetOfThings/Source/IoT-Project-TCD-GroupLPL/human_crowdedness/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
