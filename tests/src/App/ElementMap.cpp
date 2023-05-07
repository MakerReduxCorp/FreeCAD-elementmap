// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include <App/Application.h>
#include "App/ElementMap.h"

// NOLINTBEGIN(readability-magic-numbers)

class ElementMapTest: public ::testing::Test
{
protected:
    void SetUp() override {
        int argc = 1;
        char *argv[] = { "FreeCAD" };
        App::Application::Config()["ExeName"] = "FreeCAD";
        App::Application::init(argc, argv);
        App::GetApplication().newDocument("test", "testUser");
    }
    // void TearDown() override {}
};

TEST_F(ElementMapTest, defaultConstruction)
{
    // Act
    Data::ElementMap elementMap = Data::ElementMap();

    // Assert
    EXPECT_EQ(elementMap.size(), 0);
}

TEST_F(ElementMapTest, setElementNameDefaults)
{
    // Arrange
    Data::MappedName mappedName("TEST");
    Data::IndexedName element("Edge", 1);
    Data::ElementIDRefs _sid;
    auto sids = &_sid;
    Data::MappedName expectedName("TEST");
    Data::ElementMap elementMap;

    // Act
    auto resultName = elementMap.setElementName(element, mappedName, 0);

    // Assert
    EXPECT_EQ(resultName, expectedName);
    auto mappedToElement = elementMap.find(element);
    EXPECT_EQ(mappedToElement, mappedName);
}

TEST_F(ElementMapTest, encodeElementName)
{
    // Arrange
    Data::ElementMap elementMap;
    Data::MappedName nameHolder("START");
    Data::IndexedName childOne("Edge", 1);
    Data::MappedName childOneName("TEST");
    std::ostringstream ss;
    int newTag = 2;
    int masterTag = 1;
    auto n = elementMap.setElementName(childOne, childOneName, 0);
    ss.str("[");
    const char* postfix = "ZZZ"; // should be auto-prefixed by encodeElementName

    // Act
    elementMap.encodeElementName(
        childOne.getType()[0],
        nameHolder,
        ss,
        nullptr,
        masterTag,
        postfix,
        newTag);

    // Assert
    auto result = ss.str();
    EXPECT_EQ(result, ";ZZZ;:H2:4,E");
}

// NOLINTEND(readability-magic-numbers)
