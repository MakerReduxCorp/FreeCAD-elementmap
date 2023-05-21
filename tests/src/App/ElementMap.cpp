// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include <App/Application.h>
#include "App/ElementMap.h"

// NOLINTBEGIN(readability-magic-numbers)

class LessComplexPart
{
    // this is a "holder" class used for simpler testing of ElementMap in the context
    // of a class
public:
    mutable long Tag;
    Data::ElementMapPtr elementMapPtr;
};

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

TEST_F(ElementMapTest, mimicOnePart)
{
    // Arrange
    //   pattern: new doc, create Cube
    //   for a single part, there is no "naming algo" to speak of
    std::ostringstream ss;
    auto docName = "Unnamed";
    auto cube = LessComplexPart();
    auto cubeName = Data::MappedName("Box");
    cube.elementMapPtr = std::make_shared<Data::ElementMap>();
    cube.Tag = 1;
    auto finalMappedName = Data::MappedName();
    auto face1 = Data::IndexedName("Face", 1);
    auto face2 = Data::IndexedName("Face", 2);
    auto face3 = Data::IndexedName("Face", 3);
    auto face4 = Data::IndexedName("Face", 4);
    auto face5 = Data::IndexedName("Face", 5);
    auto face6 = Data::IndexedName("Face", 6);
    auto face6NameHolder = Data::MappedName(face6);

    // Act
    cube.elementMapPtr->setElementName(face1, Data::MappedName(face1), 0); //  build a box with faces
    cube.elementMapPtr->setElementName(face2, Data::MappedName(face2), 0);
    cube.elementMapPtr->setElementName(face3, Data::MappedName(face3), 0);
    cube.elementMapPtr->setElementName(face4, Data::MappedName(face4), 0);
    cube.elementMapPtr->setElementName(face5, Data::MappedName(face5), 0);
    cube.elementMapPtr->encodeElementName(  // we will invoke the encoder anyway for face 6; should make no difference
            face6.getType()[0],
            face6NameHolder,
            ss,
            nullptr,
            cube.Tag,
            nullptr,
            cube.Tag);  // naming against itself
    cube.elementMapPtr->setElementName(face6, face6NameHolder, 0);
    auto children = cube.elementMapPtr->getAll();
    ss << docName << "#" << cubeName << "." << cube.elementMapPtr->find(face6);

    // Assert
    EXPECT_EQ(children.size(), 6);
    EXPECT_EQ(children[0].index.toString(), "Face1");
    EXPECT_EQ(children[0].name.toString(), "Face1");
    EXPECT_EQ(children[1].index.toString(), "Face2");
    EXPECT_EQ(children[1].name.toString(), "Face2");
    EXPECT_EQ(children[2].index.toString(), "Face3");
    EXPECT_EQ(children[2].name.toString(), "Face3");
    EXPECT_EQ(children[3].index.toString(), "Face4");
    EXPECT_EQ(children[3].name.toString(), "Face4");
    EXPECT_EQ(children[4].index.toString(), "Face5");
    EXPECT_EQ(children[4].name.toString(), "Face5");
    EXPECT_EQ(children[5].index.toString(), "Face6");
    EXPECT_EQ(children[5].name.toString(), "Face6");
    auto serializedResult = ss.str();
    EXPECT_EQ(serializedResult, "Unnamed#Box.Face6");
}

