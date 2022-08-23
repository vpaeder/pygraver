/** \file line.h
 *  \brief Definition of Line class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>

#include "segment.h"
#include "../log.h"

namespace pygraver::svg {
    
    /** \brief Straight line class.
     * 
     *  This produces a line out of two end points.
     * 
     *  \tparam T: numeric type.
     */
    template <typename T> class Line : public Segment<T> {
    private:
        /** \brief Line length. */
    	T _length = 0;

        /** \brief Start point. */
    	std::vector<T> p0 = {0,0};

        /** \brief End point. */
    	std::vector<T> p1 = {0,0};
    public:
        /** \brief Constructor.
         *  \param p0: start point.
         *  \param p1: end point.
         */
    	Line(const std::vector<T> & p0, const std::vector<T> & p1) {
        	this->set(p0, p1);
        }

        /** \brief Set line points.
         *  \param p0: start point.
         *  \param p1: end point.
         */
    	void set(const std::vector<T> & p0, const std::vector<T> & p1) {
            std::copy(p0.begin(), p0.end(), this->p0.begin());
            std::copy(p1.begin(), p1.end(), this->p1.begin());
            this->_length = sqrt(pow(p1[0]-p0[0], 2) + pow(p1[1]-p0[1], 2));
            PYG_LOG_V("Setting line 0x{:x} from point ({},{}) to point ({},{}).", (uint64_t)this, this->p0[0], this->p0[1], this->p1[0], this->p1[1]);
        }
	
        /** \brief Get point coordinates for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return point coordinates.
         */
    	std::vector<T> point(const T t) const override {
            std::vector<T> p(2);
            p[0] = t*(this->p1[0] - this->p0[0]) + this->p0[0];
            p[1] = t*(this->p1[1] - this->p0[1]) + this->p0[1];
            return p;
        }

        /** \brief Get derivative for given relative position.
         * 
         *  This gives the derivative versus t => dx/dt, dy/dt.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return derivative values.
         */
    	std::vector<T> dpoint(const T t) const override {
            std::vector<T> p(2);
            p[0] = this->p1[0] - this->p0[0];
            p[1] = this->p1[1] - this->p0[1];
            return p;
        }

        /** \brief Get arc derivative for given relative position.
         * 
         *  This is used to compute arc length or position along segment.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return value of arc derivative.
         */
    	T arc(const T t) const override {
    	    return this->length(t);
        }

        /** \brief Get segment length for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return segment length.
         */
    	T length(const T t) const override {
        	return t * this->_length;
        }

        /** \brief Get relative position for given length.
         *  \param l: sub-segment length.
         *  \return relative position between 0 and 1.
         */
    	T arg_at_length(const T l) const override {
        	return l/this->_length;
        }

        /** \brief Interpolate segment with constant step size.
         *  \param dl: interpolation step size.
         *  \returns a collection of point coordinate values.
         */
    	std::vector<std::vector<T>> interpolate(const T dl) const override {
            std::vector<std::vector<T>> points;
            auto np = ceil(this->_length/dl);
            PYG_LOG_D("Interpolating line 0x{:x}", (uint64_t)this);
            PYG_LOG_D("Line length: {}", this->_length);
            PYG_LOG_D("From point ({},{}) to point ({},{}).", this->p0[0], this->p0[1], this->p1[0], this->p1[1]);
            PYG_LOG_D("Number of points: {}", (int)np);
            points.reserve(np);
            points.push_back(this->p0);
            if (np>1) {
                T dt0 = T(1)/np, dt = 0;
                for (auto i=0; i<np-1; i++) {
                    dt += dt0;
                    points.emplace_back(this->point(dt));
                }
            }
            return points;  
        }
	
    };
    
}
