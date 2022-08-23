#include "types/point.h"
#include "types/path.h"
#include "types/common.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::types;

class PathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // we need to provide the size argument here, otherwise the constructor
        // with py::array_t arguments is called, and since no Python interpreter
        // was initialized it crashes
        this->path = std::make_shared<Path>(0);
        path->emplace_back(std::make_shared<Point>(0,0,0,0));
        path->emplace_back(std::make_shared<Point>(1,0,1,0));
    }

    std::shared_ptr<Path> path;
};

TEST_F(PathTest, Construction) {
    // 1st constructor: Path(size_t n)
    size_t sz = 25;
    auto p1 = Path(sz);
    EXPECT_EQ(p1.size(), sz);

    // 2nd constructor: Path(std::shared<Point> pt)
    auto pt = std::make_shared<Point>(1,1,1,1);
    auto p2 = Path(pt);
    EXPECT_EQ(p2.size(), 1);

    // 3rd constructor: Path(vector<vector<double>>)
    std::vector<std::vector<double>> dbl_array;
    dbl_array.resize(10);
    for (size_t n=0; n<10; n++) {
        dbl_array[n].resize(4);
        for (size_t i=0; i<4; i++)
            dbl_array[n][i] = n*i;
    }
    auto p3 = Path(dbl_array);
    EXPECT_EQ(p3.size(), 10);

    // note that Path(py::array_t<double> x 4) can only be tested with
    // Python interpreter initialized
}

TEST_F(PathTest, Subscripting) {
    size_t sz = 25;
    auto p1 = Path(sz);
    auto pt = std::make_shared<Point>(1,1,1,1);
    p1.push_back(pt);
    EXPECT_EQ(pt, p1[sz]);
    EXPECT_THROW(p1[sz+1], std::out_of_range);
}

TEST_F(PathTest, Arithmetics) {
    auto p2 = this->path + this->path;
    EXPECT_EQ(p2->size(), 2*this->path->size());
    auto p3 = this->path + std::make_shared<Point>(1,1,1,1);
    EXPECT_EQ(p3->size(), this->path->size()+1);
    auto p4 = this->path*3;
    EXPECT_EQ(p4->size(), 3*this->path->size());
    p4 = 3*this->path;
    auto p5 = -this->path;
    for (size_t n=0; n<this->path->size(); n++)
        EXPECT_EQ(*(*this->path)[n], *(-(*p5)[n]));
}

TEST_F(PathTest, Centroid) {
    auto centroid = this->path->get_centroid();
    EXPECT_EQ(centroid->x, 0.5);
    EXPECT_EQ(centroid->y, 0.0);
    EXPECT_EQ(centroid->z, 0.5);
    EXPECT_EQ(centroid->c, 0.0);
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    centroid = this->path->get_centroid();
    EXPECT_EQ(centroid->x, 0.3333333333333333);
    EXPECT_EQ(centroid->y, 0.3333333333333333);
    EXPECT_EQ(centroid->z, 0.6666666666666666);
    EXPECT_EQ(centroid->c, 0.0);
}

TEST_F(PathTest, Radii) {
    EXPECT_EQ(this->path->get_largest_radius(), 0.7071067811865476);
    auto radii = this->path->get_radii();
    EXPECT_EQ(radii.size(), 2);
    EXPECT_EQ(radii[0], 0.7071067811865476);
    EXPECT_EQ(radii[1], 0.7071067811865476);
}

TEST_F(PathTest, PathLogic) {
    // can't compute winding for 2-point curve
    EXPECT_FALSE(this->path->is_ccw());
    EXPECT_FALSE(this->path->is_closed());
    // close curve
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    EXPECT_TRUE(this->path->is_closed());
    EXPECT_TRUE(this->path->is_ccw());
    // create path with same points in reverse order
    auto rev_path = std::make_shared<Path>(4);
    std::reverse_copy(this->path->begin(), this->path->end(), rev_path->begin());
    EXPECT_TRUE(rev_path->is_closed());
    EXPECT_FALSE(rev_path->is_ccw());
}

