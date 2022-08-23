#include "types/point.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::types;


TEST(PointTest, AngleComputation) {
  auto p = Point(1,1,1,1);
  EXPECT_EQ(p.angle(false), 46.0);
  EXPECT_EQ(p.angle(true), 0.8028514559173916);
  EXPECT_EQ(p.elevation(false), 35.264389682754654);
  EXPECT_EQ(p.elevation(true), 0.6154797086703873);
  p.x = 0;
  p.y = 1;
  p.z = 0;
  p.c = 0;
  EXPECT_EQ(p.angle(false), 90.0);
  EXPECT_EQ(p.elevation(false), 0.0);
  p.y = 0;
  EXPECT_EQ(p.angle(false), 0.0);
  p.x = 1;
  EXPECT_EQ(p.angle(false), 0.0);
  p.z = 1;
  EXPECT_EQ(p.elevation(false), 45.0);
  p.x = 0;
  EXPECT_EQ(p.elevation(false), 90.0);
}

TEST(PointTest, RadiusComputation) {
  double expected_radius = 1.7320508075688772;
  auto p = Point(1,1,1,1);
  EXPECT_EQ(p.radius(), expected_radius);
  p.c = 100;
  EXPECT_EQ(p.radius(), expected_radius);
  p.x = 0;
  p.y = 0;
  p.z = 0;
  EXPECT_EQ(p.radius(), 0.0);
  p.x = 1;
  EXPECT_EQ(p.radius(), 1.0);
  p.x = 0;
  p.y = 1;
  EXPECT_EQ(p.radius(), 1.0);
  p.y = 0;
  p.z = 1;
  EXPECT_EQ(p.radius(), 1.0);
}

TEST(PointTest, DistanceComputation) {
  auto p1 = std::make_shared<Point>(0,0,0,0);
  auto p2 = std::make_shared<Point>(1,1,1,1);
  double expected_value = 1.7320508075688772;
  EXPECT_EQ(p1->distance_to(p2), expected_value);
  EXPECT_EQ(p2->distance_to(p1), expected_value);
  EXPECT_EQ(p1->distance_to(p1), 0.0);
  EXPECT_EQ(p2->distance_to(p2), 0.0);
}

TEST(PointTest, CoordinateSystems) {
  auto p = Point(1,1,1,1);
  auto cartesian = p.to_cartesian();
  auto polar = p.to_polar();
  auto cylindrical = p.to_cylindrical(25);
  EXPECT_EQ(cartesian->x, 0.9823952887191078);
  EXPECT_EQ(cartesian->y, 1.0173001015936747);
  EXPECT_EQ(cartesian->z, 1.0);
  EXPECT_EQ(cartesian->c, 0.0);
  EXPECT_EQ(polar->x, 1.4142135623730951);
  EXPECT_EQ(polar->y, 0.0);
  EXPECT_EQ(polar->z, 1.0);
  EXPECT_EQ(polar->c, 46.0);
  EXPECT_EQ(cylindrical->x, 1.0);
  EXPECT_EQ(cylindrical->y, 25.996192378909782);
  EXPECT_EQ(cylindrical->z, 1.4363101609320879);
  EXPECT_EQ(cylindrical->c, 0.0);
}

TEST(PointTest, Operations) {
  auto p1 = Point(1,1,1,1);
  auto p2 = Point(2,2,2,2);
  EXPECT_FALSE(p1==p2);
  EXPECT_TRUE(p1!=p2);
  EXPECT_TRUE(p1+p1==p2);
  EXPECT_TRUE(p2-p1==p1);
  EXPECT_TRUE(p1-p2==-p1);
  EXPECT_TRUE(2*p1==p2);
  EXPECT_TRUE(p1*2==p2);
}
