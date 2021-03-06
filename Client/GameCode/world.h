#pragma once

#include <assert.h> 

#include "../PlatformShared/platform_shared.h"
#include "../staggered_concentric_pattern/memory.h"

#include "math.h"
#include "pattern.h"
#include "bsp_tree.h"

#define	DIST_EPSILON	(0.03125)

// each face is a quad
struct Face
{
	// Assume this is a quad
	// p0 p1 p2 p3 in clock wise order
	std::vector<glm::vec3> vertices;
};

Plane NULL_PLANE;

enum EntityFlag
{
	STATIC,
	PLAYER
};

struct Entity
{
	EntityFlag flag;

	bool IsPatternCircle;

	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 velocity;

	// For AABB physics, in object space
	glm::vec3 min;
	glm::vec3 max;

	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;
	float pitch;
	void SetViewDirection(glm::vec3 viewDirection)
	{
		zAxis = -viewDirection;
	}

	glm::vec3 GetViewDirection()
	{
		return -zAxis;
	}

	// usually you just pass in the gluLookAtMatrix
	void SetOrientation(glm::mat4 cameraMatrix)
	{
		// Hack: Todo, get rid of this extra inverse
		xAxis = glm::vec3(cameraMatrix[0][0], cameraMatrix[0][1], cameraMatrix[0][2]);
		yAxis = glm::vec3(cameraMatrix[1][0], cameraMatrix[1][1], cameraMatrix[1][2]);
		zAxis = glm::vec3(cameraMatrix[2][0], cameraMatrix[2][1], cameraMatrix[2][2]);
	}

	Entity* groundEntity;
	Plane groundPlane;
	// For Rendering
	// TODO: change this model index
	std::vector<Face> model;
};

struct PlayerEntity
{
	// consider storing these 4 as a matrix?
	glm::vec3 position;
	// camera is viewing along -zAxis
	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;
};


struct World
{
	MemoryArena memoryArena;

	BSPNode* tree;

	Entity entities[1024];
	int numEntities;
	int maxEntityCount;


	int startPlayerEntityId;
	int maxPlayerEntity;
	int numPlayerEntity;

};


void initEntity(Entity* entity, glm::vec3 pos, std::vector<Face> faces)
{
	entity->pos = pos;
	entity->model = faces;
	entity->flag = EntityFlag::STATIC;
}

void initPlayerEntity(Entity* entity, glm::vec3 pos)
{
	entity->pos = pos;
	entity->flag = EntityFlag::PLAYER;
	entity->min = glm::vec3(-10, -10, -10);
	entity->max = glm::vec3(10, 10, 10);

	entity->xAxis = glm::vec3(1.0, 0.0, 0.0);
	entity->yAxis = glm::vec3(0.0, 1.0, 0.0);
	entity->zAxis = glm::vec3(0.0, 0.0, 1.0);
}

std::vector<glm::vec3> GetCubeVertices(glm::vec3 min, glm::vec3 max)
{
	/*
		y
		^
	  (-x,y,-z) p4 ------------ p5 (x,y,-z)
		|		|\              |\
		|		| \             | \
		|		| (-x,y,z)      |  \
		|		|	p0 ------------ p1 (x,y,z)
		|	    p6 -|----------	p7	|
	   (-x,-y,-z)\  |	(x,-y,-z)\	|
		|		  \	|		      \ |
		|		   \|			   \|
		|			p2 ------------ p3 (x,-y,z)
		|         (-x,-y,z)
		|
		------------------------------------------> x
		\
		 \
		  \
		   V z
	*/

	// 4 points on front face 
	glm::vec3 p0 = glm::vec3(min.x, max.y, max.z);
	glm::vec3 p1 = glm::vec3(max.x, max.y, max.z);
	glm::vec3 p2 = glm::vec3(min.x, min.y, max.z);
	glm::vec3 p3 = glm::vec3(max.x, min.y, max.z);

	// 4 points on back face 
	glm::vec3 p4 = glm::vec3(min.x, max.y, min.z);
	glm::vec3 p5 = glm::vec3(max.x, max.y, min.z);
	glm::vec3 p6 = glm::vec3(min.x, min.y, min.z);
	glm::vec3 p7 = glm::vec3(max.x, min.y, min.z);

	return {p0, p1, p2, p3, p4, p5, p6, p7};
}

