#include "TwoThumbs.h"

const IndexSet thumbIndices = IndexSet()
/IndexRange(0,20)
/86/116/121/126/131/136
/IndexRange(144,150);

const IndexSet extraThumbBlendIndices = IndexSet()
/89/88/87 /86/85/84 /136/135 /142 /143
/131/126/121/116/113/110
/130/125/120/115/112/109;

const IndexSet wristBlendIndices = IndexSet()
/144
/131/126/121/116/113
/130/125/120/115/112;

string TwoThumbs::getName() const {
    return "TwoThumbs";
}

void TwoThumbs::initialize() {

    TwoThumbs::initializeGui();

    this->gui->addSpacer();
    this->gui->addToggle("wireframe", &wireframe);
    this->gui->autoSizeToFitWidgets();
    
    final.load("models/mesh_TwoThumbs_final.ply");
    for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
        ofVec2f aMeshVertex;
        aMeshVertex.x =  final.getVertex(i).y;
        aMeshVertex.y =  768 - final.getVertex(i).x;
        final.addTexCoord( aMeshVertex );
    }
    
    // Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
    
    bUseButterfly = true;
    
}

void TwoThumbs::update(const ofMesh& mesh) {
    
    ofMesh handMesh = mesh;
    
    // get a mesh of just the thumb
    
    thumbMesh = copySubmesh(handMesh, thumbIndices);
    
    ofVec3f flipVertex = thumbMesh.getVertex(21); // closest point to index finger
    ofVec3f bottomVertex = thumbMesh.getVertex(22); // closest point to wrist
    
    // flip new thumb vertically on flipVertex
    
    for (int i = 0; i < thumbMesh.getNumVertices(); i++) {
        ofVec3f v = thumbMesh.getVertex(i);
        
        ofVec3f flippedVertex = ofVec3f(v.x, flipVertex.y-(v.y-flipVertex.y)-150, v.z);
        thumbMesh.setVertex(i, flippedVertex);
    }
    
    handMesh.append(thumbMesh);
    
    // attach new thumb to other side of hand
    
    int toStitchLeft[]  = {21, 132,127,122,111};
    int toStitchRight[] = {172,177,176,175,173};
    int toStitchCount = 5;
    
    stitchIndices.clear();
    for(int i = 0; i < toStitchCount; i++) {
        stitchIndices.push_back(pair<ofIndexType, ofIndexType>(toStitchLeft[i], toStitchRight[i]));
    }
    handMesh = stitch(handMesh, stitchIndices, STITCH_FROM_LEFT);
    
    // fix the gap
    // (the gap happens because we're attaching 5 vertices to 6 vertices...)
    
    handMesh.setVertex(174, handMesh.getVertex(117));
    handMesh.setVertex(114, (handMesh.getVertex(111)+handMesh.getVertex(117))/2.0);
    
    // blend upper palm with thumb
    
    extraThumbBlendMesh = copySubmesh(handMesh, extraThumbBlendIndices);
    
    int extraThumbToBeMovedIndices[] = {
        3,0,
        20,18,16,14, 12,10,8,6,
        4,1,21,
        5,2, 19,17,15,13,11,9,7
    };
    
    int extraThumbMoveToIndices[] = {
        26,23,
        138,133,128,123, 118,115,112,109,
        25,22,137,
        24,21, 132,127,122,117,114,111,108
    };
    
    int moveVertsCount = 22;
    
    for(int i = 0; i < moveVertsCount; i++) {
        extraThumbBlendMesh.setVertex(extraThumbToBeMovedIndices[i],
                                      handMesh.getVertex(extraThumbMoveToIndices[i]));
    }
    
    extraThumbBlendMesh.setTexCoord(9, extraThumbBlendMesh.getTexCoord(11));
    extraThumbBlendMesh.setTexCoord(11,
                                    (extraThumbBlendMesh.getTexCoord(9)+extraThumbBlendMesh.getTexCoord(13))/2.0
                                    );
    extraThumbBlendMesh.setVertex(7, handMesh.getVertex(105));
    
    int extraThumbOpacity[] = {
        0, .5, 1, 0, 0,
        0, 0, 0, 0, 0,
        0, 1, 0, 1, 0,
        1, 0, 1, 0, 1,
        0, .5
    };
    int extraThumbChangeOpacityCount = 22;
    
    vector<ofFloatColor> colors(extraThumbChangeOpacityCount);
    for(int i = 0; i < extraThumbChangeOpacityCount; i++) {
        colors[i] = ofFloatColor(1, extraThumbOpacity[i]);
    }
    extraThumbBlendMesh.addColors(colors);
    
    // blend wrist with lower thumb
    
    wristBlendMesh = copySubmesh(handMesh, wristBlendIndices);
    
    int wristToBeMovedIndices[] = {
        8,6,4,2,0,
        9,7,5,3,1,
        10
    };
    
    int wristMoveToIndices[] = {
        123,118,115,112,109,
        122,117,114,111,108,
        178
    };
    
    int wristMoveVertsCount = 11;
    
    for(int i = 0; i < wristMoveVertsCount; i++) {
        wristBlendMesh.setVertex(wristToBeMovedIndices[i],
                                 handMesh.getVertex(wristMoveToIndices[i]));
    }
    
    int wristOpacity[] = {
        0,0,0,1,0,
        1,0,0,0,0,
        0
    };
    int wristChangeOpacityCount = 11;
    
    vector<ofFloatColor> wristColors(wristChangeOpacityCount);
    for(int i = 0; i < wristChangeOpacityCount; i++) {
        wristColors[i] = ofFloatColor(1, wristOpacity[i]);
    }
    wristBlendMesh.addColors(wristColors);
    
    //finalize
    final = handMesh;
    
}

ofMesh& TwoThumbs::getModifiedMesh() {
    return final;
}

void TwoThumbs::drawBlends() {
    
}

void TwoThumbs::draw (const ofTexture& texture) {
    
    if (bUseButterfly){
        
        if(!wireframe) {
            
            texture.bind();
            
            butterflySubdivider.fixMesh (final, refinedMesh);
            refinedMesh.drawFaces();
    
            final.drawFaces();
            
            extraThumbBlendMesh.drawFaces();
            wristBlendMesh.drawFaces();

        } else {
            
            texture.unbind();
            
            ofSetColor(255,255,255,255);
            
            butterflySubdivider.fixMesh (final, refinedMesh);
            refinedMesh.drawWireframe();
            final.drawWireframe();
            
        }
        
    } else {
        final.drawFaces();
        extraThumbBlendMesh.drawFaces();
    }
    
    texture.unbind();
}

void TwoThumbs::saveMeshes(){
    
}