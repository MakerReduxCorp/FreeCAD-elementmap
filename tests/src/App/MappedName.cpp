// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include "App/ComplexGeoData.h"
#include "App/MappedName.h"

#include <string>

// NOLINTBEGIN(readability-magic-numbers)

TEST(MappedName, defaultConstruction)
{
    // Act
    Data::MappedName mappedName;

    // Assert
    EXPECT_EQ(mappedName.empty(), true);
    EXPECT_EQ(mappedName.size(), 0);
    EXPECT_EQ(mappedName.name(), std::string());
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, namedConstruction)
{
    // Act
    Data::MappedName mappedName("TEST");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, namedConstructionWithMaxSize)
{
    // Act
    Data::MappedName mappedName("TEST", 2);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 2);
    EXPECT_EQ(mappedName.name(), std::string("TE"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, namedConstructionDiscardPrefix)
{
    // Arrange
    std::string name = Data::ComplexGeoData::elementMapPrefix() + "TEST";

    // Act
    Data::MappedName mappedName(name.c_str());

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, stringNamedConstruction)
{
    // Act
    Data::MappedName mappedName(std::string("TEST"));

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, stringNamedConstructionDiscardPrefix)
{
    // Arrange
    std::string name = Data::ComplexGeoData::elementMapPrefix() + "TEST";

    // Act
    Data::MappedName mappedName(name);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, constructFromIndexedName)
{
    // Arrange
    Data::IndexedName indexedName {"INDEXED_NAME"};
    Data::IndexedName indexedName1 {"INDEXED_NAME", 1};

    // Act
    Data::MappedName mappedName {indexedName};
    Data::MappedName mappedName1 {indexedName1};

    // Assert
    EXPECT_EQ(mappedName.toString(), indexedName.toString());
    EXPECT_EQ(mappedName1.toString(), indexedName1.toString());
}

TEST(MappedName, copyConstructor)
{
    // Arrange
    Data::MappedName temp("TEST");

    // Act
    Data::MappedName mappedName(temp);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, copyConstructorWithPostfix)
{
    // Arrange
    Data::MappedName temp("TEST");

    // Act
    Data::MappedName mappedName(temp, "POSTFIXTEST");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 15);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));

    // Act
    mappedName = Data::MappedName(mappedName, "ANOTHERPOSTFIX");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 29);
    EXPECT_EQ(mappedName.name(), std::string("TESTPOSTFIXTEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("ANOTHERPOSTFIX"));
}

TEST(MappedName, copyConstructorStartpos)
{
    // Arrange
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    Data::MappedName mappedName = Data::MappedName(temp, 2);
        
    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 13);
    EXPECT_EQ(mappedName.name(), std::string("ST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));
}

TEST(MappedName, copyConstructorStartposAndSize)
{
    // Arrange
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    Data::MappedName mappedName(temp, 2, 6);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 6);
    EXPECT_EQ(mappedName.name(), std::string("ST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POST"));
}

TEST(MappedName, moveConstructor)
{
    // Arrange
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    Data::MappedName mappedName(std::move(temp));

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 15);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));

    EXPECT_EQ(temp.empty(), true);
    EXPECT_EQ(temp.size(), 0);
    EXPECT_EQ(temp.name(), std::string());
    EXPECT_EQ(temp.postfix(), std::string());
}



TEST(MappedName, assignmentOperator)
{
    // Arrange
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    Data::MappedName mappedName = temp;

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 15);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));
}

TEST(MappedName, assignmentOperatorString)
{
    // Arrange
    Data::MappedName mappedName;

    // Act
    mappedName = std::string("TEST");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, assignmentOperatorConstCharPtr)
{
    // Arrange
    Data::MappedName mappedName;

    // Act
    mappedName = "TEST";

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string());
}