TEST_F(ElementMapTest, mimicSimpleUnion)
{
    // Arrange
    //   pattern: new doc, create Cube, create Cylinder, Union of both (Cube first)
    const char* expectedNameOfTopFaceOfCubeSide = "Unnamed#Fusion.;Face6;:M2;FUS;:H1:8,F.Face3";
    const char* expectedUnionOpPostfix = ":M2;FUS";
    const char* expectedFace3Name = "Face6;:M2;FUS;:H1:8,F";
    std::ostringstream ss;
    std::ostringstream finalSs;
    char* docName = "Unnamed";
    // arrange "create cube"
    auto cube = LessComplexPart();
    auto cubeName = Data::MappedName("Box");
    cube.elementMapPtr = std::make_shared<Data::ElementMap>();
    cube.Tag = 1;
    auto cface1 = Data::IndexedName("Face", 1);
    auto cface2 = Data::IndexedName("Face", 2);
    auto cface3 = Data::IndexedName("Face", 3);
    auto cface4 = Data::IndexedName("Face", 4);
    auto cface5 = Data::IndexedName("Face", 5);
    auto cface6 = Data::IndexedName("Face", 6);
    cube.elementMapPtr->setElementName(cface1, Data::MappedName(cface1), 0);//  build a box with faces
    cube.elementMapPtr->setElementName(cface2, Data::MappedName(cface2), 0);
    cube.elementMapPtr->setElementName(cface3, Data::MappedName(cface3), 0);
    cube.elementMapPtr->setElementName(cface4, Data::MappedName(cface4), 0);
    cube.elementMapPtr->setElementName(cface5, Data::MappedName(cface5), 0);
    cube.elementMapPtr->setElementName(cface6, Data::MappedName(cface6), 0);
    // arrange "create cylinder"
    auto cylinder = LessComplexPart();
    auto cylinderName = Data::MappedName("Cylinder");
    cylinder.elementMapPtr = std::make_shared<Data::ElementMap>();
    cylinder.Tag = 2; // used for the ;:Hn:n part
    auto yface1 = Data::IndexedName("Face", 1);
    auto yface2 = Data::IndexedName("Face", 2);
    auto yface3 = Data::IndexedName("Face", 3);
    cylinder.elementMapPtr->setElementName(yface1, Data::MappedName(yface1), 0);//  build a cylinder with faces
    cylinder.elementMapPtr->setElementName(yface2, Data::MappedName(yface2), 0);
    cylinder.elementMapPtr->setElementName(yface3, Data::MappedName(yface3), 0);
    // arrange Union (Fusion) operation via the Part Workbench
    auto unionPart = LessComplexPart();
    auto unionName = Data::MappedName("Fusion"); // fusion
    unionPart.elementMapPtr = std::make_shared<Data::ElementMap>();
    unionPart.Tag = 3;
    auto uface3 = Data::IndexedName("Face", 3); // we are only going to simulate one face for testing purpose
    auto uface3type = uface3.getType()[0];
    auto PartOp = "FUS"; // Part::OpCodes::Fuse;

    // Act
    //   act: simulate a union/fuse operation
    auto parent = cube.elementMapPtr->getAll()[5];
    auto postfixHolder = Data::MappedName(Data::POSTFIX_MOD + "2");
    unionPart.elementMapPtr->encodeElementName(
        "M"[0],
        postfixHolder,
        ss,
        nullptr,
        0,
        nullptr,
        0);
    auto postfixStr = postfixHolder.toString() + Data::ELEMENT_MAP_PREFIX + PartOp;
    //   act: with the fuse op, name against the cube's Face6
    auto uface3Holder = Data::MappedName(parent.index);
    auto uface3Str = uface3Holder.toString();
    unionPart.elementMapPtr->encodeElementName(// we will invoke the encoder for face 3
        uface3type,
        uface3Holder,
        ss,
        nullptr,
        unionPart.Tag,
        postfixStr.c_str(),
        cube.Tag);
    uface3Str = uface3Holder.toString();
    unionPart.elementMapPtr->setElementName(uface3, uface3Holder, unionPart.Tag);
    // act: generate a full toponame string for testing  purposes
    finalSs << docName << "#" << unionName;
    finalSs << ".";
    finalSs << Data::ELEMENT_MAP_PREFIX + uface3Str;
    finalSs << ".";
    finalSs << uface3;

    // Assert
    EXPECT_EQ(postfixStr, std::string(expectedUnionOpPostfix));
    EXPECT_EQ(uface3Str, expectedFace3Name);
    auto serializedResult = finalSs.str();
    EXPECT_EQ(serializedResult, expectedNameOfTopFaceOfCubeSide);

    // explanation of "Fusion.;Face6;:M2;FUS;:H2:3,F.Face3"  toponame
    //    Note: every postfix is prefixed by semicolon
    //    Node: the start/middle/end are separated by periods
    //
    // "Fusion" means that we are on the "Fusion" object.
    // "." we are done with the first part
    // ";Face6" means default inheritance comes from face 6 of the parent (which is a cube)
    // ";:M2" means that a Workbench op has happened. "M" is the "Mod" directory in the source tree?
    // ";FUS" means that a Fusion operation has happened. Notice the lack of a colon.
    // ";:H2" means the subtending object (cylinder) has a tag of 2
    // ":3" means the writing position is 3; literally how far into the current postfix string we are
    // ",F" means are of type "F" which is short for "Face" of Face3 of Fusion.
    // "." we are done with the second part
    // "Face3" is the localized name
}

// NOLINTEND(readability-magic-numbers)