std::vector<glm::vec3> ToVector(glm::vec3* data)
{
	std::vector<glm::vec3> result;
	for (int i = 0; i < ArrayCount(data); i++)
	{
		result.push_back(data[i]);
	}
	return result;
}

std::vector<Face> CreateCircleMesh(glm::vec3 center, float radius, float thickness)
{
	// to limit the radius
	if (thickness > radius / 2)
		thickness = radius / 2;

	std::vector<Face> result;
	int step = 1;
	for (float i = 0; i < 360; i += step)
	{
		//phys
		float cos1 = cos(i * Math::DEGREE_TO_RADIAN);
		float sin1 = sin(i * Math::DEGREE_TO_RADIAN);

		float cos2 = cos((i + step) * Math::DEGREE_TO_RADIAN);
		float sin2 = sin((i + step) * Math::DEGREE_TO_RADIAN);

		// outer ring
		float wx = radius * cos1;
		float wz = radius * sin1;
		glm::vec3 simPos0 = glm::vec3(wx, 0, wz);

		wx = radius * cos2;
		wz = radius * sin2;
		glm::vec3 simPos1 = glm::vec3(wx, 0, wz);


		// inner ring
		float radius2 = radius - thickness;
		wx = radius2 * cos1;
		wz = radius2 * sin1;
		glm::vec3 simPos2 = glm::vec3(wx, 0, wz);

		wx = radius2 * cos2;
		wz = radius2 * sin2;
		glm::vec3 simPos3 = glm::vec3(wx, 0, wz);

		Face face;
		face.vertices = { simPos2, simPos3, simPos1, simPos0 };

		for (int i = 0; i < face.vertices.size(); i++)
		{
			face.vertices[i] += center;
		}

		result.push_back(face);
	}
	return result;
}


enum RampRiseDirection
{
	POS_X,
	NEG_X,
	POS_Z,
	NEG_Z,
};

// min max as a volume
std::vector<Face> CreateRampMinMax(glm::vec3 min, glm::vec3 max, RampRiseDirection rampRiseDirection)
{
	std::vector<Face> result;
	std::vector<std::vector<glm::vec3>> temp;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];

	if (rampRiseDirection == POS_Z)
	{
		p4 = p6;
		p5 = p7;
	}
	else if (rampRiseDirection == NEG_Z)
	{
		p0 = p2;
		p1 = p3;
	}
	else if (rampRiseDirection == POS_X)
	{
		p0 = p2;
		p4 = p6;
	}
	else if (rampRiseDirection == NEG_X)
	{
		p5 = p7;
		p1 = p3;
	}

	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 
	

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}





std::vector<Face> CreatePlaneMinMax(glm::vec3 min, glm::vec3 max)
{
	std::vector<Face> result;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];


	std::vector<std::vector<glm::vec3>> temp;
	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}



std::vector<Face> CreateCubeFaceMinMax(glm::vec3 min, glm::vec3 max)
{
	std::vector<Face> result;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];

	// counter clockwise
	std::vector<std::vector<glm::vec3>> temp;
	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}


std::vector<Face> CreateCubeFaceCentered(glm::vec3 pos, glm::vec3 dim)
{
	glm::vec3 min = pos - dim;
	glm::vec3 max = pos + dim;
	return CreateCubeFaceMinMax(min, max);
}


