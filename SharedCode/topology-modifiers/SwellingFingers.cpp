#include "SwellingFingers.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);
const IndexSet thumbIndices  = IndexSet() /IndexRange(0,20);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104)
/IndexRange(0,20);

const IndexSet knuckeIndices = IndexSet() /IndexRange(3,11);

string SwellingFingers::getName() const {
    return "SwellingFingers";
}

void SwellingFingers::initialize(){
    
    timer = 0.0f;
    
    pulseSpeed = 7.0f;
    pulseGapWidth = 0.7f;
    
    swellAmt = 0.0f;
    swellPower = 0.03f;
    
    SwellingFingers::initializeGui();
    this->gui->addSlider("swellAmt", 0.0, 1.0, &swellAmt);
    this->gui->addSlider("swellPower", 0.0, 1.0, &swellPower);
    this->gui->addSlider("pulseSpeed", 0.0, 10.0, &pulseSpeed);
    this->gui->addSlider("pulseGapWidth", 0.0, 1.0, &pulseGapWidth);
    this->gui->autoSizeToFitWidgets();
    
    fingerMeshes.resize(5);
    
	final.load("models/mesh_SwellingFingers_final.ply");
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

void SwellingFingers::update(const ofMesh& mesh) {
    timer = ofGetElapsedTimef();
    swellAmt = (sin(timer * pulseSpeed) + 1.0f) / 2.0f;
    if(swellAmt > pulseGapWidth) {
        swellAmt -= (swellAmt - pulseGapWidth) * 2.0;
    }
    swellAmt *= swellPower;
    
    ofMesh handMesh = mesh;
    
    fingerMeshes[0] = copySubmesh(handMesh, pinkyIndices);
    fingerMeshes[1] = copySubmesh(handMesh, ringIndices);
    fingerMeshes[2] = copySubmesh(handMesh, middleIndices);
    fingerMeshes[3] = copySubmesh(handMesh, indexIndices);
    fingerMeshes[4] = copySubmesh(handMesh, thumbIndices);
    
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    float baseSwellAmount[] = {
        0, 1, 2, 3, 4,
        5, 5, 5, 5, 5,
        4, 3, 2, 1, 0,
    };
    
    for(int i = 0; i < 15; i++) {
        //exponential
        baseSwellAmount[i] = (baseSwellAmount[i] * baseSwellAmount[i]) * swellAmt;
        if(baseSwellAmount[i] < 0) baseSwellAmount[i] = 0;
    }
    
    int indicesNeedNormals[]     = {
        0,3,6,9,12,
        15,18,19,20,17,
        14,11,8,5,2
    };
    int normalsDirectionSource[] = {
        1,4,7,10,13,
        16,16,16,16,16,
        13,10,7,4,1
    };
    int normalsToCalculateCount = 15;
    
    for(int i = 0; i < normalsToCalculateCount; i++) {
        for(int f = 0; f < 5; f++) {
            ofVec3f source = fingerMeshes[f].getVertex(normalsDirectionSource[i]);
            ofVec3f dir    = fingerMeshes[f].getVertex(indicesNeedNormals[i]);
            
            ofVec3f normal = dir.operator-(source);
            normal.operator*=(baseSwellAmount[i]);
            
            fingerMeshes[f].setVertex(indicesNeedNormals[i], dir.operator+(normal));
        }
    }
    
    final = handMesh;
    for(int f = 0; f < 5; f++) {
        final.append(fingerMeshes[f]);
    }
    
    if(saveFinalMesh) final.save("models/mesh_SwellingFingers_final.ply");
}

ofMesh& SwellingFingers::getModifiedMesh() {
    return final;
}

void SwellingFingers::drawBlends() {
	
}

void SwellingFingers::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        final.drawFaces();
        
        for(int f = 0; f < 5; f++) {
            fingerMeshes[f].drawFaces();
        }
        
	} else {
		final.drawFaces();     // was drawBase();
		blendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

void SwellingFingers::saveMeshes(){

}