TEST(MappedName, assignmentOperatorMove)
{
    // Arrange
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    Data::MappedName mappedName = std::move(temp);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 15);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));

    EXPECT_EQ(temp.empty(), true);
    EXPECT_EQ(temp.size(), 0);
    EXPECT_EQ(temp.name(), std::string());
    EXPECT_EQ(temp.postfix(), std::string());
}

TEST(MappedName, streamInsertionOperator)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");
    std::stringstream ss;

    // Act
    ss << mappedName;

    // Assert
    EXPECT_EQ(ss.str(), std::string("TESTPOSTFIXTEST"));
}

TEST(MappedName, comparisonOperators)
{
    // Arrange
    Data::MappedName mappedName1(Data::MappedName("TEST"), "POSTFIXTEST");
    Data::MappedName mappedName2(Data::MappedName("TEST"), "POSTFIXTEST");
    Data::MappedName mappedName3(Data::MappedName("TESTPOST"), "FIXTEST");
    Data::MappedName mappedName4(Data::MappedName("THIS"), "ISDIFFERENT");

    // Act & Assert
    EXPECT_EQ(mappedName1 == mappedName1, true);
    EXPECT_EQ(mappedName1 == mappedName2, true);
    EXPECT_EQ(mappedName1 == mappedName3, true);
    EXPECT_EQ(mappedName1 == mappedName4, false);

    EXPECT_EQ(mappedName1 != mappedName1, false);
    EXPECT_EQ(mappedName1 != mappedName2, false);
    EXPECT_EQ(mappedName1 != mappedName3, false);
    EXPECT_EQ(mappedName1 != mappedName4, true);
}

TEST(MappedName, additionOperators)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    mappedName += "POST1";
    mappedName += std::string("POST2");
    mappedName += std::string("POST3");
    mappedName += Data::MappedName("POST4");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 35);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTESTPOST1POST2POST3POST4"));

    // Arrange
    mappedName = Data::MappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    mappedName = mappedName + Data::MappedName("POST5");
    mappedName = mappedName + "POST6";
    mappedName = mappedName + std::string("POST7");
    mappedName = mappedName + std::string("POST8");
        
    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 35);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTESTPOST5POST6POST7POST8"));
}

TEST(MappedName, append)
{
    // Arrange
    Data::MappedName mappedName;

    // Act
    mappedName.append("TEST");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 4);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string(""));

    // Act
    mappedName.append("POSTFIX");

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 11);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIX"));

    // Act
    mappedName.append("ANOTHERPOSTFIX", 5);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 16);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXANOTH"));
}

TEST(MappedName, appendMappedNameObj)
{
    // Arrange
    Data::MappedName mappedName;
    Data::MappedName temp(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    mappedName.append(temp);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 15);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTEST"));

    // Act
    mappedName.append(temp, 2, 7);

    // Assert
    EXPECT_EQ(mappedName.empty(), false);
    EXPECT_EQ(mappedName.size(), 22);
    EXPECT_EQ(mappedName.name(), std::string("TEST"));
    EXPECT_EQ(mappedName.postfix(), std::string("POSTFIXTESTSTPOSTF"));
}

TEST(MappedName, toString)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act & Assert
    EXPECT_EQ(mappedName.toString(), std::string("TESTPOSTFIXTEST"));
}

TEST(MappedName, toIndexedNameASCIIOnly)
{
    // Arrange
    Data::MappedName mappedName {"MAPPED_NAME"};

    // Act
    auto indexedName = mappedName.toIndexedName();

    // Assert
    EXPECT_FALSE(indexedName.isNull());
}

TEST(MappedName, toIndexedNameInvalid)
{
    // Arrange
    Data::MappedName mappedName {"MAPPED-NAME"};

    // Act
    auto indexedName = mappedName.toIndexedName();

    // Assert
    EXPECT_TRUE(indexedName.isNull());
}

