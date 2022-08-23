/** \file common.h
 *  \brief Common functions for data types.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <pybind11/pybind11.h>
#include <tuple>

namespace py = pybind11;
namespace gg = geos::geom;

/** \brief Contains definitions of data types and associated functions. */
namespace pygraver::types {
    /** \brief Global GEOS geometry factory object.
     * 
     * It is initialized when the first pygraver object is created
     * and remains in memory until pygraver library is unloaded.
     */
    static gg::GeometryFactory::Ptr gfactory;
    
    /** \brief Create the global GEOS geometry factory object, if necessary. */
    static void create_geometry_factory() {
        if (!gfactory.get()) {
            auto pm = std::make_unique<gg::PrecisionModel>();
            gfactory = gg::GeometryFactory::create(pm.get());
        }
    }

     /** \brief Compares two values within some numerical precision.
      *  \tparam T : type of values to compare.
      *  \param x : first value.
      *  \param y : second value.
      *  \param ulp : number of digits to take into account.
      *  \returns true if the two numbers are equal within the requested numerical precision.
      */
     template<typename T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type almost_equal(const T & x, const T & y, const int ulp) {
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::abs(x-y) <= std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp * 1e2
        // unless the result is subnormal
               || std::abs(x-y) < std::numeric_limits<T>::min();
     }

   /** \brief Adds two objects of the given type. This function is necessary to create the Python function __add__.
    *  \tparam T : type of the objects.
    *  \param p : first object.
    *  \param q : second object.
    *  \returns the sum of the two input objects (p+q).
    */
    template <typename T> static auto add_objects(std::shared_ptr<const T> p, std::shared_ptr<const T> q) {
        return p+q;
    }

   /** \brief Adds two objects of different types. This function is necessary to create the Python function __add__.
    *  \tparam T : type of first object.
    *  \tparam V : type of second object.
    *  \param p : first object.
    *  \param q : second object.
    *  \returns the sum of the two input objects (p+q).
    */
    template <typename T, typename V> static auto add_objects(std::shared_ptr<const T> p, std::shared_ptr<const V> q) {
        return p+q;
    }

   /** \brief Subtracts two objects of the given type. This function is necessary to create the Python function __sub__.
    *  \tparam T : type of the objects.
    *  \param p : first object.
    *  \param q : second object.
    *  \returns the difference of the two input objects (p-q).
    */
    template <typename T> static auto sub_objects(std::shared_ptr<const T> p, std::shared_ptr<const T> q) {
        return p-q;
    }
    
   /** \brief Negates an object of the given type. This function is necessary to create the Python function __neg__.
    *  \tparam T : type of the object.
    *  \param p : object to negate.
    *  \returns the negative of the input object.
    */
    template <typename T> static auto neg_object(std::shared_ptr<const T> p) {
        return -p;
    }
    
   /** \brief Right-multiplies an object of the given type. This function is necessary to create the Python function __mul__.
    *  \tparam T : type of first object.
    *  \tparam V : type of second object.
    *  \param p : object to multiply.
    *  \param q : value to multiply with.
    *  \returns the multiplication of the input object by the given factor.
    */
    template <typename T, typename V> static auto mul_object(std::shared_ptr<const T> p, const V q) {
        return p*q;
    }    

   /** \brief Left-multiplies an object of the given type. This function is necessary to create the Python function __mul__.
    *  \tparam V : type of first object.
    *  \tparam T : type of second object.
    *  \param q : value to multiply with.
    *  \param p : object to multiply.
    *  \returns the multiplication of the input object by the given factor.
    */
    template <typename T, typename V> static auto mul_object_reverse(const V q, std::shared_ptr<const T> p) {
        return p*q;
    }    

   /** \brief Tell if objects are equal.
    *  \tparam T : type of the object.
    *  \param p : first object.
    *  \param q : second object.
    *  \returns true if objects are equal, false otherwise.
    */
    template <typename T> static bool eq_objects(std::shared_ptr<const T> p, std::shared_ptr<const T> q) {
        return p == q;
    }

   /** \brief Tell if objects are not equal.
    *  \tparam T : type of the object.
    *  \param p : first object.
    *  \param q : second object.
    *  \returns true if objects are not equal, false otherwise.
    */
    template <typename T> static bool neq_objects(std::shared_ptr<const T> p, std::shared_ptr<const T> q) {
        return p != q;
    }

   /** \brief Computes the angle modulo 360°.
    *  \param x : input angle.
    *  \returns the input value modulo 360.
    */
    inline double angle_norm (double x) {
        x = fmod(x + 180, 360);
        if (x < 0)
            x += 360;
        return x - 180;
    }
    
   /** \brief Computes the angle modulo 2*pi.
    *  \param x : input angle.
    *  \returns the input value modulo 2*pi.
    */
    inline double angle_norm_rad (double x) {
        x = fmod(x + M_PI, 2*M_PI);
        if (x < 0)
            x += 2*M_PI;
        return x - M_PI;
    }

   /** \brief Pack a vector into a Python list object.
    *  \tparam T: type of objects in vector.
    *  \param v: vector.
    *  \returns Python list object.
    */
    template <typename T> py::list py_pack(const std::vector<const T> & v) {
        py::list res;
        for (auto & c: v)
            res.append(c);
        return res;
    }

   /** \brief Convert a Python slice to positive indices.
    *  \param slice: Python slice object.
    *  \param arr_sz: size of array to access.
    *  \returns a tuple containing slice start, slice stop and slice step values.
    */
    static std::tuple<size_t, size_t, size_t> convert_slice(const pybind11::slice & slice, const size_t arr_sz) {
        ssize_t start, stop, step;
        PySlice_Unpack(slice.ptr(), &start, &stop, &step);
        if (start<0)
            start = arr_sz - start;
        if (stop<0)
            stop = arr_sz - stop;
        if (start > stop && step < 0)
            return {stop, start, -step};
        if (stop > start && step > 0)
            return {start, stop, step};

        PyErr_SetString(PyExc_ValueError, "Invalid slice.");
        throw py::error_already_set();
        return {0,0,0};
    }

}
