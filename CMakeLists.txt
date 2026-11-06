cmake_minimum_required(VERSION 3.22)
project(AZVerb VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")   # Intel + Apple Silicon
endif()
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # VC++ runtime talab qilinmaydi
endif()

# JUCE: ./JUCE papkasida bo'lsa uni ishlatadi, aks holda GitHub'dan yuklaydi
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# Formatlar: Windows -> VST3 + AAX, macOS -> VST3 + AU + AAX, Linux -> VST3 + Standalone
if(NOT AZ_FORMATS)
    if(APPLE)
        set(AZ_FORMATS VST3 AU AAX Standalone)
    elseif(WIN32)
        set(AZ_FORMATS VST3 AAX Standalone)
    else()
        set(AZ_FORMATS VST3 Standalone)
    endif()
endif()

juce_add_plugin(AZVerb
    PRODUCT_NAME "AZ VERB"
    COMPANY_NAME "Azimxodjayev Music"
    COMPANY_WEBSITE "https://github.com/alifrecords"
    BUNDLE_ID "com.azimxodjayevmusic.azverb"
    PLUGIN_MANUFACTURER_CODE Azmx
    PLUGIN_CODE AzVb
    FORMATS ${AZ_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Reverb
    AU_MAIN_TYPE kAudioUnitType_Effect
    AAX_CATEGORY AAX_ePlugInCategory_Reverb
    DESCRIPTION "AZ VERB - Smart Vocal Reverb")

juce_add_binary_data(AZVerbData SOURCES
    Resources/InterRegular.otf Resources/InterMedium.otf Resources/InterSemiBold.otf Resources/InterBold.otf)

target_sources(AZVerb PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(AZVerb PRIVATE Source)

target_compile_definitions(AZVerb PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(AZVerb
    PRIVATE AZVerbData juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# --- Testlar (faqat -DAZ_TESTS=ON bilan) ---
if(AZ_TESTS)
    juce_add_console_app(AZSmartTest PRODUCT_NAME "AZSmartTest")
    target_sources(AZSmartTest PRIVATE tests/smart/SmartTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(AZSmartTest PRIVATE Source)
    target_compile_definitions(AZSmartTest PRIVATE JucePlugin_VersionString="1.0.0" JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(AZSmartTest PRIVATE AZVerbData juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
