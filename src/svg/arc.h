/** \file arc.h
 *  \brief Definitions of Arc class and related functions.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <cmath>
#include "../types/common.h"
#include "segment.h"
#include "util.h"
#include "../log.h"

#define ELLIPTIC_MAX_ITER 100 ///< Maximum number of iterations for iterative algorithms

namespace pygraver::svg {
    
    /** \brief Compute the Carlson symmetric integral RF.
     * 
     *  This algorithm and the one for RD are adapted from:
     *      B.C. Carlson and E.M. Notis, Algorithms for Incomplete Elliptic Integrals,
     *      ACM Transactions on Mathematical Software 7, 3 (September 1981), pp. 398-403
     * 
     *  \tparam T: numeric type.
     *  \param x: 1st parameter.
     *  \param y: 2nd parameter.
     *  \param z: 3rd parameter.
     *  \param errtol: error tolerance.
     *  \returns computed value.
     */
    template <typename T> static T carlson_rf(const T x, const T y, const T z, const T errtol) {
        if (x<0 || y<0 || z<0)
            throw std::invalid_argument{"x, y, and z must be positive"};
        
        T eps, mu;
        T xn = x, yn = std::max(y, std::numeric_limits<T>::min()), zn = z;
        T xndev, yndev, zndev;
        T xnroot, ynroot, znroot;
        T lambda;
        
        unsigned int iter = ELLIPTIC_MAX_ITER;
        do {
            mu = (xn + yn + zn)/3;
            xndev = 2 - (mu + xn)/mu;
            yndev = 2 - (mu + yn)/mu;
            zndev = 2 - (mu + zn)/mu;
            eps = std::max({xndev, -xndev, yndev, -yndev, zndev, -zndev});
            if (eps < errtol) break;
            xnroot = sqrt(xn);
            ynroot = sqrt(yn);
            znroot = sqrt(zn);
            lambda = xnroot * (ynroot + znroot) + ynroot * znroot;
            xn = (xn + lambda)/4;
            yn = (yn + lambda)/4;
            zn = (zn + lambda)/4;
        } while (--iter);

        T e1 = xndev * yndev;
        T e2 = e1 - zndev * zndev;
        T e3 = e1 * zndev;
        T s = 1 + (e2/24 - 0.1 - 3*e3/44) * e2 + e3/14;

        return s / sqrt(mu);
    }

    /** \brief Compute the Carlson symmetric integral RD.
     *  \tparam T: numeric type.
     *  \param x: 1st parameter.
     *  \param y: 2nd parameter.
     *  \param z: 3rd parameter.
     *  \param errtol: error tolerance.
     *  \returns computed value.
     */
    template <typename T> static T carlson_rd(const T x, const T y, const T z, const T errtol) {
        if (x<0 || y<0 || z<0)
            throw std::invalid_argument{"x, y, and z must be positive"};
        
        T eps, mu;
        T xn = x, yn = std::max(y, std::numeric_limits<T>::epsilon()), zn = z;
        T pow4 = 1;
        T xndev, yndev, zndev;
        T xnroot, ynroot, znroot;
        T lambda, sigma=0;

        unsigned int iter = ELLIPTIC_MAX_ITER;
        do {
            mu = (xn + yn + 3 * zn)/5;
            xndev = (mu - xn) / mu;
            yndev = (mu - yn) / mu;
            zndev = (mu - zn) / mu;
            eps = std::max({xndev, -xndev, yndev, -yndev, zndev, -zndev});
            if (eps < errtol) break;
            xnroot = sqrt(xn);
            ynroot = sqrt(yn);
            znroot = sqrt(zn);
            lambda = xnroot * (ynroot + znroot) + ynroot * znroot;
            sigma += pow4 / (znroot * (zn + lambda));
            pow4 /= 4;
            xn = (xn + lambda)/4;
            yn = (yn + lambda)/4;
            zn = (zn + lambda)/4;
        } while (--iter);

        T ea = xndev * yndev;
        T eb = zndev * zndev;
        T ec = ea - eb;
        T ed = ea - 6*eb;
        T ef = ed + ec + ec;
        T s1 = ed * (9/88*ed - 9/52*zndev * ef - 3/14);
        T s2 = zndev * (ef/6 + zndev * (zndev * ea*3/26 - ec*9/22));
        return 3 * sigma + pow4 * (1 + s1 + s2) / (mu * sqrt(mu));
    }

    /** \brief Compute the complete elliptic integral of the second kind.
     *  \tparam T: numeric type.
     *  \param k: elliptic modulus [0, 1].
     *  \param errtol: error tolerance.
     *  \returns computed value.
     */
    template <typename T> static T elliptic_e(const T k, const T errtol) {
        T kk = k*k;
        return carlson_rf<T>(0, 1-kk, 1, errtol) - kk/3*carlson_rd<T>(0, 1-kk, 1, errtol);
    }

    /** \brief Compute the incomplete elliptic integral of the second kind.
     *  \tparam T: numeric type.
     *  \param phi: modular angle, in radians (-inf, inf).
     *  \param k: elliptic modulus [0, 1].
     *  \param errtol: error tolerance.
     *  \returns computed value.
     */
    template <typename T> static T elliptic_e(const T phi, const T k, const T errtol) {
        // The original version works for phi in [-pi/2,pi/2] (+/- 1/4 of an ellipse).
        // Because the integrand is periodic, we can derive values beyond pi/2
        // by adding quarter ellipse lengths until reaching the right quadrant.
        int mf = (phi>=0) ? (phi + M_PI_2)/M_PI : (phi - M_PI_2)/M_PI;
        T c = cos(phi), s = sin(phi);
        T x = c*c, ss = s*s, kk = k*k;
        T y = 1 - kk*ss;
        T v = s*(carlson_rf<T>(x, y, 1, errtol) - kk*s*s/3*carlson_rd<T>(x, y, 1, errtol));
        T corr = 0;
        if (mf)
            corr = 2*mf*elliptic_e(k, errtol);
        if (mf & 1)
            return corr - v;
        else
            return corr + v;
    }
    

    /** \brief Compute the inverse of the incomplete elliptic integral of the second kind.
     * 
     *  This uses Newton's minimization algorithm with initial value from:
     *      J.P. Boyd, Numerical, perturbative and Chebyshev inversion of the incomplete
     *      elliptic integral of the second kind, Appl. Math. Comp. 218 (2012), pp. 7005-7013
     * 
     *  \tparam T: numeric type.
     *  \param L: arc length.
     *  \param k: elliptic modulus [0, 1].
     *  \param errtol: error tolerance.
     *  \returns computed value.
     */
    template <typename T> static T inv_elliptic_e(const T L, const T k, const T errtol) {
        T E1 = elliptic_e(k, errtol);
        T zeta = 1 - L/E1;
        T mu = 1 - k;
        T r = sqrt(zeta * zeta + mu * mu);
        T theta = atan(mu/(L + std::numeric_limits<T>::epsilon()));
        T res = M_PI_2 + sqrt(r) * (theta - M_PI_2);
        T nres;
        T s;
        unsigned int iter = ELLIPTIC_MAX_ITER;
        do {
            s = sin(res);
            nres = res - (elliptic_e(res, k, errtol) - L)/sqrt(1 - k*s*s);
            if (abs(nres - res) < errtol) break;
            res = nres;
        } while (--iter);
        return nres;
    }

    /** \brief Radial arc segment class.
     * 
     *  This produces an arc segment out of appropriate data.
     * 
     *  \tparam T: numeric type.
     */
    template <class T> class Arc : public Segment<T> {
    private:
        /** \brief Arc start point. */
    	std::vector<T> p0 = {0,0};

        /** \brief Arc radii. */
    	std::vector<T> r = {0,0};

        /** \brief Centre point. */
    	std::vector<T> pc = {0,0};

        /** \brief Arc start angle. */
    	T t_start = 0;

        /** \brief Arc end angle. */
    	T t_end = 0;

        /** \brief Arc's tilt angle. */
    	T angle = 0;

        /** \brief Circle flag: if true, arc is circular; if false, arc is elliptic. */
    	bool is_circle = true;

        /** \brief Cosine of arc's tilt angle. */
    	T ca = 0;

        /** \brief Sine of arc's tilt angle. */
    	T sa = 0;

        /** \brief Arc length. */
    	T _length = 0;

    public:
        /** \brief Constructor with centre point and angles.
         *  \param pc: centre point.
         *  \param r: radii.
         *  \param t_start: start angle.
         *  \param t_end: end angle.
         *  \param angle: orientation, in radians.
         */
    	Arc(const std::vector<T> & pc, const std::vector<T> & r,
            const T t_start, const T t_end, const T angle) {
            this->pc = pc;
            this->r = r;
            this->ca = cos(angle);
            this->sa = sin(angle);
            this->t_start = t_start;
            this->t_end = t_end;
            this->is_circle = r[0]==r[1];
            // store full length
            this->_length = this->length(1);
        }

        /** \brief Constructor with start and end points.
         *  \param start: start point.
         *  \param end: end point.
         *  \param r: radii.
         *  \param angle: orientation, in radians.
         *  \param large_arc_flag: if true, arc spans the largest arc between start
         *  and end points; if false, spans shortest arc.
         *  \param sweep_flag: if true, sweeps arc in counter-clockwise direction.
         */
    	Arc(const std::vector<T> & start, const std::vector<T> & end,
            const std::vector<T> & r, const T angle, const bool large_arc_flag,
            const bool sweep_flag) {
            this->p0 = start;
            this->r = r;
            this->ca = cos(angle);
            this->sa = sin(angle);
            this->is_circle = r[0]==r[1];
            // computation of arc angles following https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
            std::vector<T> dp(2), p1(2), pc1(2), u(2), v(2);
            dp[0] = (start[0] - end[0])/2.0;
            dp[1] = (start[1] - end[1])/2.0;
            //p1[0] = this->ca*dp[0] + this->sa*dp[1];
            //p1[1] = -this->sa*dp[0] + this->ca*dp[1];
            p1[0] = dp[0];
            p1[1] = dp[1];
            std::vector<T> p12 = vec2_pow<T>(p1, 2);
            std::vector<T> r2 = vec2_pow<T>(r, 2);
            T sign = (large_arc_flag ^ sweep_flag) ? 1 : -1;
            // compute half-distance vector
            T pc0_num = r2[0]*r2[1] - r2[0]*p12[1] - r2[1]*p12[0];
            // if we cannot connect points with arc of given radius, pc0_num may be negative;
            // in this case, we attempt to correct the problem by adjusting radii until pc0_num==0
            while (pc0_num<0) {
                r2[0] -= pc0_num/2;
                r2[1] -= pc0_num/2;
                if (r2[0]<0 || r2[1]<0)
                    throw std::runtime_error("SVG: cannot find suitable arc radii to connect endpoints.");
                pc0_num = r2[0]*r2[1] - r2[0]*p12[1] - r2[1]*p12[0];
                this->r[0] = sqrt(r2[0]);
                this->r[1] = sqrt(r2[1]);
            }
            T pc0 = sign*sqrt(pc0_num/(r2[0]*p12[1]+r2[1]*p12[0]));
            pc1[0] = pc0*r[0]*p1[1]/r[1];
            pc1[1] = -pc0*r[1]*p1[0]/r[0];
            // centre point
            this->pc[0] = this->ca*pc1[0] - this->sa*pc1[1] + (start[0] + end[0])/2;
            this->pc[1] = this->sa*pc1[0] + this->ca*pc1[1] + (start[1] + end[1])/2;
            // start and end angles
            u[0] = (p1[0]-pc1[0])/r[0];
            u[1] = (p1[1]-pc1[1])/r[1];
            v[0] = -(p1[0]+pc1[0])/r[0];
            v[1] = -(p1[1]+pc1[1])/r[1];
            T lu = sqrt(pow(u[0],2) + pow(u[1],2));
            T lv = sqrt(pow(v[0],2) + pow(v[1],2));
            this->t_start = acos(u[0]/lu)*sgn<T>(u[1]);
            T dtheta = acos((u[0]*v[0] + u[1]*v[1])/lu/lv)*sgn<T>(u[0]*v[1]-u[1]*v[0]);
            if (sweep_flag && dtheta<0) {
                dtheta += M_2PI;
            } else if (!sweep_flag && dtheta>0) {
                dtheta -= M_2PI;
            }
            this->t_end = this->t_start + dtheta;
            // store full length
            this->_length = this->length(1);
        }
	
        /** \brief Get point coordinates for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return point coordinates.
         */
    	std::vector<T> point(const T t) const override {
            T theta = this->t_start + (this->t_end - this->t_start)*t;
            T x = this->r[0] * cos(theta), y = this->r[1] * sin(theta);
            std::vector<T> p(2);
            p[0] = x*this->ca - y*this->sa + this->pc[0];
            p[1] = y*this->ca + x*this->sa + this->pc[1];
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
            T theta = this->t_start + (this->t_end - this->t_start)*t;
            T x = -this->r[0]*sin(theta), y = this->r[1]*cos(theta);
            std::vector<T> dp(2);
            dp[0] = M_2PI*(x*this->ca - y*this->sa);
            dp[1] = M_2PI*(y*this->ca + x*this->sa);
            return dp;
        }

        /** \brief Get arc derivative for given relative position.
         * 
         *  This is used to compute arc length or position along segment.
         * 
         *  \param t: relative position between 0 and 1.
         *  \return value of arc derivative.
         */
        T arc(const T t) const override {
            T theta = this->t_start + (this->t_end - this->t_start)*t;
            T x = -this->r[0]*sin(theta), y = this->r[1]*cos(theta);
            return M_2PI*sqrt(x*x + y*y);
        }

        /** \brief Get coordinates of segment centre.
         *  \return centre point coordinates.
         */
    	std::vector<T> centre() const override {
            return this->pc;
        }

        /** \brief Get segment length for given relative position.
         *  \param t: relative position between 0 and 1.
         *  \return segment length.
         */
    	T length(const T t) const override {
            if (this->is_circle)
                return t*abs(this->t_end - this->t_start)*this->r[0];
            if (t==0)
                return 0;
            
            T theta = this->t_start + (this->t_end - this->t_start)*t;
            T rmax = std::max({this->r[0], this->r[1]});
            T rmin = std::min({this->r[0], this->r[1]});
            T rr = rmin/rmax;
            T k = sqrt(1-rr*rr);
            T Q = rmax * abs(elliptic_e<T>(theta, k, 1e-6) - elliptic_e<T>(this->t_start, k, 1e-6));
            
            PYG_LOG_V("Computed arc length from 0 to {} -> {}", t, Q);
            return Q;
        }

        /** \brief Get relative position for given length.
         *  \param l: sub-segment length.
         *  \return relative position between 0 and 1.
         */
    	T arg_at_length(const T l) const override {
            if (this->is_circle)
                return l/this->r[0]/abs(this->t_end - this->t_start);
            T rmax = std::max({this->r[0], this->r[1]});
            T rmin = std::min({this->r[0], this->r[1]});
            T rr = rmin/rmax;
            T k = sqrt(1-rr*rr);
            T l0 = elliptic_e<T>(this->t_start, k, 1e-6);
            T r = (inv_elliptic_e(l0 + l/rmax, k, 1e-6) - this->t_start)/(this->t_end - this->t_start);
            PYG_LOG_V("Minimized arc parameter for l={} -> {}", l, r);
            return r;
        }
    };
    
}
