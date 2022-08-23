#include "svg/path.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::svg;
using namespace testing;

// SVG test strings adapted from https://www.w3.org/TR/SVG11/paths.html
// I also try different formats that are supported by the SVG standard
// (i.e. with/without spaces, with commas, scientific notation for numbers,...)

TEST(SVGPathTest, ParseTriangle01) {
    std::string triangle01 = "M 100 100 L 300 100 L 200 300 z";
    std::string triangle01_compact = "M100 100L300 100L200 300z";
    auto c1 = Path<double>(triangle01);
    auto & segs1 = c1.get_segments();
    EXPECT_EQ(segs1.size(), 3);
    auto c2 = Path<double>(triangle01_compact);
    auto & segs2 = c1.get_segments();
    EXPECT_EQ(segs2.size(), 3);
    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(1);
    auto pt1c = segs1[1]->point(0);
    auto pt1d = segs1[1]->point(1);
    auto pt1e = segs1[2]->point(0);
    auto pt1f = segs1[2]->point(1);
    EXPECT_EQ(pt1a[0], 100);
    EXPECT_EQ(pt1a[1], 100);
    EXPECT_EQ(pt1b[0], 300);
    EXPECT_EQ(pt1b[1], 100);
    EXPECT_EQ(pt1c[0], 300);
    EXPECT_EQ(pt1c[1], 100);
    EXPECT_EQ(pt1d[0], 200);
    EXPECT_EQ(pt1d[1], 300);
    EXPECT_EQ(pt1e[0], 200);
    EXPECT_EQ(pt1e[1], 300);
    EXPECT_EQ(pt1f[0], 100);
    EXPECT_EQ(pt1f[1], 100);
}
TEST(SVGPathTest, ParseCubic01) {
    std::string cubic01 = "M100.0,200.0 C1e2,100 250,100 250,200 S400,300 400,200";
    auto c1 = Path<double>(cubic01);
    auto & segs1 = c1.get_segments();
    EXPECT_EQ(segs1.size(), 2);
    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(1);
    auto pt1c = segs1[1]->point(0);
    auto pt1d = segs1[1]->point(1);
    EXPECT_EQ(pt1a[0], 100);
    EXPECT_EQ(pt1a[1], 200);
    EXPECT_EQ(pt1b[0], 250);
    EXPECT_EQ(pt1b[1], 200);
    EXPECT_EQ(pt1c[0], 250);
    EXPECT_EQ(pt1c[1], 200);
    EXPECT_EQ(pt1d[0], 400);
    EXPECT_EQ(pt1d[1], 200);
}

TEST(SVGPathTest, ParseQuad01) {
    std::string quad01 = "M200,300 Q400,50 600,300 T1000,300";
    auto c1 = Path<double>(quad01);
    auto & segs1 = c1.get_segments();
    EXPECT_EQ(segs1.size(), 2);
    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(1);
    auto pt1c = segs1[1]->point(0);
    auto pt1d = segs1[1]->point(1);
    EXPECT_EQ(pt1a[0], 200);
    EXPECT_EQ(pt1a[1], 300);
    EXPECT_EQ(pt1b[0], 600);
    EXPECT_EQ(pt1b[1], 300);
    EXPECT_EQ(pt1c[0], 600);
    EXPECT_EQ(pt1c[1], 300);
    EXPECT_EQ(pt1d[0], 1000);
    EXPECT_EQ(pt1d[1], 300);
}

TEST(SVGPathTest, ParseArc01) {
    std::string arc01 = "M300,200 h-150 a150,150 0 1,0 150,-150 z";
    auto c1 = Path<double>(arc01);
    auto & segs1 = c1.get_segments();
    EXPECT_EQ(segs1.size(), 3);
    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(1);
    auto pt1c = segs1[1]->point(0);
    auto pt1d = segs1[1]->point(1);
    auto pt1e = segs1[2]->point(0);
    auto pt1f = segs1[2]->point(1);
    EXPECT_EQ(pt1a[0], 300);
    EXPECT_EQ(pt1a[1], 200);
    EXPECT_EQ(pt1b[0], 150);
    EXPECT_EQ(pt1b[1], 200);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[0], 150);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[1], 200);
    EXPECT_EQ(pt1d[0], 300);
    EXPECT_EQ(pt1d[1], 50);
    EXPECT_EQ(pt1e[0], 300);
    EXPECT_EQ(pt1e[1], 50);
    EXPECT_EQ(pt1f[0], 300);
    EXPECT_EQ(pt1f[1], 200);
}
