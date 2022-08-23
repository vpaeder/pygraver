#include "svg/bezier3.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::svg;
using namespace testing;

TEST(Bezier3Test, Base) {
    auto bez1 = Bezier3<double>({1,3}, {5,2}, {8,7}, {5,5});
    auto pa = bez1.point(0);
    auto pb = bez1.point(0.5);
    auto pc = bez1.point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, pa[0], 1);
    EXPECT_PRED_FORMAT2(DoubleLE, pa[1], 3);
    EXPECT_PRED_FORMAT2(DoubleLE, pb[0], 5.625);
    EXPECT_PRED_FORMAT2(DoubleLE, pb[1], 4.375);
    EXPECT_PRED_FORMAT2(DoubleLE, pc[0], 5);
    EXPECT_PRED_FORMAT2(DoubleLE, pc[1], 5);

    auto dpa = bez1.dpoint(0);
    auto dpb = bez1.dpoint(0.5);
    auto dpc = bez1.dpoint(1);
    EXPECT_PRED_FORMAT2(DoubleLE, dpa[0], 12);
    EXPECT_PRED_FORMAT2(DoubleLE, dpa[1], -3);
    EXPECT_PRED_FORMAT2(DoubleLE, dpb[0], 5.25);
    EXPECT_PRED_FORMAT2(DoubleLE, dpb[1], 5.25);
    EXPECT_PRED_FORMAT2(DoubleLE, dpc[0], -9);
    EXPECT_PRED_FORMAT2(DoubleLE, dpc[1], -6);

    EXPECT_PRED_FORMAT2(DoubleLE, bez1.length(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.length(0.5), 5.039869834673979);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.length(1), 7.601833524762528);

    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arc(0), 12.36931687685298);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arc(0.5), 7.424621202458749);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arc(1), 10.816653826391969);

    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arg_at_length(0), 0);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arg_at_length(5.039869834673979), 0.5);
    EXPECT_PRED_FORMAT2(DoubleLE, bez1.arg_at_length(7.601833524762528), 1.0);
}
