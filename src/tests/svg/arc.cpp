#include "svg/arc.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::svg;
using namespace testing;

#define M_PI 3.14159265358979323846264338327950288

TEST(CarlsonTest, CarlsonRF) {
    EXPECT_PRED_FORMAT2(DoubleLE, carlson_rf<double>(0, 1, 2, 1e-16), 1.3110287771460599052324198);
}

TEST(CarlsonTest, CarlsonRD) {
    EXPECT_PRED_FORMAT2(DoubleLE, carlson_rd<double>(0, 2, 1, 1e-16), 1.7972103521033883111598837);
}

TEST(ArcTest, Circular) {
    // round arc, half a turn
    auto arc1 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{2, 2}, 0, M_PI, 0);
    auto p1a = arc1.point(0);
    auto p1b = arc1.point(0.5);
    auto p1c = arc1.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p1a[0], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p1a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p1b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p1b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p1c[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p1c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.length(1), 2*M_PI);
    // same arc, alternate construction
    auto arc2 = Arc<double>(
        std::vector<double>{7,5},
        std::vector<double>{3,5},
        std::vector<double>{2,2},
        0, false, true);
    auto p2a = arc2.point(0);
    auto p2b = arc2.point(0.5);
    auto p2c = arc2.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p2a[0], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p2a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p2b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p2b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p2c[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p2c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc2.length(1), 2*M_PI);
    // half arc in reverse direction
    auto arc3 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{2, 2}, M_PI, 0, 0);
    auto p3a = arc3.point(0);
    auto p3b = arc3.point(0.5);
    auto p3c = arc3.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p3a[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p3a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p3b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p3b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p3c[0], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p3c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc3.length(1), 2*M_PI);
    // same arc, alternate construction
    auto arc4 = Arc<double>(
        std::vector<double>{3,5},
        std::vector<double>{7,5},
        std::vector<double>{2,2},
        0, false, false);
    auto p4a = arc4.point(0);
    auto p4b = arc4.point(0.5);
    auto p4c = arc4.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p4a[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p4a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p4b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p4b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p4c[0], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p4c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc4.length(1), 2*M_PI);
    // arc with sweeping direction reversed
    auto arc5 = Arc<double>(
        std::vector<double>{3,5},
        std::vector<double>{7,5},
        std::vector<double>{2,2},
        0, false, true);
    auto p5a = arc5.point(0);
    auto p5b = arc5.point(0.5);
    auto p5c = arc5.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p5a[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p5a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p5b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p5b[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p5c[0], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p5c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc5.length(1), 2*M_PI);
    // 3/4 arc
    auto arc6 = Arc<double>(
        std::vector<double>{3,5},
        std::vector<double>{5,3},
        std::vector<double>{2,2},
        0, true, false);
    auto p6a = arc6.point(0);
    auto p6b = arc6.point(1.0/3.0);
    auto p6c = arc6.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p6a[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p6a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p6b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p6b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p6c[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p6c[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, arc6.length(1), 3*M_PI);
    // 1/4 arc
    auto arc7 = Arc<double>(
        std::vector<double>{3,5},
        std::vector<double>{5,3},
        std::vector<double>{2,2},
        0, false, false);
    auto p7a = arc7.point(0);
    auto p7b = arc7.point(0.5);
    auto p7c = arc7.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p7a[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p7a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p7b[0], p7b[1]);
    EXPECT_PRED_FORMAT2(DoubleLE, p7c[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p7c[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, arc7.length(1), M_PI);
    // rotated by a 1/4 turn
    auto arc8 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{2, 2}, 0, M_PI, M_PI/2.0);
    auto p8a = arc8.point(0);
    auto p8b = arc8.point(0.5);
    auto p8c = arc8.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p8a[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p8a[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p8b[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p8b[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p8c[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p8c[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, arc8.length(1), 2*M_PI);
    // same arc, alternate construction
    auto arc9 = Arc<double>(
        std::vector<double>{7,5},
        std::vector<double>{3,5},
        std::vector<double>{2,2},
        M_PI/2.0, false, true);
    auto p9a = arc9.point(0);
    auto p9b = arc9.point(0.5);
    auto p9c = arc9.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p9a[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p9a[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p9b[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p9b[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p9c[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p9c[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, arc9.length(1), 2*M_PI);
}


TEST(ArcTest, Elliptic) {
    // elliptic arc, half a turn
    auto arc1 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{3, 2}, 0, M_PI, 0);
    auto p1a = arc1.point(0);
    auto p1b = arc1.point(0.5);
    auto p1c = arc1.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p1a[0], 8);
    EXPECT_PRED_FORMAT2(DoubleLE, p1a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p1b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p1b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p1c[0], 2);
    EXPECT_PRED_FORMAT2(DoubleLE, p1c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.length(1), 7.932719794645293);
    // same arc, alternate construction
    auto arc2 = Arc<double>(
        std::vector<double>{8,5},
        std::vector<double>{2,5},
        std::vector<double>{3,2},
        0, false, true);
    auto p2a = arc2.point(0);
    auto p2b = arc2.point(0.5);
    auto p2c = arc2.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p2a[0], 8);
    EXPECT_PRED_FORMAT2(DoubleLE, p2a[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p2b[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p2b[1], 7);
    EXPECT_PRED_FORMAT2(DoubleLE, p2c[0], 2);
    EXPECT_PRED_FORMAT2(DoubleLE, p2c[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc2.length(1), 7.932719794645293);
    // rotated by a 1/4 turn
    auto arc3 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{3, 2}, 0, M_PI, M_PI/2.0);
    auto p3a = arc3.point(0);
    auto p3b = arc3.point(0.5);
    auto p3c = arc3.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, p3a[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p3a[1], 8);
    EXPECT_PRED_FORMAT2(DoubleLE, p3b[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, p3b[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p3c[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, p3c[1], 2);
    EXPECT_PRED_FORMAT2(DoubleLE, arc3.length(1), 7.932719794645293);
}

TEST(ArcTest, Functions) {
    // round arc, half a turn
    auto arc1 = Arc<double>(std::vector<double>{5,5}, std::vector<double>{2, 2}, 0, M_PI, 0);
    auto dp1a = arc1.dpoint(0);
    auto dp1b = arc1.dpoint(0.5);
    EXPECT_TRUE(abs(dp1a[0])<1e-14);
    EXPECT_PRED_FORMAT2(DoubleLE, dp1a[1], 12.566370614359172);
    EXPECT_PRED_FORMAT2(DoubleLE, dp1b[0], -12.566370614359172);
    EXPECT_TRUE(abs(dp1b[1])<1e-14);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arg_at_length(2*M_PI), 1);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arg_at_length(M_PI), 0.5);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arg_at_length(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arc(0), 12.566370614359172);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arc(0.5), 12.566370614359172);
    EXPECT_PRED_FORMAT2(DoubleLE, arc1.arc(1.0), 12.566370614359172);
}
