#include "types/point.h"
#include "types/path.h"
#include "types/pathgroup.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::types;

class PathGroupTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto path1 = std::make_shared<Path>(0);
        path1->emplace_back(std::make_shared<Point>(0,0,0,0));
        path1->emplace_back(std::make_shared<Point>(1,0,1,0));
        path1->emplace_back(std::make_shared<Point>(2,0,0,0));
        auto path2 = std::make_shared<Path>(0);
        path2->emplace_back(std::make_shared<Point>(5,5,0,90));
        path2->emplace_back(std::make_shared<Point>(6,6,1,45));
        path2->emplace_back(std::make_shared<Point>(7,7,0,0));
        path2->emplace_back(std::make_shared<Point>(5,5,0,90));
        auto path3 = std::make_shared<Path>(0);
        path3->emplace_back(std::make_shared<Point>(1,3,0,0));
        path3->emplace_back(std::make_shared<Point>(4,5,1,0));
        path3->emplace_back(std::make_shared<Point>(1,3,0,0));
        this->pathgroup = std::make_shared<PathGroup>();
        this->pathgroup->push_back(path1);
        this->pathgroup->push_back(path2);
        this->pathgroup->push_back(path3);
    }

    std::shared_ptr<PathGroup> pathgroup;
};

TEST_F(PathGroupTest, Construction) {
    std::vector<std::shared_ptr<Path>> paths;
    paths.push_back((*this->pathgroup)[0]);
    paths.push_back((*this->pathgroup)[1]);
    paths.push_back((*this->pathgroup)[2]);
    // alternative constructor
    auto pg = std::make_shared<PathGroup>(paths);
    EXPECT_EQ(this->pathgroup->size(), pg->size());
}

TEST_F(PathGroupTest, Subscripting) {
    EXPECT_NO_THROW((*this->pathgroup)[0]);
    EXPECT_NO_THROW((*this->pathgroup)[1]);
    EXPECT_NO_THROW((*this->pathgroup)[2]);
    EXPECT_THROW((*this->pathgroup)[3], std::out_of_range);
}

TEST_F(PathGroupTest, Envelope) {
    auto p = this->pathgroup->get_envelope();
    EXPECT_EQ(p.size(), 1);
    EXPECT_EQ(p[0]->size(), 4);
    EXPECT_EQ(*(*p[0])[0], Point(-5,5,0,0));
}

TEST_F(PathGroupTest, Steps) {
    auto steps = this->pathgroup->get_steps();
    EXPECT_EQ(steps.size(), 2);
    EXPECT_EQ(*steps[0], Point(5, 5, 0, 90));
    EXPECT_EQ(*steps[1], Point(-4, -2, 0, -90));
    auto pt = std::make_shared<Point>(0,0,0,0);
    this->pathgroup->set_steps({pt, pt});
    EXPECT_EQ(*(*(*this->pathgroup)[1])[1], Point(1, 1, 1, -45));
    EXPECT_EQ(*(*(*this->pathgroup)[2])[1], Point(3, 2, 1, 0));
}

TEST_F(PathGroupTest, Radius) {
    EXPECT_EQ(this->pathgroup->get_radius(), 6.48074069840786);
}

TEST_F(PathGroupTest, Centroid) {
    auto centroid = this->pathgroup->get_centroid();
    EXPECT_EQ(*centroid, Point(0.22222222222222232, 2.276142374915397, 0.1111111111111111, 0));
}

TEST_F(PathGroupTest, SortPaths) {
    auto pg2 = this->pathgroup->sort_paths((*(*this->pathgroup)[0])[0], SortPredicate::StartToStart);
    EXPECT_EQ(*(*(*this->pathgroup)[2])[0], *(*(*pg2)[1])[0]);
    auto pg3 = this->pathgroup->sort_paths((*(*this->pathgroup)[0])[0], SortPredicate::EndToStart);
    EXPECT_EQ(*(*(*this->pathgroup)[2])[0], *(*(*pg3)[1])[0]);
    auto pg4 = this->pathgroup->sort_paths((*(*this->pathgroup)[0])[0], SortPredicate::EndToEnd);
    EXPECT_EQ(*(*(*this->pathgroup)[2])[0], *(*(*pg4)[1])[0]);
}

TEST_F(PathGroupTest, Rearrange) {
    auto pg2 = this->pathgroup->rearrange(0.5);
    EXPECT_EQ(*(*(*pg2)[0])[0], Point(1, 0, 1, 0));
    EXPECT_EQ(*(*(*pg2)[1])[0], Point(6, 6, 1, 45));
    EXPECT_EQ(*(*(*pg2)[2])[0], Point(4, 5, 1, 0));
    auto pg3 = this->pathgroup->rearrange(1.5);
    EXPECT_EQ(*(*(*pg3)[0])[0], *(*(*this->pathgroup)[0])[0]);
    EXPECT_EQ(*(*(*pg3)[1])[0], *(*(*this->pathgroup)[1])[0]);
    EXPECT_EQ(*(*(*pg3)[2])[0], *(*(*this->pathgroup)[2])[0]);
}

TEST_F(PathGroupTest, Reorder) {
    auto pg2 = this->pathgroup->reorder({2, 1, 0});
    EXPECT_EQ((*pg2)[0], (*this->pathgroup)[2]);
    EXPECT_EQ((*pg2)[1], (*this->pathgroup)[1]);
    EXPECT_EQ((*pg2)[2], (*this->pathgroup)[0]);
}

TEST_F(PathGroupTest, Arithmetics) {
    auto pg2 = this->pathgroup + this->pathgroup;
    EXPECT_EQ(pg2->size(), 2*this->pathgroup->size());
    auto pg3 = this->pathgroup*3;
    EXPECT_EQ(pg3->size(), 3*this->pathgroup->size());
    auto pg4 = this->pathgroup + std::make_shared<Path>(0);
    EXPECT_EQ(pg4->size(), this->pathgroup->size()+1);
}
