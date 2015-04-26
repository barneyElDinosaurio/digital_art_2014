// Experimental fractal hand scene!
// work in progress

#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class FractalHand : public TopologyModifier {
protected:
    
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    vector<ofMesh> fingerMeshes;
    vector<ofMesh> fingertipMeshes;
    vector<ofMesh> miniHandMeshes;
    vector<ofMesh> wristMeshes;
    
    ofMesh final;
    
    ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
    bool                    bUseButterfly;
    
    bool wireframe;
    bool squish;
    
    int sourceBlendOpIndex;
    int destBlendOpIndex;
    
    ofFbo miniHandsFBO;
    
    // ofShader blurShader;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
    void saveMeshes();
    void initialize();
    void draw(const ofTexture& texture);
};