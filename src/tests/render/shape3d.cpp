#include "render/extrusion.h"
#include "render/shape3d.h"

#include <vtkMapper.h>

#include "types/surface.h"
#include "types/path.h"
#include "types/point.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::render;
using namespace pygraver::types;
using namespace testing;

class Shape3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto path = std::make_shared<Path>(0);
        path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
        path->emplace_back(std::make_shared<Point>(1, 0, 0, 0));
        path->emplace_back(std::make_shared<Point>(1, 1, 0, 0));
        path->emplace_back(std::make_shared<Point>(0, 1, 0, 0));
        path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
        this->shape = std::make_shared<Extrusion>(path, 1, std::make_shared<Point>(0, 0, 1, 0), std::vector<uint8_t>{0,0,0,0});
    }
    std::shared_ptr<Shape3D> shape;
};


TEST_F(Shape3DTest, GetActors) {
    auto actors = this->shape->get_actors();
    EXPECT_EQ(actors->GetNumberOfItems(), 1);
}

TEST_F(Shape3DTest, Colors) {
    // invalid color
    EXPECT_THROW(this->shape->set_base_color(std::vector<uint8_t>{0,0}), std::invalid_argument);
    EXPECT_THROW(this->shape->set_highlight_color(std::vector<uint8_t>{0,0}), std::invalid_argument);
    // valid color
    EXPECT_NO_THROW(this->shape->set_base_color(std::vector<uint8_t>{0,0,0}));
    EXPECT_NO_THROW(this->shape->set_highlight_color(std::vector<uint8_t>{0,0,0}));
    EXPECT_EQ(this->shape->get_base_color().size(), 3);
    EXPECT_EQ(this->shape->get_highlight_color().size(), 3);
    EXPECT_NO_THROW(this->shape->set_base_color(std::vector<uint8_t>{128,128,128,128}));
    EXPECT_NO_THROW(this->shape->set_highlight_color(std::vector<uint8_t>{129,129,129,129}));
    // retrieve color
    auto col = this->shape->get_base_color();
    EXPECT_EQ(col.size(), 4);
    EXPECT_EQ(col[0], 128);
    EXPECT_EQ(col[1], 128);
    EXPECT_EQ(col[2], 128);
    EXPECT_EQ(col[3], 128);
    col = this->shape->get_highlight_color();
    EXPECT_EQ(col.size(), 4);
    EXPECT_EQ(col[0], 129);
    EXPECT_EQ(col[1], 129);
    EXPECT_EQ(col[2], 129);
    EXPECT_EQ(col[3], 129);
}

TEST_F(Shape3DTest, Label) {
    std::string label = "something";
    this->shape->set_label(label);
    EXPECT_EQ(this->shape->get_label(), label);
}

TEST_F(Shape3DTest, ScalarMode) {
    this->shape->set_scalar_color_range(1, 3);
    auto range = this->shape->get_scalar_color_range();
    EXPECT_EQ(range[0], 1);
    EXPECT_EQ(range[1], 3);
    EXPECT_FALSE(this->shape->get_scalar_color_mode());
    this->shape->set_scalar_color_mode(true);
    EXPECT_TRUE(this->shape->get_scalar_color_mode());
    this->shape->toggle_scalar_color_mode();
    EXPECT_FALSE(this->shape->get_scalar_color_mode());
}

TEST_F(Shape3DTest, Visibility) {
    this->shape->set_visible(true);
    EXPECT_TRUE(this->shape->get_visibility());
    this->shape->set_visible(false);
    EXPECT_FALSE(this->shape->get_visibility());
    this->shape->toggle_visibility();
    EXPECT_TRUE(this->shape->get_visibility());
}

TEST_F(Shape3DTest, Highlight) {
    this->shape->set_highlighted(true);
    EXPECT_TRUE(this->shape->get_highlighted(0));
    this->shape->set_highlighted(0, false);
    EXPECT_FALSE(this->shape->get_highlighted(0));
    this->shape->toggle_highlighted(0);
    EXPECT_TRUE(this->shape->get_highlighted(0));
    EXPECT_THROW(this->shape->set_highlighted(1, true), std::out_of_range);
    EXPECT_THROW(this->shape->get_highlighted(1), std::out_of_range);
    EXPECT_THROW(this->shape->toggle_highlighted(1), std::out_of_range);
}

