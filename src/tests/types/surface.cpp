#include "types/common.h"
#include "types/point.h"
#include "types/path.h"
#include "types/surface.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::types;

class SurfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto path = std::make_shared<Path>(0);
        path->emplace_back(std::make_shared<Point>(-1, -1, 0, 0));
        path->emplace_back(std::make_shared<Point>(-1, 1, 0, 0));
        path->emplace_back(std::make_shared<Point>(1, 1, 0, 0));
        path->emplace_back(std::make_shared<Point>(1, -1, 0, 0));
        path->emplace_back(std::make_shared<Point>(-1, -1, 0, 0));
        this->surface = std::make_shared<Surface>(path);
    }

    std::shared_ptr<Surface> surface;
};

TEST_F(SurfaceTest, Construction) {
    auto surf1 = Surface();
    auto bnd = std::make_shared<Path>(0);
    auto bnds = std::vector<std::shared_ptr<Path>>{bnd};
    auto surf2 = Surface(bnd);
    auto surf3 = Surface(bnds);
    auto surf4 = Surface(bnd, bnds);
    auto surf5 = Surface(bnds, bnds);
}

TEST_F(SurfaceTest, MillingPaths) {
    auto paths = this->surface->get_milling_paths(0.5, 0.3);
    EXPECT_EQ(paths.size(), 4);
    EXPECT_EQ(*(*paths[0])[0], Point(0, 0, 0, 0));
    EXPECT_EQ(*(*paths[1])[0], Point(-0.15, -0.15, 0, 0));
    EXPECT_EQ(*(*paths[2])[0], Point(-0.45, -0.45, 0, 0));
    EXPECT_EQ(*(*paths[3])[0], Point(-0.75, -0.75, 0, 0));
    auto paths2 = this->surface->get_milling_paths(2.0, 0.3);
    EXPECT_EQ(paths2.size(), 0);
}

TEST_F(SurfaceTest, MilledSurface) {
    auto s2 = this->surface->get_milled_surface(0.5, 0.3);
    EXPECT_EQ(s2.size(), 1);
    EXPECT_EQ(s2[0]->get_contours()[0]->get_largest_radius(), sqrt(2)*0.75+0.25);
}

TEST_F(SurfaceTest, Centroid) {
    auto centroid = this->surface->get_centroid();
    EXPECT_EQ(*centroid, Point(0, 0, 0, 0));
}

TEST_F(SurfaceTest, Contains) {
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(0,0,0,0)));
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(0.5,0.5,0,0)));
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(-0.5,0.5,0,0)));
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(-0.5,-0.5,0,0)));
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(0.5,-0.5,0,0)));
    // check only in 2D
    EXPECT_TRUE(this->surface->contains(std::make_shared<Point>(0.5,-0.5,10.0,0)));
    // points on the boundary aren't considered inside
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(1,1,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(-1,1,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(-1,-1,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(1,-1,0,0)));
    // points outside
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(2,2,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(-2,2,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(-2,-2,0,0)));
    EXPECT_FALSE(this->surface->contains(std::make_shared<Point>(2,-2,0,0)));
}

TEST_F(SurfaceTest, Combine) {
    // one single boundary => one surface
    auto s1 = this->surface->combine();
    EXPECT_EQ(s1.size(), 1);
    // two overlapping boundaries => one surface
    auto p1 = this->surface->get_contours()[0];
    auto s2 = Surface({p1,p1->shift(std::make_shared<Point>(0.5,0.5,0,0))}).combine();
    EXPECT_EQ(s2.size(), 1);
    // two disconnected boundaries => two surfaces
    auto s3 = Surface({p1,p1->shift(std::make_shared<Point>(3,3,0,0))}).combine();
    EXPECT_EQ(s3.size(), 2);
    // works in 2D
    auto s4 = Surface({p1,p1->shift(std::make_shared<Point>(0.5,0.5,10.0,0))}).combine();
    EXPECT_EQ(s4.size(), 1);
    // with hole
    auto sh1 = Surface({p1, p1->shift(std::make_shared<Point>(0.5,0.5,0,0))}, {p1->buffer(-0.3)});
    auto s5 = sh1.combine();
    EXPECT_EQ(s5.size(), 1);
    EXPECT_EQ(s5[0]->get_holes().size(), 1);
    // with hole and distinct boundaries
    auto sh2 = Surface({p1, p1->shift(std::make_shared<Point>(2.2,2.2,0,0))},
                       {p1->shift(std::make_shared<Point>(1.1,1.1,0,0))});
    auto s6 = sh2.combine();
    EXPECT_EQ(s6.size(), 2);
    EXPECT_EQ(s6[0]->get_holes().size(), 0);
    EXPECT_EQ(s6[1]->get_holes().size(), 0);
    // with overlapping holes
    auto ph = p1->buffer(-0.3);
    auto sh3 = Surface(p1, {ph, ph->shift(std::make_shared<Point>(0.2,0.2,0,0))});
    auto s7 = sh3.combine();
    EXPECT_EQ(s7.size(), 1);
    EXPECT_EQ(s7[0]->get_contours().size(), 1);
    EXPECT_EQ(s7[0]->get_holes().size(), 1);
}