void AddPolygonToBrush(Brush* brush, std::vector<glm::vec3> verticesIn)
{
	const int arraySize = verticesIn.size();
	glm::vec3* vertices = new glm::vec3[arraySize];

	for (int i = 0; i < arraySize; i++)
	{
		vertices[i] = verticesIn[i];
	}

	assert(arraySize == 4);
	// attemp to build normal in all dimensions
	glm::vec3 normal;
	for (int i = 0; i < 2; i++)
	{
		glm::vec3 v0 = vertices[i+1] - vertices[i];
		glm::vec3 v1 = vertices[i+2] - vertices[i+1];
		normal = glm::normalize(glm::cross(v0, v1));

		if (!isnan(normal.x) && !isnan(normal.y) && !isnan(normal.z))
		{
			break;
		}
	}


	if (!isnan(normal.x) && !isnan(normal.y) && !isnan(normal.z))
	{
		std::cout << "normal " << normal << std::endl;
		BspPolygon polygon(vertices, 4);
		float dist = glm::dot(normal, vertices[0]);
		polygon.plane = { normal, dist };
		std::cout << "	dist " << dist << std::endl;
		brush->polygons.push_back(polygon);
		brush->used.push_back(false);
	}


}


Brush ConvertFaceToBrush(std::vector<Face> faces)
{
	Brush brush;

	for (int i = 0; i < faces.size(); i++)
	{
		AddPolygonToBrush(&brush, faces[i].vertices);
	}
	std::cout << std::endl;
	return brush;
}



std::vector<Face> PatternToFaces(Pattern* pattern)
{
	std::vector<Face> result;

	for (int i = 0; i < pattern->circlesA.size(); i++)
	{
		std::vector<Face> circleMesh = CreateCircleMesh(glm::vec3(pattern->centerA, 0), pattern->circlesA[i].radius, 1);
		for (int j = 0; j < circleMesh.size(); j++)
		{
			result.push_back(circleMesh[j]);
		}
	}

	for (int i = 0; i < pattern->circlesB.size(); i++)
	{
		std::vector<Face> circleMesh = CreateCircleMesh(glm::vec3(pattern->centerB, 0), pattern->circlesB[i].radius, 1);
		for (int j = 0; j < circleMesh.size(); j++)
		{
			result.push_back(circleMesh[j]);
		}
	}

	return result;
}




void CreateAreaA(World* world, std::vector<Brush>& brushes)
{
	Entity* entity = NULL;
	std::vector<Face> faces;

	// lower level
	// Box
	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 min;
	glm::vec3 max;


	/*
	// plane 1
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-200, -20, -200);
	max = glm::vec3(200, 0, 0);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
	
	
	// plane 1 wall 1
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-200, 0, 0);
	max = glm::vec3(200, 100, 25);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
	

	
	// plane 1 wall 2
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-201, 0, -200);
	max = glm::vec3(-200, 100, 0);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// plane 1 wall 3
	std::cout << "plane 1 wall 3" << std::endl;
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(200, 0, -200);
	max = glm::vec3(201, 100, 0);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);



	// plane 1 wall 4
	std::cout << "plane 1 wall 4" << std::endl;
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-200, 0, -201);
	max = glm::vec3(-100, 100, -200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
	



	// plane 2
	std::cout << "plane 2" << std::endl;
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(0, -50, -400);
	max = glm::vec3(200, 0, -200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
		

	// ramp, doing it as a hack
	std::cout << "ramp" << std::endl;
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-100, -50, -400);
	max = glm::vec3(0, 0, -200);

	faces = CreateRampMinMax(min, max, POS_Z);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);

	
	// walls for the ramp
	std::cout << "walls for ramp" << std::endl;
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(0, -50, -400);
	max = glm::vec3(1, 0, -200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// wall 3
	std::cout << "wall 3" << std::endl;
	entity = &world->entities[world->numEntities++];

	min = glm::vec3(0, -50, -401);
	max = glm::vec3(200, 0, -400);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// plane 4
	std::cout << "plane 4" << std::endl;
	entity = &world->entities[world->numEntities++];

	min = glm::vec3(-100, -51, -600);
	max = glm::vec3(200, -50, -400);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);



	// plane 4 wall 4
	std::cout << "plane 4 wall 4" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	dim = glm::vec3(100, 1, 100);

	min = glm::vec3(-101, -50, -600);
	max = glm::vec3(-100, 100, -200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// plane 4 wall 5
	std::cout << "plane 4 wall 5" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	dim = glm::vec3(100, 1, 100);

	min = glm::vec3(200, -50, -600);
	max = glm::vec3(201, 100, -200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);



	// plane 4 door 5
	std::cout << "plane 4 door 5" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	min = glm::vec3(-100, -50, -601);
	max = glm::vec3(0, 100, -600);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	min = glm::vec3(100, -50, -601);
	max = glm::vec3(200, 100, -600);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);

	
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	min = glm::vec3(0, 25, -601);
	max = glm::vec3(100, 100, -600);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
	*/


	// concentric circle 1
	Pattern pattern;
	pattern.Init();
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(0, 0, 0);

	faces = PatternToFaces(&pattern);
	initEntity(entity, pos, faces);
	entity->IsPatternCircle = true;
}