TEST_F(PathTest, GeometricOperations) {
    // shift with null point => same path
    auto path1 = this->path->shift(std::make_shared<Point>());
    EXPECT_EQ(*(*this->path)[0], *(*path1)[0]);
    // shift with non-null point
    auto pt1 = std::make_shared<Point>(1,1,1,1);
    auto path2 = this->path->shift(pt1);
    EXPECT_EQ(*(*this->path)[0], *(*path2)[0] - *pt1);
    // scale with origin as centre
    auto path3 = this->path->scale(2.5, std::make_shared<Point>());
    EXPECT_EQ(*(*this->path)[0]*2.5, *(*path3)[0]);
    // scale with another point as centre
    auto pt2 = (*this->path)[1];
    auto path4 = this->path->scale(2.5, pt2);
    EXPECT_EQ(-*(*this->path)[1]*2.5, *(*path4)[0]);
    // mirror
    auto path5 = this->path->mirror(true, true, true);
    EXPECT_EQ(-*(*this->path)[1], *(*path5)[1]);
    // rotations
    auto path6 = this->path->rotate(90, 0, 0, false);
    EXPECT_EQ((*this->path)[1]->x, (*path6)[1]->y);
    auto path7 = this->path->rotate(0, 90, 0, false);
    EXPECT_TRUE(almost_equal((*this->path)[1]->z, -(*path7)[1]->z, 10));
    auto path8 = this->path->rotate(0, 0, 90, false);
    EXPECT_EQ((*this->path)[1]->z, -(*path8)[1]->y);
}

TEST_F(PathTest, MatrixTransform) {
    // unity matrix
    auto path1 = this->path->matrix_transform({1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1});
    EXPECT_EQ(*(*this->path)[0], *(*path1)[0]);
    // shift by x=1, y=1, z=1
    auto path2 = this->path->matrix_transform({1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1});
    EXPECT_EQ(*(*this->path)[0], *(*path2)[0] - Point(1,1,1,0));
    // rotate by 90 degrees
    auto path3 = this->path->matrix_transform({0,-1,0,0,1,0,0,0,0,0,1,0,0,0,0,1});
    EXPECT_EQ((*this->path)[1]->x, (*path3)[1]->y);
    // scale
    auto path4 = this->path->matrix_transform({2,0,0,0,0,3,0,0,0,0,1,0,0,0,0,1});
    EXPECT_EQ((*this->path)[0]->x, (*path4)[0]->x*2);
    EXPECT_EQ((*this->path)[0]->y, (*path4)[0]->y*3);
}

TEST_F(PathTest, Inflate) {
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto p2 = this->path->inflate(1.0);
    double v0 = 0.7415816237971963;
    EXPECT_EQ(*(*p2)[0], Point(-v0, -v0, -2*v0, 0));
    EXPECT_EQ(*(*p2)[1], Point(2*v0, -v0, v0, 0));
    EXPECT_EQ(*(*p2)[2], Point(-v0, 2*v0, v0, 0));
    EXPECT_EQ(*(*p2)[3], *(*p2)[0]);
}

TEST_F(PathTest, Buffer) {
    // this is based on a function from libgeos; I'm not sure that the number
    // of points would be the same on every system; not sure that point 1 is
    // what I test it against.
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto p2 = this->path->buffer(1.0);
    EXPECT_EQ(*(*p2)[0], Point(-1.0, 0, 0, 0));
    EXPECT_EQ(*(*p2)[1], Point(-1.0, 1.0, 0, 0));
}

TEST_F(PathTest, ClosePath) {
    auto p2 = this->path->close();
    EXPECT_EQ(p2->size(), this->path->size()+1);
    EXPECT_EQ(*(*p2)[0], *(*p2)[this->path->size()]);
    EXPECT_FALSE(this->path->is_closed());
    EXPECT_TRUE(p2->is_closed());
}

TEST_F(PathTest, ConvexHull) {
    // this is based on a function from libgeos; just testing with a simple shape
    // that we get what we expect; I assume that more complicated cases
    // are tested elsewhere.
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    // this point should be stripped off when calculating convex hull
    this->path->emplace_back(std::make_shared<Point>(0,0.5,1,0));
    auto p2 = this->path->convex_hull();
    EXPECT_EQ(p2.size(), 1);
    EXPECT_FALSE(this->path->is_closed());
    // convex hull function returns a closed shape
    EXPECT_TRUE(p2[0]->is_closed());
    EXPECT_EQ(p2[0]->size(), this->path->size());
}

TEST_F(PathTest, Simplify) {
    // libgeos function again
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0.1,0.6,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    // 4th point should be removed here...
    auto p2 = this->path->simplify(1.0);
    // ...but nore here
    auto p3 = this->path->simplify(1e-2);
    EXPECT_EQ(p2->size(), this->path->size()-1);
    EXPECT_EQ(p3->size(), this->path->size());
}

