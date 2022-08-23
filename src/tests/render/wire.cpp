#include "render/wire.h"

#include "types/point.h"
#include "types/path.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::render;
using namespace pygraver::types;
using namespace testing;

TEST(WireTest, Base) {
    auto path = std::make_shared<Path>(0);
    path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 1, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 2, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 3, 0, 0));
    auto wire = Wire(path, 0.5, std::vector<uint8_t>{0,0,0});
    auto actors = wire.get_actors();
    EXPECT_EQ(actors->GetNumberOfItems(), 1);
    auto actor = static_cast<vtkActor*>(actors->GetItemAsObject(0));
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{5, 0, 0}), 4.75);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{5, 3, 0}), 4.75);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{0, 0, 5}), 4.75);
    EXPECT_PRED_FORMAT2(DoubleLE, Shape3D::distance_to_actor(actor, std::vector<double>{0, 3, 5}), 4.75);
}

TEST(WireCollectionTest, Base) {
    std::vector<std::shared_ptr<Path>> paths;
    auto base_path = std::make_shared<Path>(0);
    base_path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    base_path->emplace_back(std::make_shared<Point>(0, 1, 0, 0));
    base_path->emplace_back(std::make_shared<Point>(0, 2, 0, 0));
    base_path->emplace_back(std::make_shared<Point>(0, 3, 0, 0));
    paths.emplace_back(base_path);
    paths.emplace_back(base_path->shift(std::make_shared<Point>(10,10,10,0)));
    paths.emplace_back(base_path->shift(std::make_shared<Point>(20,20,20,0)));
    paths.emplace_back(base_path->shift(std::make_shared<Point>(30,30,30,0)));
    auto wires = WireCollection(paths, 0.5, std::vector<uint8_t>{0,0,0});
    auto actors = wires.get_actors();
    EXPECT_EQ(actors->GetNumberOfItems(), 4);
}
