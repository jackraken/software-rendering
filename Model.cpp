/*
 * Model.cpp
 *
 *  Created on: 2015�~3��16��
 *      Author: LukeWu
 */


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glmmm.h"
#include "Model.h"
#include "Label.h"
#include "Light.h"
#include "GlobalLoaders.h"


Model::Model(float _x, float _y, float _z, std::string file_name):rotate_center(0.0),rotate_matrix(1.0)
{
	visible = true;
	perspective = true;
	x = _x;
	z = _z;
	y = _y;

	//ObjLoader::LoadObjFromFileWithAABB(file_name.c_str(), &obj_vertex, &obj_uv, &obj_normal,
		//	&aabb_bottom_left, &aabb_top_right);

	ObjLoader::LoadObjFromFileVertexAndNormal(file_name.c_str(), &obj_vertex, &obj_normal, &aabb_bottom_left, &aabb_top_right);

	projected_vertices = std::vector<float>(obj_vertex);
	vertex_color.clear();
	int rgb[3];
	for(unsigned int i=0; i<=obj_vertex.size(); i+=9){

		rgb[0] = (i*7)%120;
		rgb[1] = (i*11)%120;
		rgb[2] = (i*17)%120;
		vertex_color.push_back(rgb[0]);
		vertex_color.push_back(rgb[1]);
		vertex_color.push_back(rgb[2]);
	}

	glGenBuffers(1, &vbo_projected_vertex);
	/*glGenBuffers(1, &vbo_vertex);
	glGenBuffers(1, &vbo_uv);
	glGenBuffers(1, &vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, obj_vertex.size() * sizeof(float),&obj_vertex[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, obj_normal.size() * sizeof(float),&obj_normal[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
	glBufferData(GL_ARRAY_BUFFER, obj_uv.size() * sizeof(float), &obj_uv[0],GL_STATIC_DRAW);*/

	if (x || y || z) {
		glm::vec3 translate_vector(x, y, z);
		translate(translate_vector);
		//model_matrix = glm::rotate(model_matrix, 60.0f, glm::vec3(0, 1, 0));
		//model_matrix = glm::rotate(model_matrix, 300.0f, glm::vec3(1, 0, 0));
		//model_matrix = glm::translate(model_matrix, translate_vector);//glm::vec3(2,0,0));
		/*glm::vec4 bl(aabb_bottom_left,1.0);
		glm::vec4 tr(aabb_top_right,1.0);

		bl = model_matrix * bl;
		tr = model_matrix * tr;
		aabb_bottom_left_model = glm::vec3(bl[0],bl[1],bl[2]);
		aabb_top_right_model = glm::vec3(tr[0],tr[1],tr[2]);*/
	}
}

Model::~Model(){}

void Model::render(Renderer* renderer, bool view_changed){
	if(!visible)return;

	if(!perspective){
		renderOtho(renderer);
		return;
	}

	char* data = renderer->getPixelData();

	bool clip = false;
	float _x, _y, _z;
	float t, depth;
	float pixel_x, pixel_y, pixel_dis;
	glm::vec3 dir, projected_position;
	glm::vec4 modelspace_position;

	//printf("changed~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	projected_vertices.clear();
	//std::vector<float> filled_projected_vertices;

	std::vector<Light> *light_list = renderer->getLightList();
	std::vector<float> lighted_color(vertex_color);
	int rgb[3];
	glm::vec4 normal_cameraspace;
	glm::vec3 normal,light_dir,light_pos;

	float alpha, power;
	printf("obj_vertex.size() = %d\n",obj_vertex.size());
	printf("obj_normal.size() = %d\n",obj_normal.size());

	for(unsigned int i=0; i<obj_vertex.size(); i+=3){
		_x = obj_vertex[i];
		_y = obj_vertex[i+1];
		_z = obj_vertex[i+2];


		if(i%9==0){
			normal_cameraspace = (rotate_matrix * glm::vec4(obj_normal[i], obj_normal[i+1], obj_normal[i+2], 1.0));
			printf("normal = %f,%f,%f\n",obj_normal[i], obj_normal[i+1], obj_normal[i+2]);
			normal = glm::vec3(normal_cameraspace.x, normal_cameraspace.y, normal_cameraspace.z);
			//normal = glm::normalize(normal);
			for(unsigned int l=0 ;l<light_list->size(); l++){
				//if((*light_list)[l].getType() == 0);
				printf("normal = %f,%f,%f\n",normal.x,normal.y,normal.z);
				light_dir = glm::normalize((*light_list)[l].getPosition());
				//printf("light_dir = %f,%f,%f\n",light_dir.x,light_dir.y,light_dir.z);
				alpha = glm::dot(-light_dir, normal);
				alpha = glm::clamp(alpha, 0.0f, 1.0f);
				power = (*light_list)[l].getPower() * alpha;
				printf("alpha=%f  power=%f\n",alpha,power);
				lighted_color[i/3]*=(power+0.1);
				lighted_color[i/3+1]*=(power+0.1);
				lighted_color[i/3+2]*=(power+0.1);
			}
		}
		modelspace_position = renderer->getViewMatrix() * model_matrix * glm::vec4(_x, _y, _z, 1.0);

		int window_height = renderer->getWindow()->getWindowHeight();
		int window_width = renderer->getWindow()->getWindowWidth();

		GLfloat z_near = renderer->getZnear();
		GLfloat rad = renderer->getFovy() / 180 * 3.1415926;

		glm::vec3 camera_position = glm::vec3(0,0,0);

		if(modelspace_position.z < z_near){
			clip = true;
		}
		if(i%9==0 && clip){
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			projected_vertices.pop_back();
			clip = false;
		}

		dir = glm::vec3(modelspace_position.x, modelspace_position.y, modelspace_position.z);
		depth = glm::length(dir);
		dir = glm::normalize(dir);

		GLfloat h_length = tan(rad / 2) * z_near;
		GLfloat w_length = h_length * window_width / window_height;
		glm::vec3 w = glm::vec3(w_length,0,0);
		glm::vec3 h = glm::vec3(0,h_length,0);

		glm::vec3 view = glm::vec3(0,0,-z_near);//camera_look_at_posiotion - camera_position;

		glm::vec3 middle_point_of_screen = camera_position + view;


		t = -z_near/glm::abs(dir.z);

		projected_position = dir * t;

		GLfloat h_offset = glm::dot((projected_position - middle_point_of_screen), h) / glm::dot(h, h);
		GLfloat w_offset = glm::dot((projected_position - middle_point_of_screen), w) / glm::dot(w, w);

		projected_vertices.push_back((w_offset+1)*window_width/2);
		projected_vertices.push_back((h_offset+1)*window_height/2);   //why negative QWQ?
		projected_vertices.push_back(1/modelspace_position.z);

	}

	std::vector<float> v(projected_vertices);
	renderer -> DrawTriangle(v, lighted_color, mode);

}

