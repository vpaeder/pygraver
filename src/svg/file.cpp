/** \file file.cpp
 *  \brief Implementation file for File class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <locale>
#include <clocale>
#include <libxml/parser.h>

#include "path.h"
#include "ellipse.h"
#include "rect.h"
#include "file.h"
#include "util.h"
#include "../log.h"

namespace pygraver::svg {
    
    File::File(const std::string & filename) {
      // need this otherwise stod is locale-dependent
      std::setlocale(LC_ALL, "C");
      std::locale::global(std::locale("C"));
    	this->open(filename);
    }

    void File::open(const std::string & filename) {
    	auto doc = xmlReadFile(filename.c_str(), nullptr, 0);
        if (doc == nullptr)
			throw std::runtime_error("Couldn't read file " + filename);

		this->parse_document(doc);
    }

	void File::from_memory(const std::string & buffer) {
		auto doc = xmlReadMemory(buffer.c_str(), buffer.size(), "buffer.svg", nullptr, 0);
        if (doc == nullptr)
			throw std::runtime_error("Couldn't parse buffer as SVG.");

		this->parse_document(doc);
	}

	void File::parse_document(const xmlDocPtr doc) {
    	if (this->root != nullptr)
    	    xmlFreeDoc(this->root->doc);

        this->root = xmlDocGetRootElement(doc);
    	// get drawing centre
    	std::string view_box = get_prop(this->root, "viewBox");
    	std::vector<std::string> split_vec = split_string(view_box, " ");
		this->centre = std::make_shared<types::Point>(
			(std::stod(split_vec[0]) + std::stod(split_vec[2]))/2,
			(std::stod(split_vec[1]) + std::stod(split_vec[3]))/2
		);
		PYG_LOG_V("Created SVG File object 0x{:x}", (uint64_t)this);
	}

    File::~File() {
		PYG_LOG_V("Deleting SVG File object 0x{:x}", (uint64_t)this);
    	if (!this->root) {
    		xmlFreeDoc(this->root->doc);
    	    xmlCleanupParser();
    	}
    }

	void File::check_opened() const {
		if (this->root == nullptr)
			throw std::runtime_error("No SVG document opened.");
	}

    xmlNodePtr File::get_layer(const std::string & name) const {
    	for (auto cur = this->root->children; cur != nullptr; cur = cur->next)
    		if ((!xmlStrcmp(cur->name, (const xmlChar *)"g"))) {
				// try several attributes as some tools, like Inkscape, store name in non-standard fields
				xmlChar * prop;
				prop = xmlGetProp(cur, (const xmlChar *)"id");
				if (prop == nullptr || xmlStrcmp(prop, (const xmlChar *)name.c_str()))
					prop = xmlGetProp(cur, (const xmlChar *)"name");
				if (prop == nullptr || xmlStrcmp(prop, (const xmlChar *)name.c_str()))
					prop = xmlGetProp(cur, (const xmlChar *)"label");
				if (prop == nullptr || xmlStrcmp(prop, (const xmlChar *)name.c_str()))
					prop = xmlGetNsProp(cur, (const xmlChar *)"label", (const xmlChar *)"inkscape");
				if (prop != nullptr && !xmlStrcmp(prop, (const xmlChar *)name.c_str()))
					return cur;
			}
    	
    	return nullptr;
    }

	std::string File::get_transform(xmlNodePtr node) const {
		auto transform = xmlGetProp(node, (const xmlChar*)"transform");
		if (transform != nullptr)
			return std::string(reinterpret_cast<char*>(transform));
		return std::string();
	}

    std::vector<std::unique_ptr<Shape<number_t>>> File::get_shapes(xmlNodePtr node) const {
    	std::vector<std::unique_ptr<Shape<number_t>>> shapes;
		auto base_transform = this->get_transform(node);
    	for (auto cur = node->children; cur != nullptr; cur = cur->next) {
			std::unique_ptr<Shape<number_t>> new_shape;
    		if ((!xmlStrcmp(cur->name, (const xmlChar *)"path"))) {
				new_shape = std::make_unique<Path<number_t>>(get_prop(cur, "d"));
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"polyline"))) {
				auto points = "M" + get_prop(cur, "points");
				new_shape = std::make_unique<Path<number_t>>(points);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"polygon"))) {
				auto points = "M" + get_prop(cur, "points") + "z";
				new_shape = std::make_unique<Path<number_t>>(points);
    		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"circle"))) {
    			new_shape = std::make_unique<Ellipse<number_t>>(cur);
    		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ellipse"))) {
    			new_shape = std::make_unique<Ellipse<number_t>>(cur);
    		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rect"))) {
    			new_shape = std::make_unique<Rectangle<number_t>>(cur);
    		} else {
    			if (cur->children != nullptr) {
    				auto new_shapes = this->get_shapes(cur);
    				if (new_shapes.size()>0) {
    					shapes.reserve(shapes.size() + new_shapes.size());
						for (auto & shape: new_shapes) {
							shape->transforms.emplace_back(base_transform);
							shapes.emplace_back(std::move(shape));
						}
    				}
    			}
    		}
			if (new_shape != nullptr) {
				new_shape->transforms.emplace_back(this->get_transform(cur));
				new_shape->transforms.emplace_back(base_transform);
				shapes.emplace_back(std::move(new_shape));
			}
    	}
    	return shapes;
    }

	std::vector<std::unique_ptr<Shape<number_t>>> File::get_shapes(const std::string & layer_name) const {
		auto layer = this->get_layer(layer_name);
		if (layer == nullptr)
			throw std::runtime_error("Cannot find layer: " + layer_name);
		return this->get_shapes(layer);
	}

    std::vector<std::shared_ptr<types::Path>> File::get_paths(const std::string & layer_name, const number_t dl) const {
		this->check_opened();
    	std::vector<std::shared_ptr<types::Path>> paths;
    	auto node = this->get_layer(layer_name);
    	if (node != nullptr) {
    		auto shapes = this->get_shapes(node);
    		paths.reserve(shapes.size());
    		auto inv_centre = -(this->centre);
    		for (auto &shp : shapes)
    			paths.emplace_back(shp->to_path(dl)->shift(inv_centre));
    	}
    	return paths;
    }

    std::vector<std::shared_ptr<types::Point>> File::get_points(const std::string & layer_name) const {
		this->check_opened();
    	std::vector<std::shared_ptr<types::Point>> points;
    	auto node = this->get_layer(layer_name);
    	if (node != nullptr) {
    		auto shapes = this->get_shapes(node);
    		points.reserve(shapes.size());
    		for (auto & shp : shapes)
    			if (shp->get_type() == SVG_SHAPETYPE_ELLIPSE || shp->get_type() == SVG_SHAPETYPE_RECTANGLE)
    				points.emplace_back(shp->centre() - this->centre);
    	}
    	return points;
    }

    std::vector<number_t> File::get_size() const {
		this->check_opened();
        auto vb_string = std::string(reinterpret_cast<const char*>(xmlGetProp(this->root, (const xmlChar *)"viewBox")));
        auto split_vec = split_string(vb_string, " ");
        std::vector<number_t> view_box(4);
        for (int i=0; i<split_vec.size(); i++)
            view_box[i] = std::stod(split_vec[i]);
    
        return view_box;
    }
    
}

