/** \file segment.h
 *  \brief Definition of Segment base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <math.h>
#include <iostream>

#include "../log.h"

namespace pygraver::svg {
    
    /** \brief Segment class.
     * 
     *  This represents an elementary segment.
     * 
     *  \tparam T: numeric type.
     */
    template <typename T> class Segment {
    public:
        /** \brief Get point coordinates for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return point coordinates.
         */
    	virtual std::vector<T> point(const T t) const {
        	return {0,0};
        }

        /** \brief Get derivative for given relative position.
         * 
         *  This gives the derivative versus t => dx/dt, dy/dt.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return derivative values.
         */
    	virtual std::vector<T> dpoint(const T t) const {
    	    return {0,0};
        }

        /** \brief Get coordinates of segment centre.
         *  \return centre point coordinates.
         */
    	virtual std::vector<T> centre() const {
    	    return {0,0};
        }

        /** \brief Get arc derivative for given relative position.
         * 
         *  This is used to compute arc length or position along segment.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return value of arc derivative.
         */
    	virtual T arc(const T t) const {
            auto p = this->dpoint(t);
            return sqrt(pow(p[0], 2) + pow(p[1], 2));
        }

        /** \brief Get segment length for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return segment length.
         */
    	virtual T length(const T t) const {
    	    return 0;
        }

        /** \brief Get relative position for given length.
         *  \param l: sub-segment length.
         *  \return relative position between 0 and 1.
         */
    	virtual T arg_at_length(const T l) const {
          	return 0;
        }

        /** \brief Interpolate segment with constant step size.
         *  \param dl: interpolation step size.
         *  \returns a collection of point coordinate values.
         */
    	virtual std::vector<std::vector<T>> interpolate(const T dl) const {
            std::vector<std::vector<T>> points;
            T l = this->length(1);
            auto np = ceil(l/dl)+1;
            PYG_LOG_D("Interpolating segment 0x{:x}", (uint64_t)this);
            PYG_LOG_D("Segment length: {}", l);
            PYG_LOG_D("Number of points: {}", (int)np);
            points.reserve(np);
            points.emplace_back(this->point(0));
            if (np>1) {
                T t0 = 0, dt = l/np;
                for (auto n=1; n<=np; n++) {
                    t0 = this->arg_at_length(n*dt);
                    points.emplace_back(this->point(t0));
                }
            }
    
            return points;
        }
    };
    
}
