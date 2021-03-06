//
//  geometric_properties.h
//  MeshEditE
//
//  Created by Francesco Usai on 08/08/14.
//  Copyright (c) 2014 J. Andreas Bærentzen. All rights reserved.
//

#ifndef __MeshEditE__geometric_properties__
#define __MeshEditE__geometric_properties__

#include <iostream>
#include <GEL/HMesh/Manifold.h>
#include <GEL/CGLA/Vec3d.h>
#include "polarize.h"
#include <set>
#include <cmath>


namespace Procedural{
    namespace Geometry{

//typedef std::pair< int, double > DistanceMetrics;
typedef int DistanceMetrics;

CGLA::Vec3d     face_normal( HMesh::Manifold& m, HMesh::FaceID f );
CGLA::Vec3d     vertex_normal( HMesh::Manifold& m, HMesh::VertexID v );

CGLA::Vec3d     ring_barycenter( HMesh::Manifold& m, HMesh::HalfEdgeID h );
CGLA::Vec3d     ring_barycenter( HMesh::Manifold& m, HMesh::HalfEdgeID h, std::vector< HMesh::VertexID > &vertices );
CGLA::Vec3d     ring_barycenter( HMesh::Manifold& m, HMesh::HalfEdgeID h, std::vector< HMesh::VertexID > &vertices,
                                 std::vector< HMesh::HalfEdgeID > &halfedges );

double          ring_mean_radius( HMesh::Manifold& m, HMesh::HalfEdgeID h, std::map< HMesh::VertexID, double> &radii );
double          ring_mean_radius ( HMesh::Manifold& m, HMesh::HalfEdgeID h );

void            ring_vertices_and_halfedges ( HMesh::Manifold& m, HMesh::HalfEdgeID h, std::vector< HMesh::VertexID > &vertices,
                                              std::vector< HMesh::HalfEdgeID > &halfedges );

int             valence( HMesh::Manifold& m, HMesh::VertexID v );
bool            is_singularity( HMesh::Manifold& m, HMesh::VertexID v );
bool            is_neighbor_of_pole( HMesh::Manifold& m, HMesh::VertexID v);
bool            is_2_neighbor_of_pole( HMesh::Manifold& m, HMesh::VertexID v );

// must have calld label junction on edge_info
void            distance_from_poles( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info,
                                     HMesh::VertexAttributeVector<DistanceMetrics> &distances, bool debug_colors = true );

void            distance_from_junctions( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info,
                                         HMesh::VertexAttributeVector<DistanceMetrics> &distances, bool debug_colors = true );

void            distance_from_poles_and_junctions( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info,
                                                   HMesh::VertexAttributeVector<DistanceMetrics> &distances, bool debug_colors = true );
        
void            dihedral_angles( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info,
                                 HMesh::VertexAttributeVector<double> &angles, EdgeType edge_type );
void            dihedral_angles( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info,
                                 HMesh::VertexAttributeVector<double> &angles );

double          get_angle( CGLA::Vec3d l, CGLA::Vec3d r );
double          dihedral_angle( CGLA::Vec3d a, CGLA::Vec3d b, CGLA::Vec3d c, CGLA::Vec3d d );
double          edge_length( HMesh::Manifold& m, HMesh::HalfEdgeAttributeVector<double> lengths );
double          mean_length_of_outoing_he( HMesh::Manifold& m, HMesh::VertexID vertex );

void            bsphere( HMesh::Manifold& m, CGLA::Vec3d& centroid, double& radius );
void            bsphere( HMesh::Manifold& m, std::vector<HMesh::VertexID> &vertices, CGLA::Vec3d& centroid, double& radius );
void            bsphere( HMesh::Manifold& m, const std::set<HMesh::VertexID> &vertices, CGLA::Vec3d& centroid, double& radius );
void            bsphere( std::vector< CGLA::Vec3d > &points, CGLA::Vec3d& centroid, double& radius );

CGLA::Vec3d     vec_from_edge( const HMesh::Manifold& m, HMesh::HalfEdgeID he );
CGLA::Vec3d     vec_from_verts( const HMesh::Manifold& m, HMesh::VertexID v1, HMesh::VertexID v2  );
        

inline bool opposite_directions( const CGLA::Vec3d& v1, const CGLA::Vec3d& v2) { return (  CGLA::dot(v1, v2) < -0.00001); }

}}


#endif /* defined(__MeshEditE__geometric_properties__) */
