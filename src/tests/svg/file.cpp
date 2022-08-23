#include "svg/file.h"
#include "svg/rect.h"
#include "svg/ellipse.h"
#include "svg/path.h"

#include <gtest/gtest.h>

using namespace pygraver;
using namespace pygraver::svg;
using namespace testing;

TEST(FileTest, ParseAttributeNames) {
    std::string attrib_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg viewBox=\"0 0 1200 400\" xmlns=\"http://www.w3.org/2000/svg\"\
    xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\
    version=\"1.1\">\
    <g id=\"layer_id\"><circle cx=\"600\" cy=\"200\" r=\"100\"/></g>\
    <g label=\"layer_label\"><circle cx=\"600\" cy=\"200\" r=\"100\"/></g>\
    <g name=\"layer_name\"><circle cx=\"600\" cy=\"200\" r=\"100\"/></g>\
    <g id=\"layer\" inkscape:label=\"layer_inkscape\"><circle cx=\"600\" cy=\"200\" r=\"100\"/></g>\
    <g random_tag=\"layer_random\"><circle cx=\"600\" cy=\"200\" r=\"100\"/></g>\
    </svg>";

    auto f = File();
    f.from_memory(attrib_svg);
    auto shp1 = f.get_shapes("layer_id");
    EXPECT_EQ(shp1.size(), 1);
    auto shp2 = f.get_shapes("layer_label");
    EXPECT_EQ(shp2.size(), 1);
    auto shp3 = f.get_shapes("layer_name");
    EXPECT_EQ(shp3.size(), 1);
    auto shp4 = f.get_shapes("layer_inkscape");
    EXPECT_EQ(shp4.size(), 1);
    EXPECT_THROW(f.get_shapes("layer_random"), std::runtime_error);
}

TEST(FileTest, ParseEllipse) {
    std::string circle_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\"/>\
    <ellipse cx=\"600\" cy=\"200\" rx=\"200\" ry=\"100\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(circle_svg);
    auto shapes = f.get_shapes("layer");

    EXPECT_EQ(shapes.size(), 2);
    EXPECT_EQ(shapes[0]->get_type(), SVG_SHAPETYPE_ELLIPSE);
    EXPECT_EQ(shapes[1]->get_type(), SVG_SHAPETYPE_ELLIPSE);
    auto centre = shapes[0]->centre();
    EXPECT_EQ(centre->x, 600);
    EXPECT_EQ(centre->y, 200);
    auto & segs1 = shapes[0]->get_segments();
    EXPECT_EQ(segs1.size(), 1); // 1 arc
    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(0.25);
    auto pt1c = segs1[0]->point(0.5);
    auto pt1d = segs1[0]->point(0.75);
    auto pt1e = segs1[0]->point(1);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1a[0], 700);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1a[1], 200);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1b[0], 600);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1b[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[0], 500);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[1], 200);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1d[0], 600);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1d[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1e[0], 700);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1e[1], 200);
    auto & segs2 = shapes[1]->get_segments();
    EXPECT_EQ(segs2.size(), 1); // 1 arc
    auto pt2a = segs2[0]->point(0);
    auto pt2b = segs2[0]->point(0.25);
    auto pt2c = segs2[0]->point(0.5);
    auto pt2d = segs2[0]->point(0.75);
    auto pt2e = segs2[0]->point(1);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2a[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2a[1], 200);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2b[0], 600);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2b[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2c[0], 400);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2c[1], 200);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2d[0], 600);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2d[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2e[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2e[1], 200);
}