void Model::renderOtho(Renderer* renderer){

	char* data = renderer->getPixelData();

	float _x, _y, _z;
	float t;
	float pixel_x, pixel_y, pixel_dis;
	glm::vec3 dir, projected_position;
	glm::vec4 modelspace_position;


	projected_vertices.clear();

	for(unsigned int i=0; i<obj_vertex.size(); i+=3){
		_x = obj_vertex[i];
		_y = obj_vertex[i+1];
		_z = obj_vertex[i+2];

		modelspace_position = model_matrix * glm::vec4(_x, _y, _z, 1.0) ;//* renderer->getViewMatrix();

		int window_height = renderer->getWindow()->getWindowHeight();
		int window_width = renderer->getWindow()->getWindowWidth();
		//printf("window_height: %d  window_width: %d\n",window_height,window_width);

		glm::vec3 camera_up = renderer->getCameraUp();
		glm::vec3 camera_position = renderer->getCameraPosition();
		glm::vec3 camera_look_at_posiotion = renderer->getCameraLookAtPosition();

		glm::vec3 view = camera_look_at_posiotion - camera_position;

		dir = glm::vec3(modelspace_position.x, modelspace_position.y, modelspace_position.z) - camera_position;
		//dir = glm::vec3(_x, _y, _z) - camera_position;
		//dir = glm::normalize(dir);

		glm::vec3 w = glm::cross(view, camera_up);
		glm::vec3 h = glm::cross(view, w);

		GLfloat h_length = 10*glm::length(view);
		GLfloat w_length = h_length * window_width / window_height;
		h = glm::normalize(h) * h_length;
		w = glm::normalize(w) * w_length;

		view = glm::normalize(view) * (float)glm::length(dir);

		glm::vec3 middle_point_of_screen = camera_position;

		t =  glm::length(view);

		projected_position = glm::vec3(modelspace_position.x, modelspace_position.y, modelspace_position.z) - view;

		GLfloat h_offset = glm::dot((projected_position - middle_point_of_screen), h) / glm::dot(h, h);
		GLfloat w_offset = glm::dot((projected_position - middle_point_of_screen), w) / glm::dot(w, w);

		projected_vertices.push_back((w_offset+1)*window_width/2);
		projected_vertices.push_back((h_offset+1)*window_height/2);   //why negative QWQ?
		projected_vertices.push_back(t);

	}

	std::vector<float> v(projected_vertices);
	renderer -> DrawTriangle(v, vertex_color, mode);
}

void Model::setMode(int next_mode){
	mode = next_mode;
	if(mode == 2)Label::setTarget(this);
}