TEST_F(Shape3DTest, PointInside) {
    EXPECT_TRUE(this->shape->is_point_inside({0.1, 0.1, 0.1})); // inside
    EXPECT_FALSE(this->shape->is_point_inside({-0.1, -0.1, -0.1})); // outside
    EXPECT_TRUE(this->shape->is_point_inside({0.1, 0.1, 0.0})); // on surface
}

TEST_F(Shape3DTest, DistanceToActor) {
    auto actor = static_cast<vtkActor*>(this->shape->get_actors()->GetItemAsObject(0));
    auto dist1 = this->shape->distance_to_actor(actor, std::vector<double>{0, 0, 3});
    EXPECT_EQ(dist1, 2);
    auto dist2 = this->shape->distance_to_actor(actor, std::vector<double>{0, 0, 1});
    EXPECT_EQ(dist2, 0);
    auto dist3 = this->shape->distance_to_actor(actor, std::vector<double>{0, 0, 0.5});
    EXPECT_EQ(dist3, 0);
    auto dist4 = this->shape->distance_to_actor(actor, std::vector<double>{0, 0, -2});
    EXPECT_EQ(dist4, 2);
    
}

TEST_F(Shape3DTest, ClosestActor) {
    // create 2nd shape and add it to this->shape
    auto path = std::make_shared<Path>(0);
    path->emplace_back(std::make_shared<Point>(0, 0, 2, 0));
    path->emplace_back(std::make_shared<Point>(1, 0, 2, 0));
    path->emplace_back(std::make_shared<Point>(1, 1, 2, 0));
    path->emplace_back(std::make_shared<Point>(0, 1, 2, 0));
    path->emplace_back(std::make_shared<Point>(0, 0, 2, 0));
    auto shape = std::make_shared<Extrusion>(path, 1, std::make_shared<Point>(0, 0, 1, 0), std::vector<uint8_t>{0,0,0,0});
    auto actor = static_cast<vtkActor*>(shape->get_actors()->GetItemAsObject(0));
    auto polydata = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
    this->shape->set_item(1, polydata);
    // test closest_actor method
    auto [distance1, closest1] = this->shape->closest_actor(std::vector<double>{0, 0, 10});
    EXPECT_EQ(distance1, 7);
    EXPECT_EQ(closest1, this->shape->get_actors()->GetItemAsObject(1));
    auto [distance2, closest2] = this->shape->closest_actor(std::vector<double>{0, 0, -2});
    EXPECT_EQ(distance2, 2);
    EXPECT_EQ(closest2, this->shape->get_actors()->GetItemAsObject(0));
}

TEST_F(Shape3DTest, IntersectingActor) {
    auto shp_actor = this->shape->get_actors()->GetItemAsObject(0);
    auto actor1 = this->shape->intersecting_actor(std::vector<double>{0, 0, -10}, std::vector<double>{0, 0, 10});
    EXPECT_EQ(actor1, shp_actor);
    auto actor2 = this->shape->intersecting_actor(std::vector<double>{0, 0, -10}, std::vector<double>{0, 0, 0.5});
    EXPECT_EQ(actor2, shp_actor);
    auto actor3 = this->shape->intersecting_actor(std::vector<double>{0, 0, 0.5}, std::vector<double>{0, 0, 10});
    EXPECT_EQ(actor3, shp_actor);
    auto actor4 = this->shape->intersecting_actor(std::vector<double>{0, 0, -10}, std::vector<double>{0, 0, -5});
    EXPECT_EQ(actor4, nullptr);
    auto actor5 = this->shape->intersecting_actor(std::vector<double>{0, 0, 5}, std::vector<double>{0, 0, 10});
    EXPECT_EQ(actor5, nullptr);
}