TEST(FileTest, ParseRectangle) {
    std::string rect_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <rect x=\"400\" y=\"100\" width=\"400\" height=\"200\"/>\
    <rect x=\"100\" y=\"100\" width=\"400\" height=\"200\" rx=\"50\" ry=\"40\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(rect_svg);
    auto shapes = f.get_shapes("layer");

    EXPECT_EQ(shapes.size(), 2);
    EXPECT_EQ(shapes[0]->get_type(), SVG_SHAPETYPE_RECTANGLE);
    EXPECT_EQ(shapes[1]->get_type(), SVG_SHAPETYPE_RECTANGLE);

    auto & segs1 = shapes[0]->get_segments();
    EXPECT_EQ(segs1.size(), 4); // 4 lines

    auto & segs2 = shapes[1]->get_segments();
    EXPECT_EQ(segs2.size(), 8); // 4 lines + 4 arcs

    auto pt1a = segs1[0]->point(0);
    auto pt1b = segs1[0]->point(1.0);
    auto pt1c = segs1[1]->point(0.0);
    auto pt1d = segs1[1]->point(1.0);
    auto pt1e = segs1[2]->point(0.0);
    auto pt1f = segs1[2]->point(1.0);
    auto pt1g = segs1[3]->point(0.0);
    auto pt1h = segs1[3]->point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1a[0], 400);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1a[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1b[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1b[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1c[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1d[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1d[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1e[0], 800);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1e[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1f[0], 400);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1f[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1g[0], 400);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1g[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1h[0], 400);
    EXPECT_PRED_FORMAT2(DoubleLE, pt1h[1], 100);

    auto pt2a = segs2[0]->point(0);
    auto pt2b = segs2[0]->point(1.0);
    auto pt2c = segs2[1]->point(0.0);
    auto pt2d = segs2[1]->point(1.0);
    auto pt2e = segs2[2]->point(0.0);
    auto pt2f = segs2[2]->point(1.0);
    auto pt2g = segs2[3]->point(0.0);
    auto pt2h = segs2[3]->point(1.0);
    auto pt2i = segs2[4]->point(0.0);
    auto pt2j = segs2[4]->point(1.0);
    auto pt2k = segs2[5]->point(0.0);
    auto pt2l = segs2[5]->point(1.0);
    auto pt2m = segs2[6]->point(0.0);
    auto pt2n = segs2[6]->point(1.0);
    auto pt2o = segs2[7]->point(0.0);
    auto pt2p = segs2[7]->point(1.0);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2a[0], 150);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2a[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2b[0], 450);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2b[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2c[0], 450);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2c[1], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2d[0], 500);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2d[1], 140);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2e[0], 500);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2e[1], 140);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2f[0], 500);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2f[1], 260);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2g[0], 500);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2g[1], 260);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2h[0], 450);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2h[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2i[0], 450);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2i[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2j[0], 150);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2j[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2k[0], 150);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2k[1], 300);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2l[0], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2l[1], 260);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2m[0], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2m[1], 260);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2n[0], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2n[1], 140);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2o[0], 100);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2o[1], 140);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2p[0], 150);
    EXPECT_PRED_FORMAT2(DoubleLE, pt2p[1], 100);
}

TEST(FileTest, ParsePath) {
    std::string path_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <path d=\"M 100 100 L 300 100 L 200 300 z\"/>\
    <polyline points=\"50,375\
                    150,375 150,325 250,325 250,375\
                    350,375 350,250 450,250 450,375\
                    550,375 550,175 650,175 650,375\
                    750,375 750,100 850,100 850,375\
                    950,375 950,25 1050,25 1050,375\
                    1150,375\"/>\
    <polygon points=\"350,75 379,161 469,161 397,215\
                    423,301 350,250 277,301 303,215\
                    231,161 321,161\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(path_svg);
    auto shapes = f.get_shapes("layer");

    EXPECT_EQ(shapes.size(), 3);
    EXPECT_EQ(shapes[0]->get_type(), SVG_SHAPETYPE_PATH);
    EXPECT_EQ(shapes[1]->get_type(), SVG_SHAPETYPE_PATH);
    EXPECT_EQ(shapes[2]->get_type(), SVG_SHAPETYPE_PATH);

    auto & segs1 = shapes[0]->get_segments();
    EXPECT_EQ(segs1.size(), 3);
    auto & segs2 = shapes[1]->get_segments();
    EXPECT_EQ(segs2.size(), 21);
    auto & segs3 = shapes[2]->get_segments();
    EXPECT_EQ(segs3.size(), 10);
    // path construction is tested in path.cpp
}

