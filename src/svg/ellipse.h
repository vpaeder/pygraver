/** \file ellipse.h
 *  \brief Definition of Ellipse class.
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
#include "arc.h"
#include "shape.h"

/** \brief Flag for SVG shape type: ellipse. */
#define SVG_SHAPETYPE_ELLIPSE 1

namespace pygraver::svg {
    
    /** \brief Ellipse class.
     * 
     *  This produces an ellipse out of \<ellipse> or \<circle> elements.
     * 
     *  \tparam T: numeric type.
     */
    template <class T> class Ellipse : public Shape<T> {
    public:
        /** \brief Tell the type of shape. */
    	const unsigned short get_type() override {
    	return SVG_SHAPETYPE_ELLIPSE;
        }

        /** \brief Constructor with XML node.
         *  \param node: pointer to XML node to read data from.
         */
    	Ellipse(xmlNodePtr node) {
        	this->from_tag(node);
        }

        /** \brief Retrieve rectangle parameters from XML node.
         *  \param node: pointer to XML node to read data from.
         */
    	void from_tag(xmlNodePtr node) {
            std::vector<T> r(2), c(2); 
            c[0] = std::stod(get_prop(node, "cx"));
            c[1] = std::stod(get_prop(node, "cy"));
            for (auto attr = node->properties; attr; attr = attr->next) {
                if (!xmlStrcmp(attr->name, (const xmlChar *)"rx")) {
                    r[0] = std::stod(get_prop(node, "rx"));
                    r[1] = std::stod(get_prop(node, "ry"));
                    break;
                } else if (!xmlStrcmp(attr->name, (const xmlChar *)"r")) {
                    r[0] = std::stod(get_prop(node, "r"));
                    r[1] = r[0];
                    break;
                }
            }
            this->segments.push_back(std::make_unique<Arc<T>>(c, r, T(0), T(M_2PI), T(0)));
        }

        /** \brief Get shape centre point.
         *  \returns a Point object containing centre point coordinates.
         */
    	std::shared_ptr<types::Point> centre() const override {
            auto v = this->segments.back()->centre();
            return std::make_shared<types::Point>(v[0], v[1]);
        }

        /** \brief Interpolate the shape with constant step size.
         *  \param dl: step size.
         *  \returns a collection of point coordinate values.
         */
    	std::vector<std::vector<T>> interpolate(const T dl) const override {
        	return this->segments.back()->interpolate(dl);
        }
    };
    
}
