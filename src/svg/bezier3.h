/** \file bezier3.h
 *  \brief Definition of Bezier3 class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <boost/math/quadrature/gauss_kronrod.hpp>

#include "segment.h"
#include "util.h"
#include "../log.h"

#define NEWTON_MAX_ITER 100 ///< Maximum number of iterations for Newton minimization
#define NEWTON_ERR_TOL 1e-6 ///< Error tolerance for Newton minimization

namespace pygraver::svg {
    
    /** \brief Cubic Bézier curve class.
     * 
     *  This produces a cubic Bézier curve out of control points p0, ..., p3.
     * 
     *  \tparam T: numeric type.
     */
    template <class T> class Bezier3 : public Segment<T> {
    private:
        /** \brief First control point. */
    	std::vector<T> p0 = {0,0};

        /** \brief Second control point. */
    	std::vector<T> p1 = {0,0};

        /** \brief Third control point. */
    	std::vector<T> p2 = {0,0};

        /** \brief Fourth control point. */
    	std::vector<T> p3 = {0,0};

        /** \brief Total length. */
    	T _length = 0;

    public:
        /** \brief Constructor.
         *  \param p0: 1st control point.
         *  \param p1: 2nd control point.
         *  \param p2: 3rd control point.
         *  \param p3: 4th control point.
         */
    	Bezier3(const std::vector<T> & p0, const std::vector<T> & p1, const std::vector<T> & p2, const std::vector<T> & p3) {
            this->p0 = p0;
            this->p1 = p1;
            this->p2 = p2;
            this->p3 = p3;
            this->_length = this->length(1);
        }
	
        /** \brief Get point coordinates for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return point coordinates.
         */
    	std::vector<T> point(const T t) const {
            std::vector<T> p(2);
            p[0] = this->p0[0]*pow(1-t, 3) + 3*this->p1[0]*t*pow(1-t, 2) + 3*this->p2[0]*(1-t)*pow(t, 2) + this->p3[0]*pow(t, 3);
            p[1] = this->p0[1]*pow(1-t, 3) + 3*this->p1[1]*t*pow(1-t, 2) + 3*this->p2[1]*(1-t)*pow(t, 2) + this->p3[1]*pow(t, 3);
            return p;
        }

        /** \brief Get derivative for given relative position.
         * 
         *  This gives the derivative versus t => dx/dt, dy/dt.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return derivative values.
         */
    	std::vector<T> dpoint(const T t) const {
            std::vector<T> p(2);
            p[0] = -3*this->p0[0]*pow(1-t, 2) + 3*this->p1[0]*(1-t)*(1-3*t) + 3*this->p2[0]*(2-3*t)*t + 3*this->p3[0]*pow(t, 2);
            p[1] = -3*this->p0[1]*pow(1-t, 2) + 3*this->p1[1]*(1-t)*(1-3*t) + 3*this->p2[1]*(2-3*t)*t + 3*this->p3[1]*pow(t, 2);
            return p;
        }

        /** \brief Get segment length for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return segment length.
         */
    	T length(const T t) const {
            if (t==1 && this->_length)
                return this->_length;
            auto F = [this] (T t) -> T { return this->arc(t); };
            T err;
            T Q = boost::math::quadrature::gauss_kronrod<T, 15>::integrate(F, 0, t, 5, 1e-6, &err);
            PYG_LOG_V("Computed bezier length from 0 to {} -> {}", t, Q);
            return Q;
        }

        /** \brief Get relative position for given length.
         *  \param l: sub-segment length.
         *  \return relative position between 0 and 1.
         */
    	T arg_at_length(const T l) const {
            T s = l/this->_length, new_s;
            unsigned int iter = NEWTON_MAX_ITER;
            do {
                new_s = s - (this->length(s) - l)/this->arc(s);
                if (abs(new_s - s) < NEWTON_ERR_TOL) break;
                s = new_s;
            } while (--iter);
            PYG_LOG_V("Minimized bezier parameter for l={} -> {}", l, new_s);
            return new_s;
        }
    };
}