void CreateAreaB(World* world, std::vector<Brush>& brushes)
{
	Entity* entity = NULL;
	std::vector<Face> faces;

	// lower level
	// Box
	entity = &world->entities[world->numEntities++];
	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 min;
	glm::vec3 max;



	// plane 1
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(0, 0, 0);
	dim = glm::vec3(200, 1, 50);

	min = glm::vec3(-200, 0, -200);
	max = glm::vec3(200, 25, 200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);

	// bottom wall
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(0, 0, 0);

	min = glm::vec3(-200, 0, -200);
	max = glm::vec3(200, 100, -175);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// left wall
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(0, 0, 0);
	dim = glm::vec3(200, 1, 50);

	min = glm::vec3(-200, 0, -200);
	max = glm::vec3(-175, 100, 200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// top wall
	std::cout << "plane 1 wall 3" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(0, 0, 0);
	dim = glm::vec3(200, 1, 50);

	min = glm::vec3(-200, 0, 175);
	max = glm::vec3(200, 100, 200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);



	// right wall
	std::cout << "plane 1 wall 4" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(50, 0, 0);
	dim = glm::vec3(100, 1, 100);

	min = glm::vec3(175, 0, -200);
	max = glm::vec3(200, 100, 200);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);


	// in the middle
	std::cout << "plane 1 wall 4" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(-50, 0, 0);
	dim = glm::vec3(100, 1, 100);

	min = glm::vec3(-50, 0, -50);
	max = glm::vec3(50, 50, 50);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
}




void CreateAreaC(World* world, std::vector<Brush>& brushes)
{
	Entity* entity = NULL;
	std::vector<Face> faces;

	// lower level
	// Box
	entity = &world->entities[world->numEntities++];
	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 min;
	glm::vec3 max;


	// in the middle
	std::cout << "plane 1 wall 4" << std::endl;
	entity = &world->entities[world->numEntities++];
	pos = glm::vec3(-50, 0, 0);
	dim = glm::vec3(100, 1, 100);

	min = glm::vec3(-50, 0, -50);
	max = glm::vec3(50, 50, 50);

	faces = CreateCubeFaceMinMax(min, max);
	brushes.push_back(ConvertFaceToBrush(faces));
	initEntity(entity, pos, faces);
}



// this contains the result when a box is swept through the world
struct TraceResult
{
	float timeFraction; // 0 ~ 1;
	glm::vec3 endPos;
	bool outputStartsOut;	// True if the line segment starts outside of a solid volume.
	bool outputAllSolid;	// True if the line segment is completely enclosed in a solid volume.
							// meaning both start and end are inside a brush

	Plane plane;			// surface normal at impact;
	Entity* entity;			// ground entity
};


struct TraceSetupInfo
{
	glm::vec3 mins;
	glm::vec3 maxs;
	glm::vec3 traceExtends;
	bool isTraceBoxAPoint;	// does min == max;
};


