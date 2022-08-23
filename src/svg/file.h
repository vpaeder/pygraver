/** \file file.h
 *  \brief Header file for File class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <string>
#include <libxml/tree.h>

#include "../types/point.h"
#include "../types/path.h"
#include "shape.h"

/** \brief Number type for SVG parser classes. */
using number_t = double;

namespace pygraver::svg {
    
    /** \brief SVG file class.
     * 
     *  This is a simple SVG file loader. It parses shapes and can
     *  produce rasterized curves.
     */
    class File {
    private:
      /** \brief Pointer to root XML node. */
      xmlNodePtr root = nullptr;
      
      /** \brief Drawing centre point. */
      std::shared_ptr<types::Point> centre;

      /** \brief Check if a document is opened. */
      void check_opened() const;

      /** \brief Parse XML document.
       *  \param doc: pointer to XML document root.
       */
	    void parse_document(const xmlDocPtr doc);

      /** \brief Get XML node for given layer name.
       *  
       *  This searches for a \<g> element with id = given name.
       * 
       *  \param name: layer name.
       *  \returns a pointer to the XML node, or nullptr if none was found.
       */
      xmlNodePtr get_layer(const std::string & name) const;

      /** \brief Get transform attribute from given XML node.
       * 
       *  \param node: pointer to XML node.
       *  \returns a string containing transform attribute's content.
       */
      std::string get_transform(xmlNodePtr node) const;

      /** \brief Get shapes contained in given XML node.
       * 
       *  This searches for SVG shapes inside the XML node.
       * 
       *  \param node: pointer to XML node.
       *  \returns a collection of shape objects.
       */
      std::vector<std::unique_ptr<Shape<number_t>>> get_shapes(xmlNodePtr node) const;

    public:
      /** \brief Default constructor. */
      File() = default;

      /** \brief Constructor with file name argument.
       *  \param file_name: name of file to read from.
       */
      File(const std::string & file_name);

      /** \brief Open given file.
       *  \param file_name: name of file to read from.
       */
      void open(const std::string & file_name);

      /** \brief Parse SVG from buffer.
       *  \param buffer: buffer containing SVG file to parse.
       */
      void from_memory(const std::string & buffer);

      /** \brief Destructor. */
      ~File();
      
      /** \brief Get shapes in given layer and convert them to paths.
       *  \param layer_name: layer name.
       *  \param dl: interpolation step size.
       *  \returns a collection of Path objects.
       */
      std::vector<std::shared_ptr<types::Path>> get_paths(const std::string & layer_name, const number_t dl) const;

      /** \brief Get shapes contained in given XML node.
       * 
       *  This searches for SVG shapes inside the XML node.
       * 
       *  \param layer_name: layer name.
       *  \returns a collection of shape objects.
       */
      std::vector<std::unique_ptr<Shape<number_t>>> get_shapes(const std::string & layer_name) const;

      /** \brief Get centre points of objects in given layer.
       *  
       *  This is used for instance to produce drill patterns. 
       * 
       *  \param layer_name: layer name.
       *  \returns a collection of Point objects.
       */
      std::vector<std::shared_ptr<types::Point>> get_points(const std::string & layer_name) const;

      /** \brief Get drawing size.
       *  \returns a 4-point vector with viewbox data: (x, y, width, height)
       */
      std::vector<number_t> get_size() const;
      
    };    
}
