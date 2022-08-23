/** \file wire.h
 *  \brief Header file for Wire classes.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "../types/path.h"
#include "shape3d.h"

namespace pygraver::render {

    using namespace pygraver::types;

    /** \brief A shape representing a wire in 3D.
     *
     *  This is used to draw paths with a given thickness.
     */
    class Wire : public Shape3D {
    protected:
        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        virtual double color_mapping_function(const double pos[3]) override;

    public:
        Wire() = default;

        /** \brief Default destructor. */
        ~Wire();

        /** \brief Constructor with path object.
         *  \param path: pointer to a path object.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        Wire(const std::shared_ptr<Path> path,
             const double diameter,
             const std::vector<uint8_t> & color,
             const unsigned int sides=4);

        /** \brief Set path.
         *  \param path: pointer to a path object.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        virtual void set_path(const std::shared_ptr<Path> path,
                              const double diameter,
                              const std::vector<uint8_t> & color,
                              const unsigned int sides=4);
    };


    class CylindricalWire : public Wire {
    protected:
         /** \brief radius of cylinder around which wire is drawn. */
        double cylinder_radius;

        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        double color_mapping_function(const double pos[3]) override;

    public:
        CylindricalWire() = default;

        /** \brief Constructor with path object.
         *  \param cylinder_radius: radius of cylinder around which wire is drawn.
         *  \param path: pointer to a path object.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        CylindricalWire(const double cylinder_radius,
                        const std::shared_ptr<Path> path,
                        const double diameter,
                        const std::vector<uint8_t> & color,
                        const unsigned int sides=4);

        /** \brief Set path.
         *  \param path: pointer to a path object.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        void set_path(const std::shared_ptr<Path> path,
                      const double diameter,
                      const std::vector<uint8_t> & color,
                      const unsigned int sides=4) override;
    };


    /** \brief A shape representing a bundle of wires in 3D.
     *
     *  This is used to draw multiple paths together.
     */
    class WireCollection : public Shape3D {
    protected:
        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        virtual double color_mapping_function(const double pos[3]) override;

        /** \brief Diameter of wires. */
        double diameter;

    public:
        WireCollection() = default;

        /** \brief Default destructor. */
        ~WireCollection();

        /** \brief Constructor with paths.
         *  \param paths: vector of pointers to path objects.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        WireCollection(const std::vector<std::shared_ptr<Path>> & paths,
                       const double diameter,
                       const std::vector<uint8_t> & color,
                       const unsigned int sides=4);

        /** \brief Set paths.
         *  \param paths: vector of pointers to path objects.
         *  \param diameter: wire diameter.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         *  \param sides: number of sides (>=4).
         */
        void set_paths(const std::vector<std::shared_ptr<Path>> & paths,
                       const double diameter,
                       const std::vector<uint8_t> & color,
                       const unsigned int sides=4);
        

        /** \brief Set path at given index.
         *  \param idx: path index.
         *  \param path: pointer to path object.
         *  \param sides: number of sides (>=4).
         */
        void set_path(const size_t idx,
                      const std::shared_ptr<Path> path,
                      const unsigned int sides=4);
    };


    /** \brief Wire collection in cylindrical coordinates. */
    class CylindricalWireCollection : public WireCollection {
    protected:
        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        double color_mapping_function(const double pos[3]) override;

    public:
        /** \brief Constructor with arguments.
         *  \param cylinder_radius: radius of cylinder around which wires are drawn.
         *  \param paths: list of pointers to paths.
         *  \param diameter: wire diameter.
         *  \param color: RGB or RGBA color.
         *  \param sides: number of sides (>=4).
         */
        CylindricalWireCollection(const double cylinder_radius,
                                  const std::vector<std::shared_ptr<Path>> & paths,
                                  const double diameter,
                                  const std::vector<uint8_t> & color,
                                  const unsigned int sides=4);
    };


    /** \fn void py_wire_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_wire_exports(py::module_ & mod);

}