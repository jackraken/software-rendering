/*
 * Light.h
 *
 *  Created on: 2015�~4��27��
 *      Author: LukeWu
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Light {
public:
	Light(float r, float g, float b, float a, float x, float y, float z, float p);
	glm::vec4 getRGBA(){return rgba;}
	glm::vec3 getPosition(){return position;}
	unsigned int getType(){return type;}
	float getPower(){return power;}
protected:
	unsigned int type;// 0:parallel light, 1:spot light
	bool on;
	glm::vec4 rgba;
	glm::vec3 position;
	float power;
};

#endif /* LIGHT_H_ */