TEST_F(SurfaceTest, BooleanOperation) {
    auto p2 = std::make_shared<Path>(0);
    p2->emplace_back(std::make_shared<Point>(-2, -0.5, 0, 0));
    p2->emplace_back(std::make_shared<Point>(2, -0.5, 0, 0));
    p2->emplace_back(std::make_shared<Point>(2, 0.5, 0, 0));
    p2->emplace_back(std::make_shared<Point>(-2, 0.5, 0, 0));
    p2->emplace_back(std::make_shared<Point>(-2, -0.5, 0, 0));
    auto s2 = std::make_shared<Surface>(p2);
    // union
    auto op_add = this->surface->boolean_operation(s2, BooleanOperation::Union);
    EXPECT_EQ(op_add.size(), 1);
    // difference
    auto op_diff = this->surface->boolean_operation(s2, BooleanOperation::Difference);
    EXPECT_EQ(op_diff.size(), 2);
    // symmetric difference
    auto op_symdiff = this->surface->boolean_operation(s2, BooleanOperation::SymmetricDifference);
    EXPECT_EQ(op_symdiff.size(), 4);
    // intersection
    auto op_mul = this->surface->boolean_operation(s2, BooleanOperation::Intersection);
    EXPECT_EQ(op_mul.size(), 1);
    // I skip testing complex cases as they rely on GEOS anyway;
    // what is important is that the code extracts correctly what GEOS produces
}

TEST_F(SurfaceTest, CorrectHeight) {
    auto path = std::make_shared<Path>(0);
    for (int n=-15; n<16; n++)
        path->emplace_back(std::make_shared<Point>(double(n)/10.0, 0, 0, 0));

    // correct outside    
    auto cpo = this->surface->correct_height({path}, 0, 2.0, true, false);
    EXPECT_EQ((*cpo[0])[0]->z, 2.0);
    EXPECT_EQ((*cpo[0])[4]->z, 2.0);
    EXPECT_EQ((*cpo[0])[5]->z, 0.0);
    EXPECT_EQ((*cpo[0])[25]->z, 0.0);
    EXPECT_EQ((*cpo[0])[26]->z, 2.0);
    // correct inside
    auto cpi = this->surface->correct_height({path}, 0, 2.0, false, false);
    EXPECT_EQ((*cpi[0])[0]->z, 0.0);
    EXPECT_EQ((*cpi[0])[4]->z, 0.0);
    EXPECT_EQ((*cpi[0])[5]->z, 2.0);
    EXPECT_EQ((*cpi[0])[25]->z, 2.0);
    EXPECT_EQ((*cpi[0])[26]->z, 0.0);
    // with added points around boundaries
    auto p2 = std::make_shared<Path>(0);
    p2->emplace_back(std::make_shared<Point>(-10.0, 0, 0, 0));
    p2->emplace_back(std::make_shared<Point>(10.0, 0, 0, 0));
    auto cpa = this->surface->correct_height({p2}, 0, 2.0, true, true);
    EXPECT_EQ(cpa[0]->size(), 8);
    EXPECT_EQ(*(*cpa[0])[1], Point(-1.001, 0, 2, 0));
    // points exactly on a boundary may be considered inside or outside
    // because of finite numerical precision; therefore, we can't expect
    // a given value for z
    EXPECT_EQ((*cpa[0])[2]->x, -1);
    EXPECT_EQ(*(*cpa[0])[3], Point(-0.999, 0, 0, 0));
    EXPECT_EQ(*(*cpa[0])[4], Point(0.999, 0, 0, 0));
    EXPECT_EQ((*cpa[0])[5]->x, 1);
    EXPECT_EQ(*(*cpa[0])[6], Point(1.001, 0, 2, 0));
    // correct with positive clearance
    auto cpcp = this->surface->correct_height({p2}, 0.1, 2.0, true, true);
    EXPECT_TRUE(almost_equal((*cpcp[0])[2]->x, -1.1, 6));
    EXPECT_TRUE(almost_equal((*cpcp[0])[5]->x, 1.1, 6));
    // correct with negative clearance
    auto cpcn = this->surface->correct_height({p2}, -0.1, 2.0, true, true);
    EXPECT_TRUE(almost_equal((*cpcn[0])[2]->x, -0.9, 6));
    EXPECT_TRUE(almost_equal((*cpcn[0])[5]->x, 0.9, 6));
}
