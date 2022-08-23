/** \file path.h
 *  \brief Definition of Path class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <string>
#include <regex>

#include "line.h"
#include "arc.h"
#include "bezier3.h"
#include "util.h"
#include "shape.h"
#include "../log.h"

/** \brief Flag for SVG shape type: path. */
#define SVG_SHAPETYPE_PATH 2

namespace pygraver::svg {
    
    /** \brief Complex curve class.
     * 
     *  This produces a curve out of a \<path> element.
     * 
     *  \tparam T: numeric type.
     */
    template <typename T> class Path : public Shape<T> {
    private:
        /** \brief If true, curve is closed. */
    	bool is_closed = false;

    public:
        /** \brief Tell the type of shape. */
    	const unsigned short get_type() override {
            return SVG_SHAPETYPE_PATH;
        }

        /** \brief Constructor with curve descriptor.
         *  \param curve_str: curve descriptor; this is the value of the 'd' property
         *  of a \<path> element.
         */
    	Path(const std::string & curve_str) {
            PYG_LOG_V("Creating curve 0x{:x} from string {}", (uint64_t)this, curve_str);
            this->from_string(curve_str);
        }

        /** \brief Construct curve from given curve descriptor.
         *  \param curve_str: curve descriptor; this is the value of the 'd' property
         *  of a \<path> element.
         */
    	void from_string(const std::string & curve_str) {
            std::string obj_str = curve_str;
            // curve in SVG format (e.g. "M 10,10 L 5,0 ..." or "M10,10L5,0" or "M10 10L3 5")
            std::regex segment_regex("[MmLlHhVvCcSsQqTtAaZz] *([-0123456789eE.]+(,| |)+)*");
            std::regex args_regex("[-0123456789eE.]+");
            // implicit flag; SVG commands can be implicitly specified (e.g. "m 10,10 5,0" ~ "m 10,10 m 5,0");
            // if we find numbers at the end of a command instead of another command,
            // we try to treat these numbers as a new command of the same type
            std::string current_command = "", previous_command = "";
            // start and end points for current curve segment
            std::vector<T> p0 = {0,0}, p1 = {0,0}, p2 = {0,0}, p3 = {0,0};
            PYG_LOG_V("Constructing curve...");
            for (auto seg_it = std::sregex_iterator(obj_str.begin(), obj_str.end(), segment_regex); seg_it != std::sregex_iterator(); ++seg_it) {
                std::string seg_match = seg_it->str();
                current_command = seg_match[0];

                for (auto arg_it = std::sregex_iterator(seg_match.begin()+1, seg_match.end(), args_regex); arg_it != std::sregex_iterator(); ++arg_it) {
                    if (current_command == "M" || current_command == "m") {
                        // move to - absolute
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "m")
                            vec2_add<T>(p1, p0);
                        PYG_LOG_V("Found move-to command with parameters:");
                        PYG_LOG_V("  destination: ({},{})", p1[0], p1[1]);
                        // in case there's an implicit command, next point should be considered as a line
                        current_command = (current_command == "M") ? "L" : "l";
                    } else if (current_command == "L" || current_command == "l") {
                        // line
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "l")
                            vec2_add<T>(p1, p0);
                        PYG_LOG_V("Found line with parameters:");
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Line<T>>(p0, p1));
                    } else if (current_command == "H" || current_command == "h") {
                        // horizontal line
                        p1[0] = parse_value<T>(arg_it->str());
                        p1[1] = p0[1];
                        if (current_command == "h")
                            p1[0] += p0[0];
                        PYG_LOG_V("Found horizontal line with parameters:");
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Line<T>>(p0, p1));
                    } else if (current_command == "V" || current_command == "v") {
                        // vertical line - absolute
                        p1[0] = p0[0];
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "v")
                            p1[1] += p0[1];
                        PYG_LOG_V("Found vertical line with parameters:");
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Line<T>>(p0, p1));
                    } else if (current_command == "C" || current_command == "c") {
                        // cubic bezier
                        p2[0] = parse_value<T>((arg_it++)->str());
                        p2[1] = parse_value<T>((arg_it++)->str());
                        p3[0] = parse_value<T>((arg_it++)->str());
                        p3[1] = parse_value<T>((arg_it++)->str());
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "c") {
                            vec2_add<T>(p1, p0);
                            vec2_add<T>(p2, p0);
                            vec2_add<T>(p3, p0);
                        }
                        PYG_LOG_V("Found cubic bezier curve with parameters:");
                        PYG_LOG_V("  handle point 1: ({},{})", p2[0], p2[1]);
                        PYG_LOG_V("  handle point 2: ({},{})", p3[0], p3[1]);
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Bezier3<T>>(p0, p2, p3, p1));
                    } else if (current_command == "S" || current_command == "s") {
                        // smooth cubic bézier
                        if (previous_command == "C" || previous_command == "S"
                            || previous_command == "c" || previous_command == "s") {
                            p2[0] = p0[0] - p3[0];
                            p2[1] = p0[1] - p3[1];
                        } else {
                            p2[0] = p0[0];
                            p2[1] = p0[1];
                        }
                        p3[0] = parse_value<T>((arg_it++)->str());
                        p3[1] = parse_value<T>((arg_it++)->str());
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "s") {
                            vec2_add<T>(p1, p0);
                            vec2_add<T>(p3, p0);
                        }
                        PYG_LOG_V("Found smooth cubic bezier curve with parameters:");
                        PYG_LOG_V("  handle point 1: ({},{})", p2[0], p2[1]);
                        PYG_LOG_V("  handle point 2: ({},{})", p3[0], p3[1]);
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Bezier3<T>>(p0, p2, p3, p1));
                    } else if (current_command == "Q" || current_command == "q") {
                        // quadratic bezier
                        p2[0] = parse_value<T>((arg_it++)->str());
                        p2[1] = parse_value<T>((arg_it++)->str());
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "q") {
                            vec2_add<T>(p2, p0);
                            vec2_add<T>(p1, p0);
                        }
                        PYG_LOG_V("Found quadratic bezier curve with parameters:");
                        PYG_LOG_V("  handle point 1: ({},{})", p2[0], p2[1]);
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Bezier3<T>>(p0, p2, std::vector<T>{0,0}, p1));
                    } else if (current_command == "T" || current_command == "t") {
                        // smooth quadratic bézier
                        if (previous_command == "T" || previous_command == "Q"
                            || previous_command == "t" || previous_command == "q") {
                            p2[0] = p1[0] - p2[0];
                            p2[1] = p1[1] - p2[1];
                        } else {
                            p2[0] = p0[0];
                            p2[1] = p0[1];
                        }
                        p1[0] = parse_value<T>((arg_it++)->str());
                        p1[1] = parse_value<T>(arg_it->str());
                        if (current_command == "t")
                            vec2_add<T>(p1, p0);
                        PYG_LOG_V("Found smooth quadratic bezier curve with parameters:");
                        PYG_LOG_V("  handle point 1: ({},{})", p2[0], p2[1]);
                        PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                        this->segments.push_back(std::make_unique<Bezier3<T>>(p0, p2, std::vector<T>{0,0}, p1));
                    } else if (current_command == "A" || current_command == "a") {
                        // arc
                        std::vector<T> r(2);
                        r[0] = parse_value<T>((arg_it++)->str());
                        r[1] = parse_value<T>(arg_it->str());
                        // the SVG TR specifies a number of invalid cases and how to handle them;
                        // e.g. if only one of the radii is 0, we generate a line instead;
                        // I assume SVG generators already handle that, so if one radius is 0 I just
                        // dump the segment (check your SVG);
                        // the other case is when p0 and p1 are specified with incompatible radii;
                        // in this case, the arc code tries to find a suitable combination
                        // that permits connecting p0 and p1 with an arc, and throws an exception
                        // if it fails; in this case, debug logs won't reflect adjustments.
                        if (r[0]!=0 && r[1]!=0) {
                            arg_it++;
                            T angle = parse_value<T>((arg_it++)->str())/180*M_PI;
                            int large_arc_flag = std::stoi((arg_it++)->str());
                            int sweep_flag = std::stoi((arg_it++)->str());
                            p1[0] = parse_value<T>((arg_it++)->str());
                            p1[1] = parse_value<T>(arg_it->str());
                            if (current_command == "a")
                                vec2_add<T>(p1, p0);
                            PYG_LOG_V("Found arc with parameters:");
                            PYG_LOG_V("  radii: ({},{})", r[0], r[1]);
                            PYG_LOG_V("  tilt angle: {} degrees", angle/M_PI*180);
                            PYG_LOG_V("  large arc flag is {}set", large_arc_flag ? "" : "not ");
                            PYG_LOG_V("  sweep flag is {}set", sweep_flag ? "" : "not ");
                            PYG_LOG_V("  end point: ({},{})", p1[0], p1[1]);
                            this->segments.push_back(std::make_unique<Arc<T>>(p0, p1, r, angle, large_arc_flag==1, sweep_flag==1));
                        }
                    } else {
                        PYG_LOG_I("Unknown SVG curve segment type: {}", current_command);
                    }
                    previous_command = current_command;
                    p0 = p1;
                    PYG_LOG_V("Next segment starting at ({},{})", p0[0], p0[1]);
                }
            }
            if (current_command == "z" || current_command == "Z") {
                // close path
                p0 = this->segments.back()->point(1);
                p1 = this->segments[0]->point(0);
                PYG_LOG_V("Found closing line. Adding line from ({},{}) to ({},{})", p0[0], p1[1], p1[0], p1[1]);
                this->segments.push_back(std::make_unique<Line<T>>(p0, p1));
                this->is_closed = true;
            }
        }

        /** \brief Interpolate the shape with constant step size.
         *  \param dl: step size.
         *  \returns a collection of point coordinate values.
         */
    	std::vector<std::vector<T>> interpolate(const T dl) const override {
            PYG_LOG_D("Interpolating curve 0x{:x}", (uint64_t)this);
            PYG_LOG_D("Number of segments: {}", this->segments.size());
            std::vector<std::vector<T>> points;
            for (auto & seg : this->segments) {
                auto new_points = seg->interpolate(dl);
                points.reserve(points.size() + new_points.size());
                points.insert(points.end(), new_points.begin(), new_points.end());
            }
            if (!this->is_closed)
                points.push_back(this->segments.back()->point(1));

            return points;
        }
    };

}
