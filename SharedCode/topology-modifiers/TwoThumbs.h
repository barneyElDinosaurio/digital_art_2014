// Two thumbs scene!
// rispoli 1-26-15

#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class TwoThumbs : public TopologyModifier {
protected:
    
    ofPolyline removalRegion;
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    ofMesh origThumbMesh;
    ofMesh thumbMesh;
    
    ofMesh extraThumbBlendMesh;
    ofMesh wristBlendMesh;
    
    ofMesh final;
    
    ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
    bool            bUseButterfly;
    
    bool wireframe;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
    void saveMeshes();
    void initialize();
    void draw(const ofTexture& texture);
};