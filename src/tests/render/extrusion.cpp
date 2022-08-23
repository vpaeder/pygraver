#include "render/extrusion.h"

#include "types/surface.h"
#include "types/path.h"
#include "types/point.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::render;
using namespace pygraver::types;
using namespace testing;

TEST(ExtrusionTest, MakePolyData) {
    auto path = std::make_shared<Path>(0);
    path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    path->emplace_back(std::make_shared<Point>(1, 0, 0, 0));
    path->emplace_back(std::make_shared<Point>(1, 1, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 1, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    auto polydata = make_polydata(path);
    auto points = polydata->GetPoints();
    EXPECT_EQ(points->GetNumberOfPoints(), 5);
    auto polys = polydata->GetPolys();
    EXPECT_EQ(polys->GetNumberOfCells(), 1);
    // alternative way: create PolyData from Surface;
    // this one applies Delaunay triangulation in case there are holes
    // => makes 2 triangles and retains 4 points
    auto surface = std::make_shared<Surface>(path);
    polydata = make_polydata(surface);
    points = polydata->GetPoints();
    EXPECT_EQ(points->GetNumberOfPoints(), 4);
    polys = polydata->GetPolys();
    EXPECT_EQ(polys->GetNumberOfCells(), 2);
    // same function but with one hole
    auto hole = path->buffer(-0.2);
    auto surface_with_hole = std::make_shared<Surface>(path, std::vector<std::shared_ptr<Path>>{hole});
    polydata = make_polydata(surface_with_hole);
    points = polydata->GetPoints();
    EXPECT_EQ(points->GetNumberOfPoints(), 8);
    polys = polydata->GetPolys();
    EXPECT_EQ(polys->GetNumberOfCells(), 8);
}

TEST(ExtrusionTest, ExtrudePolyData) {
    auto path = std::make_shared<Path>(0);
    path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    path->emplace_back(std::make_shared<Point>(1, 0, 0, 0));
    path->emplace_back(std::make_shared<Point>(1, 1, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 1, 0, 0));
    path->emplace_back(std::make_shared<Point>(0, 0, 0, 0));
    auto polydata = make_polydata(path);
    auto extrusion = extrude(polydata, std::make_shared<Point>(0,0,1,0), std::make_shared<Point>(0,0,0,0), 1);
    auto points = extrusion->GetPoints();
    EXPECT_EQ(points->GetNumberOfPoints(), 10);
    auto polys = extrusion->GetPolys();
    EXPECT_EQ(polys->GetNumberOfCells(), 2);
    // alternative way
    auto surface = std::make_shared<Surface>(path);
    polydata = make_polydata(surface);
    extrusion = extrude(polydata, std::make_shared<Point>(0,0,1,0), std::make_shared<Point>(0,0,0,0), 1);
    points = extrusion->GetPoints();
    EXPECT_EQ(points->GetNumberOfPoints(), 8);
    polys = extrusion->GetPolys();
    EXPECT_EQ(polys->GetNumberOfCells(), 4);
}
