// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include "App/ComplexGeoData.h"
#include "Base/BoundBox.h"

// NOLINTBEGIN(readability-magic-numbers)

class TestSegmentClass : public Data::Segment
{
public:
    TestSegmentClass(){}
    std::string getName() const override {};
};

class TestCGDClass: public ::Data::ComplexGeoData {
public:
    TestCGDClass() {
        _testSegment = TestSegmentClass();
    };
    unsigned int getMemSize () const { return 0; };
    void Save (Base::Writer &/*writer*/) const { };
    void Restore(Base::XMLReader &/*reader*/) { };
    void SaveDocFile (Base::Writer &/*writer*/) { };
    std::vector<const char*> getElementTypes() const { return std::vector<const char*>(); };
    unsigned long countSubElements(const char* Type) const { return 0; };
    TestSegmentClass* getSubElement(const char* Type, unsigned long) const { };
    TestSegmentClass* getSubElementByName(const char* Name) const { };
    void setTransform(const Base::Matrix4D& rclTrf) {} ;
    Base::Matrix4D getTransform() const { return Base::Matrix4D(); };
    void transformGeometry(const Base::Matrix4D &rclMat) { };
    Base::BoundBox3d getBoundBox() const{ return _bb; };

    TestSegmentClass _testSegment;
    Base::BoundBox3d _bb;
};

class ComplexGeoDataTest: public ::testing::Test
{
protected:
    void SetUp() override {
    }
    // void TearDown() override {}
};

TEST_F(ComplexGeoDataTest, defaultConstruction)
{
    // Act
    TestCGDClass target = TestCGDClass();
    auto sz = target.getMemSize();

    // Assert
    EXPECT_EQ(sz, 0);
}

TEST_F(ComplexGeoDataTest, emptyMappedNameWhenElementNotFound)
{
    // Arrange
    TestCGDClass emptyTest = TestCGDClass();  // TODO: also add a non-empty elementmap, but still not found.
    auto indexedName = Data::IndexedName("face", 1);

    // Act
    auto mappedName = emptyTest.getMappedName(indexedName, false, nullptr);

    // Assert
    EXPECT_EQ(mappedName.toString(), "");
}

// NOLINTEND(readability-magic-numbers)