TEST(MappedName, compare)
{
    // Arrange
    Data::MappedName mappedName1(Data::MappedName("TEST"), "POSTFIXTEST");
    Data::MappedName mappedName2(Data::MappedName("TEST"), "POSTFIXTEST");
    Data::MappedName mappedName3(Data::MappedName("TESTPOST"), "FIXTEST");
    Data::MappedName mappedName4(Data::MappedName("THIS"), "ISDIFFERENT");
    Data::MappedName mappedName5(Data::MappedName("SH"), "ORTHER");
    Data::MappedName mappedName6(Data::MappedName("VERYVERYVERY"), "VERYMUCHLONGER");

    // Act & Assert
    EXPECT_EQ(mappedName1.compare(mappedName1), 0);
    EXPECT_EQ(mappedName1.compare(mappedName2), 0);
    EXPECT_EQ(mappedName1.compare(mappedName3), 0);
    EXPECT_EQ(mappedName1.compare(mappedName4), -1);
    EXPECT_EQ(mappedName1.compare(mappedName5), 1);
    EXPECT_EQ(mappedName1.compare(mappedName6), -1);

    EXPECT_EQ(mappedName1 < mappedName1, false);
    EXPECT_EQ(mappedName1 < mappedName2, false);
    EXPECT_EQ(mappedName1 < mappedName3, false);
    EXPECT_EQ(mappedName1 < mappedName4, true);
    EXPECT_EQ(mappedName1 < mappedName5, false);
    EXPECT_EQ(mappedName1 < mappedName6, true);
}

TEST(MappedName, subscriptOperator)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act & Assert
    EXPECT_EQ(mappedName[0], 'T');
    EXPECT_EQ(mappedName[1], 'E');
    EXPECT_EQ(mappedName[2], 'S');
    EXPECT_EQ(mappedName[3], 'T');
    EXPECT_EQ(mappedName[4], 'P');
    EXPECT_EQ(mappedName[5], 'O');
    EXPECT_EQ(mappedName[6], 'S');
    EXPECT_EQ(mappedName[7], 'T');
    EXPECT_EQ(mappedName[8], 'F');
    EXPECT_EQ(mappedName[9], 'I');
}




TEST(MappedName, boolOperator)
{
    // Arrange
    Data::MappedName mappedName;

    // Act & Assert
    EXPECT_EQ((bool)mappedName, false);

    // Arrange
    mappedName.append("TEST");

    // Act & Assert
    EXPECT_EQ((bool)mappedName, true);
}

TEST(MappedName, clear)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act
    mappedName.clear();

    // Assert
    EXPECT_EQ(mappedName.empty(), true);
}

TEST(MappedName, find)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act & Assert
    EXPECT_EQ(mappedName.find(""), 0);
    EXPECT_EQ(mappedName.find(std::string("")), 0);
    EXPECT_EQ(mappedName.find("TEST"), 0);
    EXPECT_EQ(mappedName.find("STPO"), 2); //DROPPED: sentence must be fully contained in data or postfix
    EXPECT_EQ(mappedName.find("POST"), 4);
    EXPECT_EQ(mappedName.find("POST", 4), 4);
    EXPECT_EQ(mappedName.find("POST", 5), -1);

    EXPECT_EQ(mappedName.rfind("ST"), 13);
    EXPECT_EQ(mappedName.rfind("ST", 15), 13);
    EXPECT_EQ(mappedName.rfind("ST", 14), 13);
    EXPECT_EQ(mappedName.rfind("ST", 13), 13);
    EXPECT_EQ(mappedName.rfind("ST", 12), 6);
    EXPECT_EQ(mappedName.rfind("ST", 11), 6);
    EXPECT_EQ(mappedName.rfind("ST", 10), 6);
    EXPECT_EQ(mappedName.rfind("ST", 9), 6);
    EXPECT_EQ(mappedName.rfind("ST", 8), 6);
    EXPECT_EQ(mappedName.rfind("ST", 7), 6);
    EXPECT_EQ(mappedName.rfind("ST", 6), 6);
    EXPECT_EQ(mappedName.rfind("ST", 5), 2);
    EXPECT_EQ(mappedName.rfind("ST", 4), 2);
    EXPECT_EQ(mappedName.rfind("ST", 3), 2);
    EXPECT_EQ(mappedName.rfind("ST", 2), 2);
    EXPECT_EQ(mappedName.rfind("ST", 1), -1);
    EXPECT_EQ(mappedName.rfind("ST", 0), -1);
}

