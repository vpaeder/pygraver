/** \file rect.h
 *  \brief Definition of Rectangle class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <string>
#include <libxml/tree.h>

#include "../types/point.h"
#include "util.h"
#include "line.h"
#include "arc.h"
#include "shape.h"

/** \brief Flag for SVG shape type: rectangle. */
#define SVG_SHAPETYPE_RECTANGLE 0

namespace pygraver::svg {
    
    /** \brief Rectangle class.
     * 
     *  This produces a rectangle out of a \<rect> element.
     * 
     *  \tparam T: numeric type.
     */
    template <typename T> class Rectangle : public Shape<T> {
    private:
        /** \brief Centre point. */
        std::shared_ptr<types::Point> c;

    public:
        /** \brief Tell the type of shape. */
    	const unsigned short get_type() override {
    	return SVG_SHAPETYPE_RECTANGLE;
        }

        /** \brief Constructor with XML node.
         *  \param node: pointer to XML node to read data from.
         */
    	Rectangle(xmlNodePtr node) {
        	this->from_tag(node);
        }

        /** \brief Retrieve rectangle parameters from XML node.
         *  \param node: pointer to XML node to read data from.
         */
    	void from_tag(xmlNodePtr node) {
            T x = get_number<T>(node, "x");
            T y = get_number<T>(node, "y");
            T w = get_number<T>(node, "width");
            T h = get_number<T>(node, "height");
            this->c = std::make_shared<types::Point>(x + w/2, y + h/2);
            T rx = 0, ry = 0;
            bool has_rx = false, has_ry = false;
            for (auto attr = node->properties; attr; attr = attr->next) {
                if (!xmlStrcmp(attr->name, (const xmlChar *)"rx")) {
                    rx = get_number<T>(node, "rx", w);
                    has_rx = true;
                } else if (!xmlStrcmp(attr->name, (const xmlChar *)"ry")) {
                    ry = get_number<T>(node, "ry", h);
                    has_ry = true;
                }
            }
            if (has_rx && !has_ry)
                ry = rx;
            if (has_ry && !has_rx)
                rx = ry;
            // rectangle with straight corners => 4 lines
            if (!has_rx && !has_ry) {
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x, y}, std::vector<T>{x+w, y}));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x+w, y}, std::vector<T>{x+w, y+h}));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x+w, y+h}, std::vector<T>{x, y+h}));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x, y+h}, std::vector<T>{x, y}));
            } else {
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x+rx, y}, std::vector<T>{x+w-rx, y}));
                this->segments.emplace_back(std::make_unique<Arc<T>>(std::vector<T>{x+w-rx, y+ry}, std::vector<T>{rx, ry}, M_PI_2, M_PI, M_PI));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x+w, y+ry}, std::vector<T>{x+w, y+h-ry}));
                this->segments.emplace_back(std::make_unique<Arc<T>>(std::vector<T>{x+w-rx, y+h-ry}, std::vector<T>{rx, ry}, 0, M_PI_2, 0));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x+w-rx, y+h}, std::vector<T>{x+rx, y+h}));
                this->segments.emplace_back(std::make_unique<Arc<T>>(std::vector<T>{x+rx, y+h-ry}, std::vector<T>{rx, ry}, M_PI_2, M_PI, 0));
                this->segments.emplace_back(std::make_unique<Line<T>>(std::vector<T>{x, y+h-ry}, std::vector<T>{x, y+ry}));
                this->segments.emplace_back(std::make_unique<Arc<T>>(std::vector<T>{x+rx, y+ry}, std::vector<T>{rx, ry}, 0, M_PI_2, M_PI));
            }
        }

        /** \brief Get shape centre point.
         *  \returns a Point object containing centre point coordinates.
         */
    	std::shared_ptr<types::Point> centre() const override {
            return this->c;
        }

        /** \brief Interpolate the shape with constant step size.
         *  \param dl: step size.
         *  \returns a collection of point coordinate values.
         */
    	std::vector<std::vector<T>> interpolate(const T dl) const override {
            std::vector<std::vector<T>> points;
            for (auto & seg : this->segments) {
                auto new_points = seg->interpolate(dl);
                points.reserve(points.size() + new_points.size());
                points.insert(points.end(), new_points.begin(), new_points.end());
            }
            return points;
        }
    };
    
}
