#ifndef BOID_H
#define BOID_H

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

class Boid {
public:
	
	Boid();
	~Boid();
	
    bool pred = false;
	std::vector<float> parameters;
	
	glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 accelerationVector = glm::vec3(0, 0, 0);

	float followRadius;
	float velocityMatchRadius;
	float avoidanceRadius;
	
	float followWeight;
	float velocityMatchWeight;
	float avoidanceWeight;
	
	float fieldOfView;
	
    float maxSpeed;
	void updateParams();
    bool calcVisibility(glm::vec3 nearbyBoidPos);
};

#endif