void CheckBrush(Brush* brush, glm::vec3 start, glm::vec3 end, TraceResult* result, TraceSetupInfo* setupInfo)
{
	if (brush->polygons.size() == 0)
	{
		return;
	}

	float startFraction = -1;
	float endFraction = 1;
	bool startsOut = false;
	bool endsOut = false;
	Plane* clipPlane = NULL;

	glm::vec3 offsets;
//	std::cout << ">>>>>>> CheckBrush" << std::endl;
	for (int i = 0; i < brush->polygons.size(); i++)
	{
		BspPolygon polygon = brush->polygons[i];
		Plane plane = polygon.plane;

		float startToPlaneDist = 0;
		float endToPlaneDist = 0;

		//std::cout << "plane normal " << plane.normal << " dist " << plane.distance << std::endl;
		if (plane.normal.x == 0 && plane.normal.y == 0 && plane.normal.z == -1)
		{
	//		std::cout << "plane dist " << plane.distance << std::endl;
		}
		
		for (int j = 0; j < 3; j++)
		{
			if (plane.normal[j] < 0)
			{
				offsets[j] = setupInfo->maxs[j];
			}
			else
			{
				offsets[j] = setupInfo->mins[j];
			}
		}

		startToPlaneDist = (start[0] + offsets[0]) * plane.normal[0] +
					(start[1] + offsets[1]) * plane.normal[1] +
					(start[2] + offsets[2]) * plane.normal[2] - plane.distance;

		endToPlaneDist = (end[0] + offsets[0]) * plane.normal[0] +
					(end[1] + offsets[1]) * plane.normal[1] +
					(end[2] + offsets[2]) * plane.normal[2] - plane.distance;

	//	std::cout << "	startToPlaneDist " << startToPlaneDist << std::endl;
	//	std::cout << "	endToPlaneDist " << endToPlaneDist << std::endl;


		if (startToPlaneDist > 0)
		{
			startsOut = true;
		}
		if (endToPlaneDist > 0)
		{
			endsOut = true;
		}

		/*
		if its completely in front, we return
				 _______
				|		|
				|		|
 ---->		<---|		|
				|		|
				|_______|
		
		*/

		// makesure the trace isn't completely 
		if (startToPlaneDist > 0 && endToPlaneDist > 0)
		{
			return;
		}


		/*
		both are behind this plane, it will get clipped by another one
				 _______
				|		|
				|		|
			<---|  ------->
				|		|
				|_______|
		*/
		if (startToPlaneDist <= 0 && endToPlaneDist <= 0)
		{
			continue;
		}

		// crosses face

		/*
		startDist > endDist means line is entering into the brush. See graph below
		startDist is > endDist cuz its further along the plane normal direction
				 _______
				|		|
	 	----------->	|
      		<---|		|
				|		|
				|_______|


		startDist > endDist means line is entering into the brush. See graph below
		startDist is > endDist cuz its further along the plane normal direction
				 _______
				|		|
	 			|	----------->	
      			|		|--->
				|		|
				|_______|
		*/
		if (startToPlaneDist > endToPlaneDist)
		{
			// line is entering into the brush
			float fraction = (startToPlaneDist - DIST_EPSILON) / (startToPlaneDist - endToPlaneDist);
			if (fraction > startFraction)
			{
				startFraction = fraction;
				clipPlane = &plane;
			}
		}
		else
		{
			// line is leaving the brush
			float fraction = (startToPlaneDist + DIST_EPSILON) / (startToPlaneDist - endToPlaneDist);
			if (fraction < endFraction)
			{
				endFraction = fraction;
			}
		}
	}

	if (startsOut == false)
	{
		result->outputStartsOut = false;
		if (endsOut == false)
		{
			result->outputAllSolid = true;
		}
		return;
	}

	if (startFraction < endFraction)
	{
		// -1 is just the default value. we just want to check if startFraction
		// has been set a proper value.
		if (startFraction > -1 && startFraction < result->timeFraction)
		{
			if (startFraction < 0)
				startFraction = 0;
			result->timeFraction = startFraction;
			result->plane = *clipPlane;
		}
	}


}

