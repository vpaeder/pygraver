#include "svg/line.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::svg;
using namespace testing;

TEST(LineTest, Base) {
    auto l1 = Line<double>({2,3}, {5,7});
    auto pa = l1.point(0);
    auto pb = l1.point(0.5);
    auto pc = l1.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, pa[0], 2);
    EXPECT_PRED_FORMAT2(DoubleLE, pa[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, pb[0], 3.5);
    EXPECT_PRED_FORMAT2(DoubleLE, pb[1], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, pc[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, pc[1], 7);

    auto dpa = l1.dpoint(0);
    auto dpb = l1.dpoint(0.5);
    auto dpc = l1.dpoint(1);
    EXPECT_PRED_FORMAT2(DoubleLE, dpa[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, dpa[1], 4);
    EXPECT_PRED_FORMAT2(DoubleLE, dpb[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, dpb[1], 4);
    EXPECT_PRED_FORMAT2(DoubleLE, dpc[0], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, dpc[1], 4);

    EXPECT_PRED_FORMAT2(DoubleLE, l1.length(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.length(0.5), 2.5);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.length(1), 5.0);

    EXPECT_PRED_FORMAT2(DoubleLE, l1.arc(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.arc(0.5), 2.5);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.arc(1), 5.0);

    EXPECT_PRED_FORMAT2(DoubleLE, l1.arg_at_length(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.arg_at_length(2.5), 0.5);
    EXPECT_PRED_FORMAT2(DoubleLE, l1.arg_at_length(5.0), 1.0);
}
