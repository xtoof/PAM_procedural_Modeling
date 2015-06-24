//
//  Module.h
//  MeshEditE
//
//  Created by Francesco Usai on 24/04/15.
//  Copyright (c) 2015 J. Andreas Bærentzen. All rights reserved.
//

#ifndef __MeshEditE__Module__
#define __MeshEditE__Module__

#include <stdio.h>

#include <map>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <iostream>

#include <GEL/HMesh/Manifold.h>
#include <GEL/CGLA/Vec3d.h>
#include <GEL/CGLA/Mat4x4d.h>

#include <polarize.h>

#include "pam_skeleton.h"
#include "collision_detection.h"

namespace Procedural{
    
typedef std::pair< HMesh::VertexID, HMesh::VertexID >   Match;
    
enum PoleLabel {
    NoneP    = 0,       // this is 00000000 it will match no value
    RedP     = 1,
    BlueP    = 2,
    GreenP   = 4,
    CyanP    = 8,
    MagentaP = 16,
    YellowP  = 32,
    PurpleP  = 64,
    AllP     = 127      // this is 11111111 it will match each value
};

typedef unsigned int Moduletype;
    
struct PoleGeometryInfo{
    unsigned int    valence;
    CGLA::Vec3d     pos;
    CGLA::Vec3d     normal;
};
    
struct PoleInfo{
    PoleGeometryInfo    geometry;
    Moduletype          moduleType  = 0;     // an identifier for the module's type
    PoleLabel           labels      = AllP;  // a label attached to a pole
    int                 age;                 // pole's starting age
    bool                isFree      = false;
};
    
typedef std::map<HMesh::VertexID, PoleInfo>             PoleInfoMap;
typedef std::vector< HMesh::VertexID>                   PoleList;
typedef std::set< HMesh::VertexID >                     PoleSet;

class Module{
    
public:
         // this will instantiate the internal manifold structure and pole info using obj_load
         Module () { m = NULL; }
         Module( std::string path, Moduletype mType );
         Module( HMesh::Manifold &manifold, Moduletype mType, size_t no_glueings = 1 );
    
        Module& getTransformedModule( const CGLA::Mat4x4d &T, bool transform_module = false );
        void reAlignIDs( HMesh::VertexIDRemap &remapper );
    
        const PoleInfo&    getPoleInfo( HMesh::VertexID p ) const;
        bool isPole( HMesh::VertexID v );
        const Skeleton& getSkeleton() const;

        inline const PoleInfoMap& getPoleInfoMap(){ return poleInfoMap;}
private:
    void    BuildPoleInfo();            
    
/************************************************
* ATTRIBUTES                                   *
***********************************************/
public:
#warning those should be deleted from public
    HMesh::Manifold     *m;
    int                 no_of_glueings;
    PoleList            poleList;
    PoleSet             poleSet;

    CGLA::Vec3d         bsphere_center;
    double              bsphere_radius;
    
private :
    PoleInfoMap         poleInfoMap;
    Skeleton            *skeleton;

    };
}

#endif /* defined(__MeshEditE__Module__) */
