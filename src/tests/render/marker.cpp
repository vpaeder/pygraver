#include "render/marker.h"

#include "types/point.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::render;
using namespace pygraver::types;
using namespace testing;

TEST(MarkerTest, Base) {
    auto marker = Marker('H',
                         std::make_shared<Point>(5, 5, 0, 0),
                         2, 2, 2,
                         std::make_shared<Point>(0, 0, 1, 0),
                         std::vector<uint8_t>{0,0,0}
                         );
    auto actors = marker.get_actors();
    EXPECT_EQ(actors->GetNumberOfItems(), 1);
    auto actor = static_cast<vtkActor*>(actors->GetItemAsObject(0));
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{5, 5, -5}), 5);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{5, 5, 5}), 3);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{10, 5, 0}), 4);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{0, 5, 0}), 4);
}

TEST(MarkerCollectionTest, Base) {
    std::vector<std::shared_ptr<Point>> points;
    points.emplace_back(std::make_shared<Point>(0,0,0,0));
    points.emplace_back(std::make_shared<Point>(10,10,0,0));
    points.emplace_back(std::make_shared<Point>(0,0,10,0));
    points.emplace_back(std::make_shared<Point>(-10,10,10,0));
    auto markers = MarkerCollection('H', points, 2, 2, 2,
                                    std::make_shared<Point>(0, 0, 1, 0),
                                    std::vector<uint8_t>{0,0,0});
    
    auto actors = markers.get_actors();
    EXPECT_EQ(actors->GetNumberOfItems(), 4);
}
