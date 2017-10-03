#include "Boid.h"

using namespace glm;
using namespace std;

Boid::Boid() {

}


Boid::~Boid() {
}

void Boid::updateParams(){
	
	parameters.clear();
	
	char line[252];
	filebuf fName;
	
	fName.open("boidConfig.txt", std::ios::in);
	istream myfile(&fName);
	
	if (myfile.good())
	{
		while (  !myfile.eof() )
		{
			myfile.ignore(512, '='); 
			
			myfile.getline(line, 512);
		    
		    //Uncomment to see what its actually getting if desired
		    //cout << line << '\n';
		    parameters.push_back(atof(line));
		}
	} else {
		cout << "Unable to open file"; 
	}
/*	
    for (auto p : parameters) {
        cout << "p: " << p << endl;
    }
*/
	followRadius = parameters[0];
	velocityMatchRadius = parameters[1];
	avoidanceRadius = parameters[2];
	
	followWeight = parameters[3];
	velocityMatchWeight = parameters[4];
	avoidanceWeight = parameters[5];
	
    fieldOfView = parameters[6];

    maxSpeed = parameters[7];
}

bool Boid::calcVisibility(vec3 nearbyBoidPos)
{
    vec3 result = nearbyBoidPos - position;
    float radianFOV = radians(fieldOfView);
    
    if((length(accelerationVector) == 0) || (length(result) == 0)){
		return true;
	} else{
		return (dot(normalize(accelerationVector), normalize(result)) > cos(radianFOV));
	}
}
