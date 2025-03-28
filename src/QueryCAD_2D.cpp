#include "QueryCAD_2D.hpp"

QueryCAD2::QueryCAD2(const std::string& filename) {
    try {
        std::cout << "Opening STEP file: " << filename << std::endl;
        
        // Create a STEP reader
        STEPControl_Reader reader;
        
        // Read the STEP file
        IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());
        
        if (status != IFSelect_RetDone) {
            throw std::runtime_error("Failed to read STEP file");
        }
        
        // Transfer all roots
        reader.TransferRoots();
        
        // Get the shape
        TopoDS_Shape shape = reader.OneShape();
        
        // Check if the shape is valid
        if (shape.IsNull()) {
            throw std::runtime_error("No valid shape in the STEP file");
        }
        
        // Verify it's a 2D file:
        // 1. No solids
        TopExp_Explorer solidExplorer(shape, TopAbs_SOLID);
        if (solidExplorer.More()) {
            throw std::runtime_error("The STEP file contains 3D solids - not a 2D file");
        }
        
        // 2. Count faces
        int faceCount = 0;
        TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
        while (faceExplorer.More()) {
            faceCount++;
            if (faceCount > 1) {
                throw std::runtime_error("The STEP file contains multiple surfaces - not a valid 2D file");
            }
            m_face = TopoDS::Face(faceExplorer.Current());
            faceExplorer.Next();
        }
        
        if (faceCount == 0) {
            throw std::runtime_error("The STEP file contains no surfaces");
        }
        
        // 3. Extract wireframes
        TopExp_Explorer wireExplorer(m_face, TopAbs_WIRE);
        if (!wireExplorer.More()) {
            throw std::runtime_error("The STEP file contains no wireframes");
        }
        
        // Store all wireframes
        for (; wireExplorer.More(); wireExplorer.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(wireExplorer.Current());
            m_wires.push_back(wire);
        }
        
        std::cout << "Successfully loaded 2D STEP file with 1 surface and " 
                  << m_wires.size() << " wireframes" << std::endl;
                  
        m_shape = shape;
        m_isValid = true;
    }
    catch (const Standard_Failure& ex) {
        std::cerr << "OpenCASCADE exception: " << ex.GetMessageString() << std::endl;
        m_isValid = false;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        m_isValid = false;
    }
}

void QueryCAD2::get1DMesh(double h) {
    if (!m_isValid) {
        throw std::runtime_error("Invalid 2D CAD model");
    }

    int n = 0; // Global index offset for vertices
    for (const auto& wire : m_wires) {
        std::vector<size_t> wireVertexIndices; // Store indices of vertices in the current wire

        // Check if the wire is reversed
        bool isReversed = (wire.Orientation() == TopAbs_REVERSED);

        TopExp_Explorer edgeExplorer(wire, TopAbs_EDGE);
        for (; edgeExplorer.More(); edgeExplorer.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());

            // Adapt the edge to a curve
            BRepAdaptor_Curve curve(edge);

            // Discretize the curve into evenly spaced points
            GCPnts_UniformAbscissa discretizer(curve, h);
            if (!discretizer.IsDone()) {
                throw std::runtime_error("Failed to discretize edge");
            }

            // Collect points, first derivatives, and second derivatives from the discretizer
            std::vector<size_t> edgeVertexIndices; // Store indices of vertices for this edge
            for (int i = 1; i <= discretizer.NbPoints()-1; ++i) {
                double param = discretizer.Parameter(i);

                // Get the point on the curve
                gp_Pnt point = curve.Value(param);
                vertex v = {point.X(), point.Y(), point.Z()};
                vertices.push_back(v);

                // Get the first derivative
                gp_Vec d1;
                curve.D1(param, point, d1);
                vertex firstDerivative = {d1.X(), d1.Y(), d1.Z()};

                // Get the second derivative
                gp_Vec d2;
                curve.D2(param, point, d1, d2);
                vertex secondDerivative = {d2.X(), d2.Y(), d2.Z()};

                // Reverse the derivatives if the wire is reversed
                if (isReversed) {
                    firstDerivative = {-firstDerivative[0], -firstDerivative[1], -firstDerivative[2]};
                    secondDerivative = {-secondDerivative[0], -secondDerivative[1], -secondDerivative[2]};
                }

                // Store the derivatives
                D1.push_back(firstDerivative);
                D2.push_back(secondDerivative);

                // Store the vertex index
                edgeVertexIndices.push_back(n++);
            }

            // Create segments between consecutive points on the edge
            for (size_t i = 0; i < edgeVertexIndices.size() - 1; ++i) {
                segments.push_back({edgeVertexIndices[i], edgeVertexIndices[i + 1]});
            }

            // Add the edge's vertex indices to the wire's vertex indices
            wireVertexIndices.insert(wireVertexIndices.end(), edgeVertexIndices.begin(), edgeVertexIndices.end());
        }

        // Connect the last point of the last edge to the first point of the first edge in the wire
        if (!wireVertexIndices.empty()) {
            segments.push_back({wireVertexIndices.back(), wireVertexIndices.front()});
        }
    }
}

void QueryCAD2::writeVTK(const std::string& filename) {
    if (!m_isValid) {
        throw std::runtime_error("Invalid 2D CAD model");
    }

    get1DMesh();

    // Open the file for writing
    std::ofstream vtkFile(filename);
    if (!vtkFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // Write VTK header
    vtkFile << "# vtk DataFile Version 3.0\n";
    vtkFile << "1D Mesh generated by QueryCAD2\n";
    vtkFile << "ASCII\n";
    vtkFile << "DATASET POLYDATA\n";

    // Write points (vertices)
    vtkFile << "POINTS " << vertices.size() << " float\n";
    for (const auto& vertex : vertices) {
        vtkFile << vertex[0] << " " << vertex[1] << " " << vertex[2] << "\n";
    }

    // Write lines (segments)
    vtkFile << "LINES " << segments.size() << " " << segments.size() * 3 << "\n";
    for (size_t i = 0; i < segments.size(); ++i) {
        vtkFile << "2 " << segments[i][0] << " " << segments[i][1] << "\n";
    }

    // Write normals
    vtkFile << "POINT_DATA " << vertices.size() << "\n";
    vtkFile << "NORMALS Normals float\n";
    for (const auto& d1 : D1) {
        // Normalize the first derivative to compute the normal
        double magnitude = std::sqrt(d1[0] * d1[0] + d1[1] * d1[1] + d1[2] * d1[2]);
        if (magnitude > 0) {
            vtkFile << d1[1] / magnitude << " " << -d1[0] / magnitude << " " << d1[2] / magnitude << "\n";
        } else {
            // If the magnitude is zero, write a default normal (e.g., 0, 0, 1)
            vtkFile << "0 0 1\n";
        }
    }

    // Close the file
    vtkFile.close();

    std::cout << "VTK file written to: " << filename << std::endl;
}