void TraceToLeafNode(BSPNode* node, glm::vec3 start, glm::vec3 end, TraceResult* result, TraceSetupInfo* setupInfo)
{
	if (node->brushes.size() == 0)
	{
		return;
	}

	if (node->IsLeafNode())
	{
		for (int i = 0; i < node->brushes.size(); i++)
		{
			CheckBrush(&node->brushes[i], start, end, result, setupInfo);
		
			if (result->timeFraction == 0)
				return;
		}
	}
}





void RecursiveHullCheck(BSPNode* node, float startFraction, float endFraction,
	glm::vec3 start, glm::vec3 end,
	glm::vec3 traceStart, glm::vec3 traceEnd,
	TraceResult* result, TraceSetupInfo* setupInfo, bool print = false)
{


	if (print)
	{
		std::cout << "visiting node " << node->id << std::endl;
	}

	// already hit something nearer
	if (result->timeFraction <= startFraction)
	{
		return;
	}

//	std::cout << "visiting node " << node->id << std::endl;
//	std::cout << "startFraction " << startFraction << ", endFraction " << endFraction << std::endl;

	if (node->IsLeafNode())
	{
		TraceToLeafNode(node, traceStart, traceEnd, result, setupInfo);
		return;
	}

	Plane plane = node->splitPlane;
//	std::cout << "		plane " << plane.normal << std::endl;

	float startDist, endDist, offset;
	if (IsAxialPlane(plane))
	{
		// optimize this
		startDist = glm::dot(start, plane.normal) - plane.distance;
		endDist = glm::dot(end, plane.normal) - plane.distance;
		offset = fabs(setupInfo->traceExtends[0] * plane.normal[0]) +
			fabs(setupInfo->traceExtends[1] * plane.normal[1]) +
			fabs(setupInfo->traceExtends[2] * plane.normal[2]);
	}
	else
	{
		// optimize this
		startDist = glm::dot(start, plane.normal) - plane.distance;
		endDist = glm::dot(end, plane.normal) - plane.distance;
		offset = glm::dot(plane.normal, setupInfo->traceExtends);
		
		if (setupInfo->isTraceBoxAPoint)
		{
			offset = 0;
		}
		else
		{
			// similar to 5.2.3 Testing Box Against Plane
			offset = fabs(setupInfo->traceExtends[0] * plane.normal[0]) +
				fabs(setupInfo->traceExtends[1] * plane.normal[1]) +
				fabs(setupInfo->traceExtends[2] * plane.normal[2]);
		}
	}

	if (print && node->id == 7)
	{
		std::cout << "visiting node " << node->id << std::endl;
	}

	if (startDist >= offset && endDist >= offset)
	{
		RecursiveHullCheck(node->children[0], startFraction, endFraction, start, end, 
			traceStart, traceEnd, result, setupInfo, print);
		return;
	}
	if (startDist < -offset && endDist < -offset)
	{
		RecursiveHullCheck(node->children[1], startFraction, endFraction, start, end, 
			traceStart, traceEnd, result, setupInfo, print);
		return;
	}

	// the side that the start is on. 
	int side;
	float fraction1, fraction2, middleFraction;
	glm::vec3 middlePoint;
	// 1/32 epsilon to keep floating point happy


	/*
	the case where endDist > startDist

				      plane
						
			front side	|  back side
						|
				end		|    start
			<-----------|
						|
						|


	the case where startDist > endDist

					  plane

			front side	|  back side
						|
				start	|    end
			<-----------|
						|
						|

	*/
	if (startDist < endDist)
	{
		side = 1;	// start is on the back of the plane
		float inverseDistance = 1.0f / (startDist - endDist);
		fraction1 = (startDist - offset + DIST_EPSILON) * inverseDistance;
		fraction2 = (startDist + offset + DIST_EPSILON) * inverseDistance;
	}
	else if (startDist > endDist)
	{
		side = 0;	// start is on the front side of the plane
		float inverseDistance = 1.0f / (startDist - endDist);
		fraction1 = (startDist + offset + DIST_EPSILON) * inverseDistance;
		fraction2 = (startDist - offset - DIST_EPSILON) * inverseDistance;

	//	std::cout << "fraction1 " << fraction1 << std::endl;
	//	std::cout << "fraction2 " << fraction2 << std::endl;
	}
	else
	{
		side = 0;
		fraction1 = 1.0f;
		fraction2 = 0.0f;
	}

	// examine [start  middle]
	if (fraction1 < 0) { fraction1 = 0;	}
	else if (fraction1 > 1) { fraction1 = 1; }

	middleFraction = startFraction + (endFraction - startFraction) * fraction1;
	middlePoint = start + fraction1 * (end - start);

	RecursiveHullCheck(node->children[side], startFraction, middleFraction, 
												start, middlePoint, traceStart, traceEnd, 
												result, setupInfo, print);

	// examine [middle	end]
	if (fraction2 < 0) { fraction2 = 0; }
	else if (fraction2 > 1) { fraction2 = 1; }

	middleFraction = startFraction + (endFraction - startFraction) * fraction2;
	middlePoint = start + fraction2 * (end - start);

	RecursiveHullCheck(node->children[!side], middleFraction, endFraction, 
												middlePoint, end, traceStart, 
												traceEnd, result, setupInfo, print);
}