TEST(FileTest, ParsePoints) {
    std::string shapes_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"400\" cy=\"100\" r=\"100\"/>\
    <ellipse cx=\"700\" cy=\"300\" rx=\"200\" ry=\"100\"/>\
    <rect x=\"400\" y=\"100\" width=\"400\" height=\"200\"/>\
    <path d=\"M 100 100 L 300 100 L 200 300 z\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(shapes_svg);
    auto sz = f.get_size();
    EXPECT_PRED_FORMAT2(DoubleLE, sz[0], 1200);
    EXPECT_PRED_FORMAT2(DoubleLE, sz[1], 400);

    auto points = f.get_points("layer");
    // paths centre would be the centroid, which is not computed here;
    // => 3 shapes, not 4
    EXPECT_EQ(points.size(), 3);
    EXPECT_PRED_FORMAT2(DoubleLE, points[0]->x, -200);
    EXPECT_PRED_FORMAT2(DoubleLE, points[0]->y, -100);
    EXPECT_PRED_FORMAT2(DoubleLE, points[1]->x, 100);
    EXPECT_PRED_FORMAT2(DoubleLE, points[1]->y, 100);
    EXPECT_PRED_FORMAT2(DoubleLE, points[2]->x, 0);
    EXPECT_PRED_FORMAT2(DoubleLE, points[2]->y, 0);
}

TEST(FileTest, ParseTranslate) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"translate(10,-10)\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(transform_svg);

    auto paths = f.get_paths("layer", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->x, 110);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->y, -10);
}

TEST(FileTest, ParseRotate) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"rotate(90)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"rotate(180)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"rotate(-90)\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(transform_svg);

    auto paths = f.get_paths("layer", 0.1);
    // rotation is calculated around view box centre
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->x, -800);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->y, 500);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->x, -1300);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->y, -400);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->x, -400);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->y, -900);
}

TEST(FileTest, ParseScale) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"scale(2,3)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"scale(-3,-2)\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(transform_svg);

    auto paths = f.get_paths("layer", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->x, 800);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->y, 400);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->x, -2700);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->y, -600);
}

TEST(FileTest, ParseSkew) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"skewX(45)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"skewX(-45)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"skewY(45)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"skewY(-45)\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(transform_svg);

    auto paths = f.get_paths("layer", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->x, 300);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->y, 0);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->x, -100);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->y, 0);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->x, 100);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->y, 700);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[3])[0]->x, 100);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[3])[0]->y, -700);
}

TEST(FileTest, ParseMatrix) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"matrix(1,0,0,1,10,-10)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"matrix(0,-1,1,0,0,0)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"matrix(2,0,0,3,0,0)\"/>\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"matrix(1,-1,0,1,0,0)\"/>\
    </g>\
    </svg>";

    auto f = File();
    f.from_memory(transform_svg);

    auto paths = f.get_paths("layer", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->x, 110);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[0])[0]->y, -10);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->x, -400);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[1])[0]->y, -900);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->x, 800);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[2])[0]->y, 400);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[3])[0]->x, 100);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths[3])[0]->y, -700);
}

TEST(FileTest, ParseMultiTransform) {
    std::string transform_svg = "<?xml version=\"1.0\" standalone=\"no\"?>\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\
    \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\
    <svg width=\"12cm\" height=\"4cm\" viewBox=\"0 0 1200 400\"\
    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\
    <g id=\"layer1\" transform=\"translate(600,200)\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"rotate(-90)translate(-600,-200)\"/>\
    </g>\
    <g id=\"layer2\" transform=\"translate(600,200) matrix(0,-1,1,0,0,0)\">\
    <circle cx=\"600\" cy=\"200\" r=\"100\" transform=\"invalid_transform(10,10) translate(-600,-200)\"/>\
    </g>\
    </svg>";


    auto f = File();
    f.from_memory(transform_svg);

    auto paths1 = f.get_paths("layer1", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths1[0])[0]->x, 0);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths1[0])[0]->y, -100);

    auto paths2 = f.get_paths("layer2", 0.1);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths2[0])[0]->x, 0);
    EXPECT_PRED_FORMAT2(DoubleLE, (*paths2[0])[0]->y, -100);

}

