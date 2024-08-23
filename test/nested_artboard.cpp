#include <rive/solo.hpp>
#include <rive/nested_artboard.hpp>
#include <rive/shapes/rectangle.hpp>
#include <rive/shapes/shape.hpp>
#include <rive/animation/state_machine_instance.hpp>
#include <rive/animation/nested_linear_animation.hpp>
#include <rive/animation/nested_state_machine.hpp>
#include "rive_file_reader.hpp"
#include "rive_testing.hpp"
#include <catch.hpp>
#include <cstdio>
#include <iostream>

TEST_CASE("collapsed nested artboards do not advance", "[solo]")
{
    auto file = ReadRiveFile("../../test/assets/solos_with_nested_artboards.riv");

    auto artboard = file->artboard("main-artboard")->instance();
    artboard->advance(0.0f);
    auto solos = artboard->find<rive::Solo>();
    REQUIRE(solos.size() == 1);
    auto stateMachine = artboard->stateMachineAt(0);
    stateMachine->advanceAndApply(0.0f);
    REQUIRE(stateMachine->needsAdvance() == true);
    artboard->advance(0.75f);
    // Testing whether the squares have moved in the artboard is an indirect way
    // if checking whether the time of each artboard has advanced
    // Unfortunately there is no way of accessing the time of the animations directly
    auto redNestedArtboard = stateMachine->artboard()->find<rive::NestedArtboard>("red-artboard");
    auto redNestedArtboardArtboard = redNestedArtboard->artboardInstance();
    auto movingShapes = redNestedArtboardArtboard->find<rive::Shape>();
    auto redRect = movingShapes.at(0);
    REQUIRE(redRect->x() > 50);
    auto greenNestedArtboard =
        stateMachine->artboard()->find<rive::NestedArtboard>("green-artboard");
    auto greenNestedArtboardArtboard = greenNestedArtboard->artboardInstance();
    auto greenMovingShapes = greenNestedArtboardArtboard->find<rive::Shape>();
    auto greenRect = greenMovingShapes.at(0);
    REQUIRE(greenRect->x() == 50);
}

TEST_CASE("nested artboards with looping animations will keep main advanceAndApply advancing",
          "[nested]")
{
    auto file = ReadRiveFile("../../test/assets/ball_test.riv");
    auto artboard = file->artboard("Artboard")->instance();
    artboard->advance(0.0f);
    auto stateMachine = artboard->stateMachineAt(0);
    REQUIRE(stateMachine->advanceAndApply(0.0f) == true);
    REQUIRE(stateMachine->advanceAndApply(1.0f) == true);
    REQUIRE(stateMachine->advanceAndApply(1.0f) == true);
}
TEST_CASE("nested artboards with one shot animations will not main advanceAndApply advancing",
          "[nested]")
{

    auto file = ReadRiveFile("../../test/assets/ball_test.riv");
    auto artboard = file->artboard("Artboard 2")->instance();
    artboard->advance(0.0f);
    auto stateMachine = artboard->stateMachineAt(0);
    REQUIRE(stateMachine->advanceAndApply(0.0f) == true);
    REQUIRE(stateMachine->advanceAndApply(0.9f) == true);
    REQUIRE(stateMachine->advanceAndApply(0.1f) == true);
    // nested artboards animation is 1s long
    REQUIRE(stateMachine->advanceAndApply(0.1f) == false);
}