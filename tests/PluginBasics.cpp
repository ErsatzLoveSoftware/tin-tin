#include "helpers/test_helpers.h"
#include "processors/PluginProcessor.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE ("one is equal to one", "[dummy]")
{
    REQUIRE(1 == 1);
}

TEST_CASE ("Plugin instance", "[instance]")
{
    PluginProcessor testPlugin;
    auto gui = juce::ScopedJuceInitialiser_GUI{};

    SECTION("The plugin name should be configured correctly.")
    {
        CHECK_THAT(testPlugin.getName().toStdString(),
                   Catch::Matchers::Equals("TinTin"));
    }
}