// Cloning cmodel.c
TraceResult BoxTrace(glm::vec3 start, glm::vec3 end, glm::vec3 mins, glm::vec3 maxs, BSPNode* tree, bool print = false)
{
	TraceResult result = {};
	result.outputStartsOut = true;
	result.outputAllSolid = false;
	result.plane = NULL_PLANE;

	TraceSetupInfo setup = {};

	result.timeFraction = 1;

	setup.mins = mins;
	setup.maxs = maxs;

	if (start == end)
	{

	}

	if (mins == maxs)
	{
		setup.isTraceBoxAPoint = true;
	}
	else
	{
		setup.isTraceBoxAPoint = false;

		// getting the largest dimension in of min or max
		// essentially we are comparing -min[i] and max[i]
		setup.traceExtends[0] = -mins[0] > maxs[0] ? -mins[0] : maxs[0];
		setup.traceExtends[1] = -mins[1] > maxs[1] ? -mins[1] : maxs[1];
		setup.traceExtends[2] = -mins[2] > maxs[2] ? -mins[2] : maxs[2];
	}

	RecursiveHullCheck(tree, 0, 1, start, end, start, end, &result, &setup, print);

	if (result.timeFraction == 1)
	{
		result.endPos = end;
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			result.endPos[i] = start[i] + result.timeFraction * (end[i] - start[i]);
		}
	}

	return result;
}










// Essentially recreating a simplified version of dust2
void initWorld(World* world)
{
	// initlaize the game state  
	world->numEntities = 0;
	world->maxEntityCount = 1024;
	/*
	for (int i = 0; i < world->entityCount; i++)
	{
		world->entities[i].pos = glm::vec3(5 - i, 5 - i, 5 - i);
		world->entities[i].dim = glm::vec3(1);
		//			cout << i << ": " << gameState->entities[i].pos << std::endl;
	}
	*/

	NULL_PLANE = Plane();
	NULL_PLANE.normal = glm::vec3(0);

	std::vector<Brush> brushes;


	float wallHeight = 50;

	CreateAreaA(world, brushes);
	// CreateAreaC(world, brushes);

	// glm::vec3 siteBSize = glm::vec3(200, wallHeight, 200);
	//CreateAreaB(world, glm::vec3(500, 0, 0), siteBSize, brushes);
	


	std::cout << "############# BuildBSPTree" << std::endl;
	world->tree = BuildBSPTree(brushes, 0);

	std::cout << "############# PrintBSPTree" << std::endl;
	PrintBSPTree(world->tree, 0);




	world->startPlayerEntityId = world->numEntities;
	Entity* entity = &world->entities[world->numEntities++];
	glm::vec3 pos = glm::vec3(-50, 11, -12);
//	glm::vec3 pos = glm::vec3(-50, 1, -50);
	initPlayerEntity(entity, pos);
}