TEST_F(PathTest, Interpolate) {
    // libgeos does length indexation in 2D only => 3rd coordinate is interpolated
    // but not taken into account to compute length
    auto p2 = this->path->interpolate(0.1);
    auto p3 = this->path->interpolate(10.0);
    EXPECT_EQ(p2->size(), 11);
    EXPECT_EQ(*(*p2)[1], Point(0.1, 0, 0.1, 0));
    EXPECT_EQ(p3->size(), 2);
}

TEST_F(PathTest, Divergence) {
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto div_dxdx = this->path->divergence(DivComponent::DxDx);
    auto div_dydx = this->path->divergence(DivComponent::DyDx);
    auto div_dzdx = this->path->divergence(DivComponent::DzDx);
    auto div_dxdy = this->path->divergence(DivComponent::DxDy);
    auto div_dydy = this->path->divergence(DivComponent::DyDy);
    auto div_dzdy = this->path->divergence(DivComponent::DzDy);
    auto div_dxdz = this->path->divergence(DivComponent::DxDz);
    auto div_dydz = this->path->divergence(DivComponent::DyDz);
    auto div_dzdz = this->path->divergence(DivComponent::DzDz);
    auto inf = std::numeric_limits<double>::infinity();
    EXPECT_EQ(div_dxdx, std::vector<double>({1,1,1,1}));
    EXPECT_EQ(div_dydx, std::vector<double>({0,-inf, -inf, -inf}));
    EXPECT_EQ(div_dzdx, std::vector<double>({1,-inf, -inf, -inf}));
    EXPECT_EQ(div_dxdy, std::vector<double>({inf, inf, inf, 0}));
    EXPECT_EQ(div_dydy, std::vector<double>({1,1,1,1}));
    EXPECT_EQ(div_dzdy, std::vector<double>({inf, inf, inf, 1}));
    EXPECT_EQ(div_dxdz, std::vector<double>({1, -inf, -inf, 0}));
    EXPECT_EQ(div_dydz, std::vector<double>({0, inf, inf, 1}));
    EXPECT_EQ(div_dzdz, std::vector<double>({1,1,1,1}));
}

TEST_F(PathTest, TangentAngle) {
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto angles = this->path->tangent_angle(false);
    EXPECT_EQ(angles, std::vector<double>({0, 90, -90, -90}));
}

TEST_F(PathTest, Flip) {
    this->path->emplace_back(std::make_shared<Point>(0,1,1,0));
    auto flipped = this->path->flip();
    for (size_t n=0; n<this->path->size(); n++)
        EXPECT_EQ(*(*this->path)[n], *(*flipped)[this->path->size()-n-1]);
}

TEST_F(PathTest, SimplifyAbove) {
    this->path->emplace_back(std::make_shared<Point>(0.5,0,1,360));
    this->path->emplace_back(std::make_shared<Point>(0,1,1,450));
    this->path->emplace_back(std::make_shared<Point>(0,0.5,1,500));
    this->path->emplace_back(std::make_shared<Point>(0,0.5,0,600));
    this->path->emplace_back(std::make_shared<Point>(0,0.7,1,800));
    this->path->emplace_back(std::make_shared<Point>(0,0,1,1000));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,1000));
    auto p2 = this->path->simplify_above(0.5);
    // I assume here that if 2 points got suppressed and the ending angle
    // is right the remaining points are correct; I could be more thorough
    // and test each point separately though.
    EXPECT_EQ(p2->size(), 7);
    EXPECT_EQ((*p2)[p2->size()-1]->c, -80);
    auto p3 = this->path->simplify_above(2.0);
    EXPECT_EQ(p3->size(), 9);
    for (size_t n=0; n<this->path->size(); n++)
        EXPECT_EQ(*(*this->path)[n], *(*p3)[n]);
}

