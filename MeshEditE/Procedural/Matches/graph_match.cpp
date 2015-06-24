//
//  graph_match.cpp
//  MeshEditE
//
//  Created by Francesco Usai on 10/10/14.
//  Copyright (c) 2014 J. Andreas Bærentzen. All rights reserved.
//

#include "graph_match.h"
#include <MeshEditE/Procedural/Helpers/geometric_properties.h>

//#include "geometric_properties.h"
using namespace std;
using namespace HMesh;

namespace Procedural{
    namespace GraphMatch{


void graphStruct_difference( GraphStruct &g1, GraphStruct &g2, GraphStruct &g )
{
    // they should have the same node set and arc set
    // will check only the number of nodes ( assumptions can be made accordingly to the constructor )
    assert( g1.nodes.size() == g1.nodes.size() );
    size_t no_nodes = g1.nodes.size();
    g = GraphStruct( no_nodes );
    
    for( GraphEdge e : g.arcs )
    {
        g.setCost( e, g1.getCost( e ) - g2.getCost( e ));
    }
}


GraphEdge build_edge( GraphNode n1, GraphNode n2 )
{
    assert( n1 != n2 );
    if( n1 < n2 ) return std::make_pair( n1, n2 );
    else          return std::make_pair( n2, n1 );
}

// fills the given graph with costs associated to each arc
void fill_graph ( Manifold &m, vector<HMesh::VertexID> &vs, GraphStruct &g, ManifoldToGraph &mtg )
{
    vector< CGLA::Vec3d > pos, normals;
    for( VertexID v : vs )
    {
        assert( v != InvalidVertexID );
        CGLA::Vec3d p = m.pos(v),
                    n = Procedural::Geometry::vertex_normal( m, v );
        pos.push_back( p);
        normals.push_back( n );
    }
    // calculate costs save them into the graph
    for( GraphEdge e : g.arcs )
    {
        VertexID v1 = mtg.getVertexId( e.first ),
                 v2 = mtg.getVertexId( e.second );
        CGLA::Vec3d n1 = Procedural::Geometry::vertex_normal( m, v1 ),
                    n2 = Procedural::Geometry::vertex_normal( m, v2 );
        n1.normalize();
        n2.normalize();
#warning consider using squared length
        double distance = ( m.pos( v1 ) - m.pos( v2 )).length();
        double angle    = Geometry::get_angle( n1, n2 );
        
        assert( distance > 0 );
        assert( !isnan( angle ));
        g.setCost( e, make_pair( distance, angle ));
    }
}
        
void normalize_costs( GraphStruct &g1, GraphStruct g2 ){
    double max_distance = 0.0;
    // get the maximum distance between g1 and g2
    for( GraphEdge e : g1.arcs )
    {
        double current_distance = g1.getCost(e).first;
        if( current_distance > max_distance ) { max_distance = current_distance; }
    }
    for( GraphEdge e : g2.arcs )
    {
        double current_distance = g1.getCost(e).first;
        if( current_distance > max_distance ) { max_distance = current_distance; }
    }
    
    // normalize edge cost values
    for( GraphEdge e : g1.arcs )
    {
        g1.setCost(e, std::make_pair( g1.getCost(e).first / max_distance, g1.getCost(e).second / M_PI ));
#warning here I need a well thoughed assert
    }
    for( GraphEdge e : g2.arcs )
    {
        g2.setCost(e, std::make_pair( g2.getCost(e).first / max_distance, g2.getCost(e).second / M_PI ));
#warning here I need a well thoughed assert
    }
}


EdgeCost get_best_subset( Manifold &m,  vector<Match> &proposed, vector< Match > &selected, size_t target )
{
    vector<VertexID> host_vertices, module_poles;
    for( const auto& pole_and_vertex : proposed )
    {
        module_poles.push_back(  pole_and_vertex.first);
        host_vertices.push_back( pole_and_vertex.second );
    }
    
    EdgeCost c = get_best_subset(m, host_vertices, m, module_poles, selected, target );
    return c;
}
        

/// taking as input the host mesh and the module mesh, with the respective
/// vertices that are candidates for glueing. Taking also the target number of glueings,
/// it returns the matches that minimize the cost of glueing them.
EdgeCost get_best_subset( Manifold &host,   vector< VertexID > &host_candidates,
                          Manifold &module, vector< VertexID > &poles,
                          vector< Match > &selected, size_t target )
{
    assert( host_candidates.size() == poles.size( ));
    assert( target <= host_candidates.size());
    
    size_t no_nodes = host_candidates.size();
    EdgeCost cost_sum = make_pair( 0.0, 0.0 );
    GraphStruct gm( no_nodes );
    GraphStruct gh( no_nodes );
    GraphStruct g( 0 );
    ManifoldToGraph mtg_module( poles );
    ManifoldToGraph mtg_host( host_candidates );

    // fill graph costs for module
    fill_graph( module, poles, gm, mtg_module );
    // fill graph costs for host
    fill_graph( host, host_candidates, gh, mtg_host );
    // build difference graph
    graphStruct_difference( gm, gh, g );

    // iterate removing until the target is found
    size_t iterations = no_nodes - target;
    for( size_t i = 0; i < iterations; ++i )
    {
        map< GraphNode, EdgeCost > total_cost;
        // per ogni vertice
        for( size_t idx : g.nodes )
        {
            if( g.exists( idx ))
            {
                vector< GraphEdge > star;
                // prendere la sua star
                g.getStar( idx, star );
                total_cost[idx] = make_pair( 0.0, 0.0 );
                // calcolare la somma dei costi della star
                for( GraphEdge e : star )
                {
                    total_cost[idx] = total_cost[idx] + g.getCost( e );
                }
            }
        }
        
        EdgeCost    max_cost = (*total_cost.begin()).second;
        GraphNode   choosen  = (*total_cost.begin()).first;
        
        // scegliere il vertice con il costo maggiore della star
        for( const auto& star_cost : total_cost )
        {
            if( star_cost.second > max_cost )
            {
                max_cost = star_cost.second;
                choosen  = star_cost.first;
            }
        }
        // rimuoverlo dal grafo
        g.RemoveNode( choosen );
    }
    // save the matches
    for( GraphNode index : g.nodes )
    {
        if( g.exists( index ))
        {
            cost_sum = cost_sum + getStarTotalCost( g, index );
            selected.push_back( make_pair( mtg_module.getVertexId( index ), mtg_host.getVertexId( index )));
        }
    }
    return cost_sum;
}
  
        
EdgeCost getStarTotalCost( GraphStruct &g, GraphNode n )
{
    vector< GraphEdge > star;
    EdgeCost cost = make_pair( 0.0, 0.0 );
    // prendere la sua star
    g.getStar( n, star );
    // calcolare la somma dei costi della star
    for( GraphEdge e : star )
    {
        cost = cost + g.getCost( e );
    }
    return cost;
}
        
        
GraphNode remove_most_expensive_node( GraphStruct &g )
{
    map< GraphNode, EdgeCost > total_cost;
    // per ogni vertice
    for( size_t idx : g.nodes )
    {
        if( g.exists( idx ))
        {
            total_cost[idx] = getStarTotalCost( g, idx );
        }
    }
    
    EdgeCost    max_cost = (*total_cost.begin()).second;
    GraphNode   choosen  = (*total_cost.begin()).first;
    
    // scegliere il vertice con il costo maggiore della star
    for( const auto& star_cost : total_cost )
    {
        if( star_cost.second > max_cost )
        {
            max_cost = star_cost.second;
            choosen  = star_cost.first;
        }
    }
    // rimuoverlo dal grafo
    g.RemoveNode( choosen );
    return choosen;
}
        
        
// UTILITIES
std::ostream& graph_print (std::ostream &out, EdgeCost cost)
{
    out << "(" << cost.first << ", " << cost.second << ")";
    return out;
}
   
        
std::ostream& graph_print (std::ostream &out, GraphNode &node)
{
    out << "[" << (long)node << "]";
    return out;
}

        
std::ostream& graph_print (std::ostream &out, GraphEdge &edge)
{
    out << "[" << (long)edge.first << ", " << (long)edge.second << "]";
    return out;
}
        
        
std::ostream& operator<< (std::ostream &out, GraphStruct &g)
{
        out << " the graph has : " << g.no_nodes() << " nodes and " << g.arcs.size() << " edges " << std::endl;
        out << "the nodes are :" << std::endl;
        for( GraphNode n : g.nodes )
        {
            if(g.exists(n))
            {
                graph_print( out, n )  << std::endl;
            }
        }
        out << "the arcs are :" << std::endl;
        for( const auto& e : g.arcs )
        {
            graph_print(out, e) ;
            out << " ==> ";
            graph_print(out, g.getCost(e))  << std::endl ;
//            out <<  std::endl;
        }
        out << "================END=====================" << std::endl << std::endl;
    //
    return out;
}


}}