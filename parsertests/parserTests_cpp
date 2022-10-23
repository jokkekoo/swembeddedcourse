#include <gtest/gtest.h>
#include "../parser.h"

char gpsstr[] = "$GPGGA,134731.361,5540.3252,N,01231.2946,E";

TEST(parserTests, NormalTest) {
	EXPECT_EQ(parser(gpsstr), 6);
}

TEST(parserTests, WrongIndentifierInputCheck) {
	char wronggps[] = "$KAKA,134731.361,5540.3252,N,01231.2946,E";
	EXPECT_EQ(parser(wronggps), 1);
}

TEST(parserTests, WrongTimestampInputCheck) {
	char wrongtimestamp[] = "$GPGGA,-134731.361,5540.3252,N,01231.2946,E";
	EXPECT_EQ(parser(wrongtimestamp), 2); 
}

TEST(parserTests, WrongLatitudeCheck) {
	char wronglatitudelow[] = "$GPGGA,134731.361,-5540.3252,N,01231.2946,E";
	char wronglatitudehigh[] = "$GPGGA,134731.361,15540.3252,N,01231.2946,E";
	EXPECT_EQ(parser(wronglatitudehigh), 3);
	EXPECT_EQ(parser(wronglatitudelow), 3);
}

TEST(parserTests, WrongLongitudeCheck){
	char wronglongitudelow[] = "$GPGGA,134731.361,5540.3252,N,-01231.2946,E";
	char wronglongitudehigh[] = "$GPGGA,134731.361,5540.3252,N,121231.2946,E";
	EXPECT_EQ(parser(wronglongitudelow), 5);
	EXPECT_EQ(parser(wronglongitudehigh), 5);
}

TEST(parserTests, WrongHemisphere) {
	// Always north first and east last
	char wrongnorth[] = "$GPGGA,134731.361,5540.3252,S,01231.2946,E";
	char wrongeast[] = "$GPGGA,134731.361,5540.3252,N,01231.2946,W";
	EXPECT_EQ(parser(wrongnorth), 4);
	EXPECT_EQ(parser(wrongeast), 6);
}