TEST_F(PathTest, SplitAbove) {
    this->path->emplace_back(std::make_shared<Point>(0.5,0,1,360));
    this->path->emplace_back(std::make_shared<Point>(0,1,1,450));
    this->path->emplace_back(std::make_shared<Point>(0,0.5,1,500));
    this->path->emplace_back(std::make_shared<Point>(0,0.5,0,600));
    this->path->emplace_back(std::make_shared<Point>(0.1,0.6,0,700));
    this->path->emplace_back(std::make_shared<Point>(0,0.7,1,800));
    this->path->emplace_back(std::make_shared<Point>(0,0,1,1000));
    this->path->emplace_back(std::make_shared<Point>(0,0,0,1000));
    auto p2 = this->path->split_above(0.5);
    EXPECT_EQ(p2.size(), 3);
    EXPECT_EQ(p2[0]->size(), 1);
    EXPECT_EQ(p2[1]->size(), 2);
    EXPECT_EQ(p2[2]->size(), 1);
    EXPECT_EQ(*(*p2[0])[0], *(*this->path)[0]);
    EXPECT_EQ(*(*p2[1])[0], Point(0,0.5,0,-120));
    EXPECT_EQ(*(*p2[1])[1], Point(0.1,0.6,0,-20));
    EXPECT_EQ(*(*p2[2])[0], Point(0,0,0,-80));

    auto p3 = this->path->split_above(2.0);
    EXPECT_EQ(p3.size(), 1);
    EXPECT_EQ(p3[0]->size(), 10);
    for (size_t n=0; n<this->path->size(); n++)
        EXPECT_EQ(*(*this->path)[n], *(*p3[0])[n]);
}

TEST_F(PathTest, CreateRamps) {
    // open path
    this->path->emplace_back(std::make_shared<Point>(2,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(3,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(4,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(5,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(6,0,1,0));
    this->path->emplace_back(std::make_shared<Point>(7,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(8,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(9,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(10,0,0,0));
    auto p_f = this->path->create_ramps(0.8, 0.5, 5.0, RampDirection::Forward);
    auto p_b = this->path->create_ramps(0.8, 0.5, 5.0, RampDirection::Backward);
    auto p_d = this->path->create_ramps(0.8, 0.5, 5.0, RampDirection::Both);
    EXPECT_EQ((*p_f)[0]->z, 0.0);
    EXPECT_EQ((*p_f)[1]->z, 1.0);
    EXPECT_EQ((*p_f)[2]->z, 0.2);
    EXPECT_EQ((*p_f)[5]->z, 0.5);
    EXPECT_EQ((*p_f)[6]->z, 1.0);
    EXPECT_EQ((*p_f)[7]->z, 0.2);
    EXPECT_EQ((*p_f)[10]->z, 0.5);

    EXPECT_EQ((*p_b)[0]->z, 0.0);
    EXPECT_EQ((*p_b)[1]->z, 1.0);
    EXPECT_EQ((*p_b)[2]->z, 0.5);
    EXPECT_EQ((*p_b)[5]->z, 0.2);
    EXPECT_EQ((*p_b)[6]->z, 1.0);
    EXPECT_EQ((*p_b)[7]->z, 0.5);
    EXPECT_EQ((*p_b)[10]->z, 0.2);

    EXPECT_EQ((*p_d)[0]->z, 0.0);
    EXPECT_EQ((*p_d)[1]->z, 1.0);
    EXPECT_EQ((*p_d)[2]->z, 0.5);
    EXPECT_EQ((*p_d)[5]->z, 0.5);
    EXPECT_EQ((*p_d)[6]->z, 1.0);
    EXPECT_EQ((*p_d)[7]->z, 0.5);
    EXPECT_EQ((*p_d)[10]->z, 0.5);

    // closed path
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto p_d2 = this->path->create_ramps(0.8, 0.5, 5.0, RampDirection::Both);
    EXPECT_EQ((*p_d2)[10]->z, 0.2);

    // limit height too high
    auto p_d3 = this->path->create_ramps(1.5, 0.5, 5.0, RampDirection::Both);
    EXPECT_EQ((*p_d3)[10]->z, 0.0);
}

TEST_F(PathTest, Rearrange) {
    // open path
    this->path->emplace_back(std::make_shared<Point>(2,0,0,0));
    this->path->emplace_back(std::make_shared<Point>(3,0,1,0));
    this->path->emplace_back(std::make_shared<Point>(4,0,0,0));
    auto p2 = this->path->rearrange(0.5, (*this->path)[0]);
    auto p3 = this->path->rearrange(0.5, (*this->path)[3]);
    EXPECT_EQ((*p2)[0]->x, 1.0);
    EXPECT_EQ((*p3)[0]->x, 3.0);
    auto p4 = this->path->rearrange(1.5, (*this->path)[0]);
    EXPECT_EQ((*p4)[0]->x, 0.0);
    // closed path
    this->path->emplace_back(std::make_shared<Point>(0,0,0,0));
    auto p5 = this->path->rearrange(0.5, (*this->path)[3]);
    EXPECT_EQ((*p5)[0]->x, 3.0);
    EXPECT_EQ(*(*p5)[5], *(*p5)[0]);
}
