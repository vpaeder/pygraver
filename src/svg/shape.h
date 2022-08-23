/** \file shape.h
 *  \brief Definition of Shape base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <string>
#include <regex>

#include "../types/point.h"
#include "../types/path.h"
#include "segment.h"

namespace pygraver::svg {
      
    /** \brief Shape class.
     * 
     *  This represents a shape made of one or more segments.
     * 
     *  \tparam T: numeric type.
     */
    template <typename T> class Shape {
    protected:
        /** \brief Segments composing shape. */
    	std::vector<std::unique_ptr<Segment<T>>> segments;

    public:
        Shape() = default;
        virtual ~Shape() = default;

        /** \brief Transform strings. */
        std::vector<std::string> transforms;

        /** \brief Give access to shape segments.
         * 
         *  This is essentially used for testing purpose.
         * 
         *  \returns segments vector.
         */
        std::vector<std::unique_ptr<Segment<T>>> & get_segments() { return this->segments; }


        /** \brief Tell the type of shape.
         * 
         *  This is used to avoid problems with decltype demangling.
         */
    	virtual const unsigned short get_type() = 0;

        /** \brief Interpolate the shape with constant step size.
         *  \param dl: step size.
         *  \returns a collection of point coordinate values.
         */
    	virtual std::vector<std::vector<T>> interpolate(const T dl) const = 0;

        /** \brief Generate a Path object from shape.
         *  \param dl: interpolation step size.
         *  \returns a path object.
         */
    	virtual std::shared_ptr<types::Path> to_path(const T dl) const {
            auto new_points = this->interpolate(dl);
            auto path = std::make_shared<types::Path>(new_points);
            // apply transforms
            bool has_transforms = false; // use flag to avoid applying transform if none were found
            std::vector<double> matrix{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            std::regex transform_regex(R"((?:translate|scale|rotate|skewX|skewY|matrix)\((?:[-0-9, .Ee]+)\))");
            for (auto transform = this->transforms.begin(); transform != this->transforms.end(); ++transform) {
                // need to reverse order
                std::vector<std::string> transforms;
                for (auto tr_it = std::sregex_iterator(transform->begin(), transform->end(), transform_regex); tr_it != std::sregex_iterator(); ++tr_it)
                    transforms.emplace_back(tr_it->str());
                for (auto tr_it = transforms.rbegin(); tr_it != transforms.rend(); ++tr_it) {
                    // extract transform parameters
                    auto par_pos = tr_it->find("(");
                    auto operation = tr_it->substr(0, par_pos);
                    std::regex params_regex("([-0-9 ]+[Ee.]*[-0-9]*)+");
                    std::vector<T> params;
                    for (auto param_it = std::sregex_iterator(tr_it->begin()+par_pos, tr_it->end(), params_regex); param_it != std::sregex_iterator(); ++param_it)
                        params.emplace_back(stod(param_it->str()));
                    // apply transform
                    if (operation == "translate") {
                        PYG_LOG_D("Transform: translation by ({},{})", params[0], params[1]);
                        matrix[0] += params[0]*matrix[12];
                        matrix[1] += params[0]*matrix[13];
                        matrix[2] += params[0]*matrix[14];
                        matrix[3] += params[0]*matrix[15];
                        matrix[4] += params[1]*matrix[12];
                        matrix[5] += params[1]*matrix[13];
                        matrix[6] += params[1]*matrix[14];
                        matrix[7] += params[1]*matrix[15];
                    } else if (operation == "scale") {
                        PYG_LOG_D("Transform: scaling by ({},{})", params[0], params[1]);
                        matrix[0] *= params[0];
                        matrix[1] *= params[0];
                        matrix[2] *= params[0];
                        matrix[3] *= params[0];
                        matrix[4] *= params[1];
                        matrix[5] *= params[1];
                        matrix[6] *= params[1];
                        matrix[7] *= params[1];
                    } else if (operation == "rotate") {
                        PYG_LOG_D("Transform: rotation by {} degrees", params[0]);
                        auto t = params[0]/180*M_PI;
                        auto c = cos(t);
                        auto s = sin(t);
                        auto m0 = c*matrix[0] - s*matrix[4];
                        auto m1 = c*matrix[1] - s*matrix[5];
                        auto m2 = c*matrix[2] - s*matrix[6];
                        auto m3 = c*matrix[3] - s*matrix[7];
                        matrix[4] = s*matrix[0] + c*matrix[4];
                        matrix[5] = s*matrix[1] + c*matrix[5];
                        matrix[6] = s*matrix[2] + c*matrix[6];
                        matrix[7] = s*matrix[3] + c*matrix[7];
                        matrix[0] = m0;
                        matrix[1] = m1;
                        matrix[2] = m2;
                        matrix[3] = m3;
                    } else if (operation == "skewX") {
                        PYG_LOG_D("Transform: horizontal skew by {} degrees", params[0]);
                        auto t = tan(params[0]/180*M_PI);
                        matrix[0] = matrix[0] + t*matrix[4];
                        matrix[1] = matrix[1] + t*matrix[5];
                        matrix[2] = matrix[2] + t*matrix[6];
                        matrix[3] = matrix[3] + t*matrix[7];
                    } else if (operation == "skewY") {
                        PYG_LOG_D("Transform: vertical skew by {} degrees", params[0]);
                        auto t = tan(params[0]/180*M_PI);
                        matrix[4] = t*matrix[0] + matrix[4];
                        matrix[5] = t*matrix[1] + matrix[5];
                        matrix[6] = t*matrix[2] + matrix[6];
                        matrix[7] = t*matrix[3] + matrix[7];
                    } else if (operation == "matrix") {
                        PYG_LOG_D("Transform: matrix with parameters {},{},{},{},{},{}", params[0],
                                  params[1], params[2], params[3], params[4], params[5]);
                        auto m0 = params[0]*matrix[0] + params[2]*matrix[4] + params[4]*matrix[12];
                        auto m1 = params[0]*matrix[1] + params[2]*matrix[5] + params[4]*matrix[13];
                        auto m2 = params[0]*matrix[2] + params[2]*matrix[6] + params[4]*matrix[14];
                        auto m3 = params[0]*matrix[3] + params[2]*matrix[7] + params[4]*matrix[15];
                        matrix[4] = params[1]*matrix[0] + params[3]*matrix[4] + params[5]*matrix[12];
                        matrix[5] = params[1]*matrix[1] + params[3]*matrix[5] + params[5]*matrix[13];
                        matrix[6] = params[1]*matrix[2] + params[3]*matrix[6] + params[5]*matrix[14];
                        matrix[7] = params[1]*matrix[3] + params[3]*matrix[7] + params[5]*matrix[15];
                        matrix[0] = m0;
                        matrix[1] = m1;
                        matrix[2] = m2;
                        matrix[3] = m3;
                    } else {
                        throw std::invalid_argument("Invalid transform encountered: " + operation);
                    }
                    has_transforms = true;
                }
            }
            PYG_LOG_D("Transform matrix:");
            PYG_LOG_D(" {}  {}  {}  {}", matrix[0], matrix[1], matrix[2], matrix[3]);
            PYG_LOG_D(" {}  {}  {}  {}", matrix[4], matrix[5], matrix[6], matrix[7]);
            PYG_LOG_D(" {}  {}  {}  {}", matrix[8], matrix[9], matrix[10], matrix[11]);
            PYG_LOG_D(" {}  {}  {}  {}", matrix[12], matrix[13], matrix[14], matrix[15]);
            if (has_transforms) return path->matrix_transform(matrix);
            return path;
        }

        /** \brief Get shape centre point.
         *  \returns a Point object containing centre point coordinates.
         */
    	virtual std::shared_ptr<types::Point> centre() const {
        	return std::make_shared<types::Point>();
        }
    };
  
}
