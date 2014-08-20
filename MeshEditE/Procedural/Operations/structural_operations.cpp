//
//  structural_opeations.cpp
//  MeshEditE
//
//  Created by Francesco Usai on 08/08/14.
//  Copyright (c) 2014 J. Andreas Bærentzen. All rights reserved.
//

#include "structural_operations.h"

#include <GEL/CGLA/Vec3d.h>
#include <MeshEditE/Procedural/Helpers/geometric_properties.h>
#include <MeshEditE/Procedural/Operations/geometric_operations.h>
#include <MeshEditE/Procedural/Helpers/structural_helpers.h>
#include <MeshEditE/Procedural/Operations/Algorithms.h>
#include "polarize.h"

using namespace HMesh;
using namespace std;
using namespace CGLA;
using namespace Procedural::Geometry;
using namespace Procedural::Operations::Geometric;
using namespace Procedural::Structure;


namespace Procedural{
    namespace Operations{
        namespace Structural{

void add_branch ( HMesh::Manifold& m, HMesh::VertexID vid, int size, HMesh::VertexAttributeVector<int> &ring )
{
//    HMesh::VertexAttributeVector<int> ring( m.no_vertices(), 0 );
    HalfEdgeAttributeVector<EdgeInfo> edge_info = label_PAM_edges( m );
    
    // seleziona solo i punti   che non sono poli.
    //                          che non hanno vicini poli
    //                          che non hanno vicini selezionati
        
    map< HMesh::VertexID, CGLA::Vec3d > vert_pos;
    if( !is_pole( m, vid ) && !is_2_neighbor_of_pole( m,vid ))
    {
        Walker w  = m.walker( vid );
        ring[vid] = 1;
        
        for ( ; !w.full_circle(); w=w.circulate_vertex_ccw() )
        {
            if ( edge_info[w.halfedge()].is_rib() )
            {
                        ring[w.vertex()]    = 1;
                Walker  side_w              = w;
                for (int sides = 1; sides < size; ++sides )
                {
                    side_w = side_w.next().opp().next();
                    ring[side_w.vertex()] = 1;
                }
            }
            vert_pos[w.vertex()] = m.pos( w.vertex( ));
        }
        Vec3d centroid(0);
        polar_add_branch( m, ring );
    }
}
            
void cut_branch( Manifold& m, HalfEdgeID h, VertexID pole )
{
    //  0) find the ring of vertices
    vector< VertexID > vertices;
    
    // 1) save a vector from one of the pole's neighbors to the pole ( or I need all of them? )
    // need to save anche which of the vertices of the slitted ring has to be used as reference
    Vec3d       barycenter      = ring_barycenter( m, h, vertices );
    Vec3d       pole_dir        = m.pos(pole) - barycenter;
    // 2) slit that ring of vertices
    VertexAttributeVector<int> selection(m.no_vertices(), 0);
    for( VertexID v : vertices ) { selection[v] = 1; }
    HalfEdgeID hole_edge = m.slit_edges(selection);
    // 3) delete all the faces of the slitted component
    // this should cut also the sub_branches!!!!!!!!!!!! ( NOT SO EASY! )
    vector< VertexID > v_to_delete;
    v_to_delete.push_back(pole);
    Walker  w       = m.walker(pole);
    bool    done    = false;
    w       = m.walker( w.next().halfedge( ));
    
    while( !done )
    {
        // this just works for a single branch with no sub-branches
        /* starting from the pole
         for each outgoint branch
         mark the vertex to be deleted and move the walker to next.opp.next
         // each walker stops when reaches a vertex inside "vertices" ( put them in a SET! )
         */
        
        
        
        
        //        done = ( w.opp().face() == InvalidFaceID);
        //        for (; !w.full_circle(); w = w.next().opp().next()) {
        //            v_to_delete.push_back(w.vertex());
        //        }
        //        if (!done)
        //        {
        //            w = m.walker( w.opp().next().next().halfedge( ));
        //        }
    }
    for ( auto vtd : v_to_delete )
    {
        cout << "deleting vertex " << vtd << endl;
        m.remove_vertex(vtd);
    }
    // 4) rebuild the pole
    FaceID      f           = m.close_hole( hole_edge );
    VertexID    new_pole    = m.split_face_by_vertex( f );
    Vec3d       f_centroid  = ring_barycenter( m, hole_edge );
    m.pos( new_pole )       = f_centroid + pole_dir;
}

// this works only for cutting branches that are not part of a loop
// NEED TO KNOW how to understand if the ring is part of a loop
// ALSO need to understend in which direction should the walker move in order to reach the pole
void cut_branch ( HMesh::Manifold& m, HMesh::HalfEdgeID h )
{
    // if not loop OR not between two joints
    //              find the right pole
    //              call cut_branch( m, h, pole )
    // else cut creating two poles with a simple policy
}
            
void cut_branch ( Manifold& m, VertexID v, HalfEdgeAttributeVector<EdgeInfo> edge_info )
{
    vector< VertexID > vertices;
    HalfEdgeID he = get_ring_vertices( m, v, edge_info, vertices, false );
    // early termination if the ring is not found
    if ( vertices.size() == 0 ) return;
    // save two references to the two sides ( from v )
    Walker w = m.walker(he);
    HalfEdgeID rib1 = w.next().next().halfedge();
    HalfEdgeID rib2 = w.opp().next().next().halfedge();
    for( VertexID vid : vertices ) { m.remove_vertex(vid); }
    assert( m.walker(rib1).face() == InvalidFaceID );
    assert( m.walker(rib2).face() == InvalidFaceID );
    FaceID f1 = m.close_hole(rib1);
    FaceID f2 = m.close_hole(rib2);
    m.split_face_by_vertex(f1);
    m.split_face_by_vertex(f2);
    
    // NEED TO DECIDE WHAT TO DO IN SOME CASES ( avoid separate components, etc )
}
            
void remove_branch ( HMesh::Manifold& m, HMesh::VertexID pole, HMesh::HalfEdgeAttributeVector<EdgeInfo> edge_info )
{
    // check if m.walker(pole).next().halfedge è di tipo junction
    if ( !is_pole( m, pole )) return;
    
    LabelJunctions( m, edge_info );
    
    if ( !edge_info[ m.walker(pole).next().opp().halfedge()].is_junction() ) return;

    typedef pair< HalfEdgeID, HalfEdgeID > ToStitchPair;
    vector< ToStitchPair > to_stitch;

    // save the pair of edges that must be stitched together
    Walker w = m.walker(pole);
    for( ; !w.full_circle() && !is_singularity(m, w.vertex()); w = w.circulate_vertex_ccw());
    w = w.next();

    assert( edge_info[w.halfedge()].is_junction( ));
    // remove pole
    m.remove_vertex( pole );
    Walker wback = w.prev();
        
    assert( is_singularity(m, wback.vertex( )));
    do
    {
        to_stitch.push_back( make_pair( w.halfedge(), wback.halfedge( )));
        w       = w.next();
        wback   = wback.prev();
        assert( w.face()     == InvalidFaceID );
        assert( wback.face() == InvalidFaceID );
        
    } while( valency( m, w.vertex()) != 5 );
//    to_stitch.push_back( make_pair( w.halfedge(), wback.halfedge( )));
    
    // stich pair of edges ( note that at the vertices with valence 6 you should stich
    // togheter the junction outgoing edge and its prev ( around the hole )
    for( ToStitchPair tsp : to_stitch )
    {
        // debug
        VertexID vfirst  = m.walker(tsp.first).vertex();
        VertexID vsecond = m.walker(tsp.second).vertex();
        
        
        m.stitch_boundary_edges( tsp.first, tsp.second );
        cout << " is first? # "
             << m.in_use(vfirst) << " or is second? # "
             << m.in_use(vsecond) << endl;;
        
        //debug
        
    }
    
    // trovare un modo per risistemare le posizioni dei vertici
    Procedural::Operations::Algorithms::along_spines(m, edge_info);
}
            
            
void glue_poles ( Manifold& m, VertexID pole1, VertexID pole2 )
{
    // early termination in case the input vertices are not poles
    if( !is_pole( m, pole1 )) return;
    if( !is_pole( m, pole2 )) return;
    // get valency of the input vertices
    int         pole1_valency  = valency( m, pole1 ),
                pole2_valency  = valency( m, pole2 );
    // choose the one that has lowest valency ( say low_val_pole )
    VertexID    low_val_pole   = pole1_valency < pole1_valency ? pole1 : pole2;
    VertexID    hi_val_pole    = low_val_pole == pole1         ? pole2 : pole1;
    int         lvpole_valency = low_val_pole == pole1         ? pole1_valency : pole2_valency;
    int         hvpole_valency = low_val_pole == pole1         ? pole2_valency : pole1_valency;
    int         diff           = pole2_valency - pole1_valency;
    // subdivide it for a number of times equal to the difference in valence
    vector< VertexID >      vs;
    vector< HalfEdgeID >    hes;
    HalfEdgeID starter = m.walker( low_val_pole ).next().halfedge();
    ring_vertices_and_halfedges( m, starter, vs, hes );
    // could happen that the difference in valence is greater than the low_val_pole valency
    bool done = false;
    do
    {
        int limit = diff > hes.size() ? hes.size() : diff;
        diff -= limit;
        int pace = hes.size() / limit;
        done = ( diff <= 0 );

        for( int i = 0; i < limit; ++i )
        {
            
        }
        
    } while( !done );
    

    
    
    // find adequate matchings between edges
    // delete poles
    // stich_boundary_edges between edges
    
}
    
   
            
}}}
