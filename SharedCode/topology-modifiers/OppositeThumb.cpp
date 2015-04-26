#include "OppositeThumb.h"

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
const IndexSet replacementEdgeIndices = IndexSet()
/24/25/26 /21/22/23 /132/137/138/ 133/ 127/128/ 122/123/ 117/118/ 114/115/ 111/112;

const IndexSet gapReplacementIndices = IndexSet()
/112/115/118/123/128/
133/138/23/22/
21/132/127/122/117/114/111/
137;

string OppositeThumb::getName() const {
    return "OppositeThumb";
}

void OppositeThumb::initialize() {
    removeThumb = true;
    
    squishConstPower = 0.4f;
    squishRadius = 150.0f;
    
    OppositeThumb::initializeGui();
    this->gui->addToggle("wireframe", &wireframe);
    this->gui->addSlider("squishRadius", 0.0, 500.0, &squishRadius);
    this->gui->addSlider("squishConstPower", 0.0, 1.0, &squishConstPower);
    this->gui->autoSizeToFitWidgets();
    
    final.load("models/mesh_OppositeThumb_final.ply");
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

void OppositeThumb::update(const ofMesh& mesh) {
    
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
    
    // smooth out the side of the hand that we removed the thumb
    // from to avoid it looking so flat
    
    ofVec3f squishTarget = handMesh.getVertex(126)+(handMesh.getVertex(126)-handMesh.getVertex(125));
    for(int v = 0; v < handMesh.getNumVertices(); v++) {
        ofVec3f d = squishTarget-handMesh.getVertex(v);
        
        float h = (handMesh.getVertex(v)-squishTarget).length();
        
        float squishFactor = (1.0-(h/squishRadius)) * squishConstPower;
        if(squishFactor < 0.0) squishFactor = 0.0;
        
        ofVec3f cur = handMesh.getVertex(v);
        cur += d * squishFactor;
        handMesh.setVertex(v, cur);
    }
    
    if(removeThumb) {
        
        // add blending to edge of hand where we removed the thumb
        // to avoid the edge looking to sharp (we do this by copying
        // the original thumbless side of the hand)
        
        replacementEdgeMesh = copySubmesh(handMesh, replacementEdgeIndices);
        
        int toBeMovedIndices[] = {
            3,4,5, 0,1,2,
            18,19,
            16,17, 14,15, 12,13, 10,11, 8,9, 6,7
        };
        int moveToIndices[]    = {
            89,88,87, 86,85,84,
            143,142,
            136,135, 131,130, 126,125, 121,120, 116,115, 113,112
        };
        int moveVertsCount = 20;
        
        for(int i = 0; i < moveVertsCount; i++) {
            replacementEdgeMesh.setVertex(toBeMovedIndices[i],
                                          handMesh.getVertex(moveToIndices[i]));
        }
        
        
        int opacity[] = {1,0.5,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,0.5,0.0};
        int changeOpacityCount = 20;
        
        vector<ofFloatColor> colors(changeOpacityCount);
        for(int i = 0; i < changeOpacityCount; i++) {
            colors[i] = ofFloatColor(1, opacity[i]);
        }
        replacementEdgeMesh.addColors(colors);
        
        // remove the thumb
        
        handMesh = removeSubmesh(handMesh, thumbIndices);
        
        // the edge where we removed the thumb sticks out a lot still.
        // so here we make each vertex of the edge the same distance
        // from the middle of the hand.
        
        int edgeIndices[] = {
            115,110,105,100,95
        };
        int fromEdgeIndices[] = {
            114,109,104,99,94
        };
        int numEdgeVerticesToSmooth = 5;
        
        float edgeTargetLength = handMesh.getVertex(edgeIndices[0]).distance(handMesh.getVertex(fromEdgeIndices[0]));
        
        for(int i = 0; i < numEdgeVerticesToSmooth; i++) {
            
            ofVec3f edge = handMesh.getVertex(edgeIndices[i]);
            ofVec3f fromEdge = handMesh.getVertex(fromEdgeIndices[i]);
            
            float h = edge.distance(fromEdge);
            
            ofVec3f edgeLength = edge - fromEdge;
            edgeLength.normalize();
            
            edgeLength = edgeLength * edgeTargetLength;
            
            ofVec3f newEdge = fromEdge + edgeLength;
            
            handMesh.setVertex(edgeIndices[i], newEdge);
            
        }
        
        // this vertex sticks out a little still, we fix it here
        // (find average two adjacent vertices)
        
        handMesh.setVertex(92,
                           (handMesh.getVertex(95)+handMesh.getVertex(89))/2
                           );
        
        // save final mesh for butterfly smoothing
        
        if(saveFinalMesh) handMesh.save("models/mesh_OppositeThumb_final.ply");
    }
    
    // finalize
    final = handMesh;
}

ofMesh& OppositeThumb::getModifiedMesh() {
    return final;
}

void OppositeThumb::drawBlends() {
    
}

void OppositeThumb::draw (const ofTexture& texture) {
    
    texture.bind();
    if (bUseButterfly){
        
        if(!wireframe) {
            
            butterflySubdivider.fixMesh (final, refinedMesh);
            refinedMesh.drawFaces();
            
            glDisable(GL_CULL_FACE);
            
            final.drawFaces();
            
            extraThumbBlendMesh.drawFaces();
            wristBlendMesh.drawFaces();
            
            if(removeThumb) {
                //replacementEdgeMesh.drawFaces();
            }
            
        } else {
            
            texture.unbind();
            
            final.drawWireframe();
            
            if(removeThumb) {
                replacementEdgeMesh.drawWireframe();
            }
            
        }
        
    } else {
        final.drawFaces();     // was drawBase();
        extraThumbBlendMesh.drawFaces(); // was drawBlends();
    }
    texture.unbind();
}

void OppositeThumb::saveMeshes(){
    
}
