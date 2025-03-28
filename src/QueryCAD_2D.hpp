#ifndef __QueryCAD_2D_HPP__
#define __QueryCAD_2D_HPP__

#include <string>
#include <vector>
#include <iostream>
#include <array>

// OpenCASCADE headers
#include <STEPControl_Reader.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Solid.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Standard_Failure.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <TopExp.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>

typedef std::array<double, 3> vertex;
typedef std::vector<std::array<size_t, 2>> Segments;

class QueryCAD2 {
public:
    QueryCAD2(const std::string& filename);
    
    bool isValid() const { return m_isValid; }

    void get1DMesh(double h = 0.01);
    
    void writeVTK(const std::string& filename);

private:
    bool m_isValid = false;
    TopoDS_Shape m_shape;
    TopoDS_Face m_face;
    std::vector<TopoDS_Wire> m_wires;

    Segments segments;
    std::vector<vertex> vertices;
    std::vector<vertex> D1;
    std::vector<vertex> D2;
};

#endif // __QueryCAD_2D_HPP__