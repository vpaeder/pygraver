/** \file util.h
 *  \brief Common functions for SVG parsing routines.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <libxml/tree.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
#ifndef M_2PI
#define M_2PI 2*M_PI
#endif
#ifndef M_PI_2
#define M_PI_2 M_PI/2
#endif

/** \namespace pygraver::svg
 *  \brief Contains a simple SVG file parser.
 */
namespace pygraver::svg {
      /** \brief Splits a string at given token.
       *  \param str_to_split : string to split
       *  \param tokens : token to use as separator.
       *  \returns a string vector containing split strings.
       */
      static std::vector<std::string> split_string(const std::string & str_to_split, const std::string & tokens) {
            size_t pos = 0, last = 0;
            std::vector<std::string> split_vec;
            for (pos = 0; pos < str_to_split.size(); pos++) {
                if (tokens.find_first_of(str_to_split[pos]) != std::string::npos) {
                    split_vec.emplace_back(str_to_split.substr(last, pos-last));
                    last = pos+1;
                }
            }
            if (last < pos)
                split_vec.emplace_back(str_to_split.substr(last, pos-last));
            return split_vec;
        }

      /** \brief Parse a point definition string in the form "x,y".
       *  \tparam T: numeric type.
       *  \param point_str: string to parse.
       *  \returns a vector with point coordinate values.
       */
      template <typename T> std::vector<T> parse_point(const std::string & point_str) {
          auto split_vec = split_string(point_str, ",");
          std::vector<T> point(2);
          point[0] = std::stod(split_vec[0]);
          point[1] = std::stod(split_vec[1]);
          return point;
      }

      /** \brief Parse a number.
       *  \tparam T: number type.
       *  \param val_str: string to parse.
       *  \returns a number.
       */
      template <typename T> T parse_value(const std::string & val_str) {
          return std::stod(val_str);
      }

      /** \fn template <typename T> inline void vec2_add(std::vector<T> & p, const std::vector<T> & q)
       *  \brief Add a 2-point vector to another.
       *  \param p: vector to add to.
       *  \param q: vector to add.
       */
      template <typename T> inline void vec2_add(std::vector<T> & p, const std::vector<T> & q) {
          p[0] += q[0];
          p[1] += q[1];
      }

      /** \fn template <typename T> inline void vec2_sub(std::vector<T> & p, const std::vector<T> & q)
       *  \brief Subtract a 2-point vector from another.
       *  \param p: vector to subtract from.
       *  \param q: vector to subtract.
       */
      template <typename T> inline void vec2_sub(std::vector<T> & p, const std::vector<T> & q) {
          p[0] -= q[0];
          p[1] -= q[1];
      }

      /** \fn template <typename T> inline std::vector<T> vec2_pow(const std::vector<T> & p, const double e)
       *  \brief Compute the power of the components of a vector.
       *  \param p: 2-point vector.
       *  \param e: exponent.
       *  \returns vector with components [pow(p[0],e), pow[p[1],e]]
       */
      template <typename T> inline std::vector<T> vec2_pow(const std::vector<T> & p, const double e) {
          std::vector<T> pe(2);
          pe[0] = pow(p[0], e);
          pe[1] = pow(p[1], e);
          return pe;
      }

      /** \fn template <typename T> inline int sgn(const T val)
       *  \brief Compute sign of a value.
       *  \param val: value.
       *  \returns +1 for positive, -1 for negative, 1 for null.
       */
      template <typename T> inline int sgn(const T val) {
          auto sgn = (T(0) < val) - (val < T(0));
          return sgn!=0 ? sgn : 1;
      }

      /** \brief Extract a property from a XML node.
       *  \param node: pointer to a XML node.
       *  \param str: property name.
       *  \returns property value as string.
       */
      inline std::string get_prop(const xmlNodePtr node, const std::string & str) {
  		    return std::string(reinterpret_cast<const char*>(xmlGetProp(node, (const xmlChar *)str.c_str())));
      }

      /** \brief Trims a string from white spaces on the right.
       *  \param s : string to trim
       */
      static void rtrim(std::string &s) {
          s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
              return !std::isspace(ch);
          }).base(), s.end());
      }
      
      /** \brief Trims a string from white spaces on the left.
       *  \param s : string to trim
       */
      static void ltrim(std::string &s) {
          s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
              return !std::isspace(ch);
          }));
      }

      /** \brief Extract a number from a XML node property.
       *  \param node: pointer to a XML node.
       *  \param str: property name.
       *  \param rel: for percent values, this is the value to relate to.
       *  \param ft: for em values, this is the font size.
       *  \returns converted value.
       */
  	  template <typename T> T get_number(const xmlNodePtr node, const std::string & str, const T rel = 0, const T ft = 16) {
          auto val_str = get_prop(node, str);
          rtrim(val_str);
          ltrim(val_str);
          size_t upos;
          T val = std::stod(val_str, &upos);
          if (upos == val_str.size()) return val;
          auto units = val_str.substr(upos);
          if (units == "px") return val;
          if (units == "%") return val*rel/100;
          if (units == "em") return val*ft;
          return val;
  	  }

    
}
