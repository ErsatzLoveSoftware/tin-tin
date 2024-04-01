cmake_minimum_required(VERSION 3.27.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(PROJECT_NAME "tintin0.1")

include(PamplejuceVersion)
include(PamplejuceMacOS)
include(JUCEDefaults)

set(PRODUCT_NAME "TinTin")
set(COMPANY_NAME "wammy")
set(BUNDLE_ID "com.wammy.tintin") # Used for the MacOS bundle identifier and signing.
set(FORMATS Standalone AU VST3 AUv3)

# For simplicity, the name of the CMake project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})


# JUCE is setup as a submodule in the /JUCE folder
# Locally, you must run `git submodule update --init --recursive` once
# and later `git submodule update --remote --merge` to keep it up to date
# On Github Actions, this is done as a part of actions/checkout
add_subdirectory(JUCE)

# Add any other modules you want modules here, before the juce_add_plugin call
# juce_add_module(modules/my_module)

# This adds the melatonin inspector module
add_subdirectory (modules/melatonin_inspector)

# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
        VERSION "0.1"                               # Set this if the plugin version is different to the project version
        ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"
        # ICON_SMALL ...
        COMPANY_NAME "${COMPANY_NAME}"              # Specify the name of the plugin's author
        BUNDLE_ID "${BUNDLE_ID}"
        IS_SYNTH FALSE                              # Is this a synth or an effect?
        NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT TRUE                      # Does the plugin need midi output?
        IS_MIDI_EFFECT TRUE                         # Is this plugin a MIDI effect?
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor_consts need keyboard focus?
        # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE Wamm               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE P001                            # A unique four-character plugin id with exactly one upper-case character
        FORMATS "${FORMATS}"                        # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME "${PRODUCT_NAME}"              # The name of the final executable, which can differ from the target name
)

# This lets us use our code in both the JUCE targets and our Test target
# Without running into ODR violations
#add_library(SharedCode INTERFACE
#        source/TinTinProcessor.cpp
#        source/NoteDisplays.h
#        source/TinTInNoteDisplayComponent.h
#        source/FIFO.h
#        source/ui/TinTinComponents.h)

add_library(SharedCode INTERFACE
        source/ui/TinTinOctaveComponent.h)

# C++20, please
# Use cxx_std_23 for C++23 (as of CMake v 3.20)
target_compile_features(SharedCode INTERFACE cxx_std_20)

# Manually list all .h and .cpp files for the plugin
# If you are like me, you'll use globs for your sanity.
# Just ensure you employ CONFIGURE_DEPENDS so the build system picks up changes
# If you want to appease the CMake gods and avoid globs, manually add files like so:
# set(SourceFiles Source/TinTinEditor.h Source/PluginProcessor.h Source/TinTinEditor.cpp Source/PluginProcessor.cpp)
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})

# Adds a BinaryData target for embedding assets into the binary
include(Assets)

# MacOS only: Cleans up folder and target organization on Xcode.
include(XcodePrettify)

# This is where you can set preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
    INTERFACE

    # JUCE_WEB_BROWSER and JUCE_USE_CURL off by default
    JUCE_WEB_BROWSER=0  # If you set this to 1, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you set this to 1, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0

    # Uncomment if you are paying for a an Indie/Pro license or releasing under GPLv3
    # JUCE_DISPLAY_SPLASH_SCREEN=0

    # lets the app known if we're Debug or Release
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"

    # JucePlugin_Name is for some reason doesn't use the nicer PRODUCT_NAME
    PRODUCT_NAME_WITHOUT_VERSION="TinTin"
)

# Link to any other modules you added (with juce_add_module) here!
# Usually JUCE modules must have PRIVATE visibility
# See https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md#juce_add_module
# However, with Pamplejuce, you'll link modules to SharedCode with INTERFACE visibility
# This allows the JUCE plugin targets and the Tests target to link against it
target_link_libraries(
    SharedCode
    INTERFACE
    Assets
    melatonin_inspector
    juce_audio_utils
    juce_audio_processors
    juce_dsp
    juce_gui_basics
    juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# IPP support, comment out to disable
#include(PamplejuceIPP)

# Everything related to the tests target
include(Tests)

# A separate target keeps the Tests target fast!
include(Benchmarks)

# Pass some config to GA (like our PRODUCT_NAME)
#include(GitHubENV)