void Model::translate(glm::vec3 translate_vector){

	translate_matrix[3].x = translate_vector.x;
	translate_matrix[3].y = translate_vector.y;
	translate_matrix[3].z = translate_vector.z;
	model_matrix =  model_matrix * translate_matrix;
	//return;
	glm::vec4 bl(aabb_bottom_left,1.0);
	glm::vec4 tr(aabb_top_right,1.0);
	bl = model_matrix * bl;
	tr = model_matrix * tr;
	aabb_bottom_left_model = glm::vec3(bl[0],bl[1],bl[2]);
	aabb_top_right_model = glm::vec3(tr[0],tr[1],tr[2]);
	//aabb_top_right +=  translate_vector;
	//aabb_bottom_left += translate_vector;
	//return glm::translate(matrix, translate_vector);
	//model_matrix = glmmm::translate(model_matrix, translate_vector);
}

void Model::rotate(float angle, glm::vec3 axis, glm::vec3 offset){
	glm::mat4 rotate_m(1.0);
	float u = axis.x;
	float v = axis.y;
	float w = axis.z;
	angle = angle/360*2*3.14159;
	rotate_m[0].x = glm::cos(angle) + (u * u) * (1 - glm::cos(angle));
	rotate_m[1].x = u * v * (1 - glm::cos(angle)) + w * glm::sin(angle);
	rotate_m[2].x = u * w * (1 - glm::cos(angle)) - v * glm::sin(angle);
	rotate_m[0].y = u * v * (1 - glm::cos(angle)) - w * glm::sin(angle);
	rotate_m[1].y = glm::cos(angle) + v * v * (1 - glm::cos(angle));
	rotate_m[2].y = w * v * (1 - glm::cos(angle)) + u * glm::sin(angle);
	rotate_m[0].z = u * w * (1 - glm::cos(angle)) + v * glm::sin(angle);
	rotate_m[1].z = v * w * (1 - glm::cos(angle)) - u * glm::sin(angle);
	rotate_m[2].z = glm::cos(angle) + w * w * (1 - glm::cos(angle));
	translate(rotate_center);
	model_matrix =  model_matrix * rotate_m;
	rotate_matrix = rotate_matrix * rotate_m;
	translate(-rotate_center);
	//model_matrix = glm::rotate(model_matrix, angle, axis);
}

void Model::scale(glm::vec3 scale_vector){
	glm::mat4 scale_matrix(1.0);
	scale_matrix[0].x = scale_vector.x;
	scale_matrix[1].y = scale_vector.y;
	scale_matrix[2].z = scale_vector.z;
	model_matrix =  model_matrix * scale_matrix;
}

void Model::mirror(glm::vec3 mirror_vector){
	if(mirror_vector.x != 0){
		for(unsigned int i=0; i<obj_vertex.size(); i+=3){
			obj_vertex[i] = -obj_vertex[i];
		}
		aabb_bottom_left_model.x += (mirror_vector.x - aabb_bottom_left_model.x)*2;
		aabb_top_right_model.x += (mirror_vector.x - aabb_top_right_model.x )*2;
		std::swap(aabb_bottom_left_model.x, aabb_top_right_model.x);
		model_matrix = glm::translate(model_matrix, glm::vec3((mirror_vector.x-x)*2, 0, 0));
		x+=(mirror_vector.x-x)*2;
	}
	else if (mirror_vector.y != 0) {
		for (unsigned int i = 0; i < obj_vertex.size(); i += 3) {
			obj_vertex[i+1] = -obj_vertex[i+1];
		}
		aabb_bottom_left_model.y += (mirror_vector.y - aabb_bottom_left_model.y)*2;
		aabb_top_right_model.y += (mirror_vector.y - aabb_top_right_model.y )*2;
		std::swap(aabb_bottom_left_model.y, aabb_top_right_model.y);
		model_matrix = glm::translate(model_matrix, glm::vec3(0, (mirror_vector.y-y)*2, 0));
		y+=(mirror_vector.y-y)*2;
	}
	else {
		for (unsigned int i = 0; i < obj_vertex.size(); i += 3) {
			obj_vertex[i+2] = -obj_vertex[i+2];
		}
		aabb_bottom_left_model.z += (mirror_vector.z - aabb_bottom_left_model.z)*2;
		aabb_top_right_model.z += (mirror_vector.z - aabb_top_right_model.z )*2;
		std::swap(aabb_bottom_left_model.z, aabb_top_right_model.z);
		model_matrix = glm::translate(model_matrix, glm::vec3(0, 0, (mirror_vector.z-z)*2));
		z+=(mirror_vector.z-z)*2;
	}
}

void Model::shear(glm::vec3 shear_vector){
	for (unsigned int i = 0; i < obj_vertex.size(); i += 3) {
		if(shear_vector.x == 1 )obj_vertex[i] += obj_vertex[i+2];
		else obj_vertex[i] -= obj_vertex[i+2];
	}
}

void Model::update(){

	return;
}


void Model::keyActive(int key, int action){
	if(key == 'M' && action == GLFW_PRESS){
		mirror(glm::vec3(10,0,0));
	}
	else if(key == 'C' && action == GLFW_PRESS){
		scale(glm::vec3(1.2,1.2,1.2));
	}
}