TEST(MappedName, rfind)
{
    // Arrange
    Data::MappedName mappedName(Data::MappedName("TEST"), "POSTFIXTEST");

    // Act & Assert
    EXPECT_EQ(mappedName.rfind(""), mappedName.size());
    EXPECT_EQ(mappedName.rfind("TEST"), 11);
    EXPECT_EQ(mappedName.rfind("STPO"), 2); //DROPPED: sentence must be fully contained in data or postfix
    EXPECT_EQ(mappedName.rfind("POST"), 4);
    EXPECT_EQ(mappedName.rfind("ST"), 13); 
    EXPECT_EQ(mappedName.rfind("POST", 4), 4);
    EXPECT_EQ(mappedName.rfind("POST", 3), -1);
    
    EXPECT_EQ(mappedName.rfind(std::string("")), mappedName.size());
    EXPECT_EQ(mappedName.rfind("TEST"), 11);
    EXPECT_EQ(mappedName.rfind("STPO"), 2); //DROPPED: sentence must be fully contained in data or postfix
    EXPECT_EQ(mappedName.rfind("POST"), 4);
    EXPECT_EQ(mappedName.rfind("POST", 4), 4);
    EXPECT_EQ(mappedName.rfind("POST", 3), -1);

    EXPECT_EQ(mappedName.rfind("ST"), 13);
    EXPECT_EQ(mappedName.rfind("ST", 0), -1);
    EXPECT_EQ(mappedName.rfind("ST", 1), -1);
    EXPECT_EQ(mappedName.rfind("ST", 2), 2);
    EXPECT_EQ(mappedName.rfind("ST", 3), 2);
    EXPECT_EQ(mappedName.rfind("ST", 4), 2);
    EXPECT_EQ(mappedName.rfind("ST", 5), 2);
    EXPECT_EQ(mappedName.rfind("ST", 6), 6);
    EXPECT_EQ(mappedName.rfind("ST", 7), 6);
    EXPECT_EQ(mappedName.rfind("ST", 8), 6);
    EXPECT_EQ(mappedName.rfind("ST", 9), 6);
    EXPECT_EQ(mappedName.rfind("ST", 10), 6);
    EXPECT_EQ(mappedName.rfind("ST", 11), 6);
    EXPECT_EQ(mappedName.rfind("ST", 12), 6);
    EXPECT_EQ(mappedName.rfind("ST", 13), 13);
    EXPECT_EQ(mappedName.rfind("ST", 14), 13);
    EXPECT_EQ(mappedName.rfind("ST", 15), 13);
}

TEST(MappedName, endswith)
{
    // Arrange
    Data::MappedName mappedName("TEST");

    // Act & Assert
    EXPECT_EQ(mappedName.endsWith("TEST"), true);
    EXPECT_EQ(mappedName.endsWith(std::string("TEST")), true);
    EXPECT_EQ(mappedName.endsWith("WASD"), false);

    // Arrange
    mappedName.append("POSTFIX");

    // Act & Assert
    EXPECT_EQ(mappedName.endsWith("TEST"), false);
    EXPECT_EQ(mappedName.endsWith("FIX"), true);
}

TEST(MappedName, startsWith)
{
    // Arrange
    Data::MappedName mappedName("TEST");

    // Act & Assert
    EXPECT_EQ(mappedName.startsWith(std::string()), true);
    EXPECT_EQ(mappedName.startsWith(std::string("TEST")), true);
    EXPECT_EQ(mappedName.startsWith("TEST"), true);
    EXPECT_EQ(mappedName.startsWith("WASD"), false); 
}

//TODO test reference 

// NOLINTEND(readability-magic-numbers)
