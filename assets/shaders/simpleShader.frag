#version 450
#extension GL_ARB_separate_shader_objects : enable

const int SPHERES_LENGTH = 7;
const int VERTS_LENGTH = 5000;
const int TRIANGLES_LENGTH = 5000;
const int LIGHTS_LENGTH = 5;
const int NODES_LENGTH = (2 * 7) - 1;
const int SIZE_BVH_ARR = 30000;//(1 << VERTS_LENGTH) - 1;
 uint SAMPLES_PER_PIXEL = 4;
const uint MAX_DEPTH = 4;
const float PI = 3.14f;
const float MAX = 9999;


struct Material
{
	vec4 color;
	vec4 emitted;
	float specularity;
	float reflectance;
	float refractiveIndex;
};

struct Sphere
{
	vec4 center;
	float radius;
	Material material;
};

struct Triangle // expects CW winding order
{
	// positions
	int v0;
	int v1;
	int v2;
	// normals
	int n0;
	int n1;
	int n2;
	// uv coordinates
	int t0;
	int t1;
	int t2;
	Material material;
};

struct Camera
{
	vec4 position;
	vec4 up;
	vec4 forward;
	vec4 right;
	
	int pixelWidth;
	int pixelHeight;

	float focalPlaneWidth;
	float focalPlaneHeight;
	vec4 lowerLeftCorner; // position;
};

struct BVHStack
{
	int nodeIndices[NODES_LENGTH];
	int top;
};

struct BVHNode
{
	vec4 corner1;
	vec4 corner2;
	int primitiveIndex;
};

struct HitInfo
{
	bool hit;
	bool frontFacing;
	vec3 hitPoint;
	vec3 normal;
	Material material;
	vec3 scatterDirection;
	vec2 uv;
	float PDF;
};

layout(binding = 0) uniform UniformBufferObject
{
	uint RNGSeed;
	Camera camera;
};

layout(binding = 1) readonly buffer StorageBufferObject
{
	Sphere spheres[SPHERES_LENGTH];
	BVHNode BVHNodes[SIZE_BVH_ARR];
	vec4 vertPositions[VERTS_LENGTH];
	vec4 vertNormals[VERTS_LENGTH];
	vec4 vertUVCoords[VERTS_LENGTH];
	Triangle triangles[TRIANGLES_LENGTH];
	uint numTriangles;

	BVHNode lightBVHNode;
	float lightArea;
};

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;


float G_RAND_VAR;// = rand(gl_FragCoord.xy);
uint g = 1;

//https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-37-efficient-random-number-generation-and-application
// S1, S2, S3, and M are all constants, and z is part of the
// private per-thread generator state.
uint TausStep(uint z, int S1, int S2, int S3, uint M)
{
	uint b = (((z << S1) ^ z) >> S2);
	return z = (((z & M) << S3) ^ b);
}

//https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-37-efficient-random-number-generation-and-application
// A and C are constants
uint LCGStep(uint z, uint A, uint C)
{
	return z=(A*z+C);
}

float gFloat= 1.0f;

//https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-37-efficient-random-number-generation-and-application
uint z1, z2, z3, z4;
float HybridTaus()
{
	// Combined period is lcm(p1,p2,p3,p4)~ 2^121
	float result = 2.3283064365387e-10 * (
		// Periods
		TausStep(z1, 13, 19, 12, 4294967294) ^  // p1=2^31-1
		TausStep(z2, 2, 25, 4, 4294967288) ^    // p2=2^30-1
		TausStep(z3, 3, 11, 17, 4294967280) ^   // p3=2^28-1
		LCGStep(z4, 1664525, 1013904223)        // p4=2^32
		);

	z1 = uint(result * 1234);
	return result;
	} 


//https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co)
{
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

uint rand2()
{
	return g = TausStep(g, int(gl_FragCoord.x), int(gl_FragCoord.y), 71, 81731);
}

int nodeIndices[NODES_LENGTH];
BVHStack G_BVH_STACK = BVHStack(nodeIndices, 0);

//https://stackoverflow.com/questions/1026327/what-common-algorithms-are-used-for-cs-rand
// [0,1)
float getRand()
{
	//return G_RAND_VAR = rand(vec2(G_RAND_VAR + 7, rand(vec2(gl_FragCoord.xy * (G_RAND_VAR + 12)))));
	//return G_RAND_VAR = 0;

	uint next = g;
	int result;

	next *= 1103515245;
	next += 12345;
	result = int(next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= int(next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= int(next / 65536) % 1024;

	g = next;

	return (result % 100) / 99.0f;
}

uint seedRand(vec2 coord)
{
	z1 = z3 = uint(coord.x);
	z2 = z4 = uint(coord.y);

	g = uint((HybridTaus() * 1000)) % 1000;
	return g;
}

//https://math.stackexchange.com/questions/1585975/how-to-generate-random-points-on-a-sphere
vec3 randInSphere(float seed)
{
	float seed2 = rand(vec2(seed,seed));
	float seed3 = rand(vec2(seed2,seed2));
	//return (vec3(rand(gl_FragCoord.xy + seed), rand(gl_FragCoord.xy + seed2), rand(gl_FragCoord.xy + seed3)) - 0.5f) + I_w.xyz;

	float lambda = acos(2 * getRand() - 1) - (PI / 2);
    float phi = getRand() * 2 * PI;
    float r = getRand();

    float radius = cos(lambda);
    return vec3(cos(phi) * radius, sin(phi) * radius, sin(lambda));

}

float getRandInRange(float start, float end)
{
	return getRand() * (end - start) + start;
}

vec3 randomCosineDirection()
{
	float r1 = getRand();
	float r2 = getRand();
	float z = sqrt(1 - r2);

	float phi = 2 * PI * r1;
	float x  = cos(phi) * sqrt(r2);
	float y = sin(phi) * sqrt(r2);

	return vec3(x, y, z);
}

vec3 rayAt(vec3 origin, vec3 dir, float t)
{
	return origin + t * normalize(dir);
}

int popStack()
{
	if (G_BVH_STACK.top <= 0)
	{
		return -1;
	}
	else
	{
		return G_BVH_STACK.nodeIndices[--G_BVH_STACK.top];
	}
}

void pushStack(int element)
{
	G_BVH_STACK.nodeIndices[G_BVH_STACK.top++] = element;
}

bool stackEmpty(BVHStack stack)
{
	return stack.top <= 0;
}

int getLeftChild(int idx)
{
	return (idx * 2) + 1;
}

int getRightChild(int idx)
{
	return (idx * 2) + 2;
}

//https://raytracing.github.io/books/RayTracingTheNextWeek.html#boundingvolumehierarchies/anoptimizedaabbhitmethod
bool hitAABB(BVHNode node, vec3 rayOrigin, vec3 rayDirection)
{
	float tmin = 0;
	float tmax = MAX;
	for (int axis = 0; axis < 3; axis++)
	{
		float t0 = min((node.corner1[axis] - rayOrigin[axis]) / rayDirection[axis],
						(node.corner2[axis] - rayOrigin[axis]) / rayDirection[axis]);
		float t1 = max((node.corner1[axis] - rayOrigin[axis]) / rayDirection[axis],
						(node.corner2[axis] - rayOrigin[axis]) / rayDirection[axis]);
		tmin = max(t0, tmin);
		tmax = min(t1, tmax);
		if (tmax <= tmin)
		{
			return false;
		}
	}

	return true;
}




// returns a or b randomly, weighing a with probability/ probability = 1 means to always select a. probability = 0 means always b
vec3 selectVector(vec3 a, vec3 b, float probability)
{
	// what is the cost of ceil()? alternative solution: choose random number x; 0 <= x <= 1; result = x - probability; return options[overflow/negative bit]
	vec3 options[2] = {a, b};
	return options[int(ceil(getRand() - probability))];
}

// Schlick approximation for transparent surfaces reflectance
float schlick(float cosTheta, float etaiOverEtat)
{
	float r0 = (1 - etaiOverEtat) / (1 + etaiOverEtat);
	r0 = r0 * r0;
	return r0 + (1-r0) * pow(1 - cosTheta, 5);
}

// returns defaultReflect if refraction is not possible
vec3 calculateRefractionDirection(vec3 inDirection, vec3 localNormal, float cosTheta, float etaiOverEtat, vec3 defaultReflect)
{
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	


	vec3 resultPerpindicular = etaiOverEtat * (inDirection + cosTheta * localNormal);
	vec3 resultParallel = -sqrt(abs(1.0f - dot(resultPerpindicular, resultPerpindicular))) * localNormal;

	if (etaiOverEtat * sinTheta > 1.0f)
	{
		return defaultReflect;
	}
	else
	{
		return resultPerpindicular + resultParallel;
	}
}

vec3 shiftBasis(vec3 unshiftedInput, vec3 basisW)
{
	vec3 a = abs(basisW.x) > 0.9 ? vec3(0,1,0) : vec3(1,0,0);
	vec3 v = normalize(cross(basisW, a));
	vec3 u = cross(basisW, v);

	return (unshiftedInput.x * u) + (unshiftedInput.y * v) + (unshiftedInput.z * basisW);
}

vec3 getNewDirection(Material material, vec3 inDirection, vec3 normal, bool frontFacing)
{
	// Calculate the new direction of the ray for lambertian scatter and for specular scatter.
	// lerp between them according to the sphere's specularity.
	// assumes specularty is between 0 and 1
	//vec3 diffuseReflection = (normal) + randInSphere(getRand());
	vec3 diffuseReflection = normal + shiftBasis(randomCosineDirection(), normal);
	vec3 specularReflection = inDirection - (2 * dot(inDirection, normal) * normal);
	vec3 reflectionDirection = mix(diffuseReflection, specularReflection, material.specularity);

	//frontFacing = dot(inDirection, normal) < 0;

	float etaiOverEtat = 0;
	vec3 localNormal = normal;// * (-1 * int(!frontFacing)); // localNormal always points towards the inDirection. Important when a ray is inside of a material
	if (frontFacing)
	{
		etaiOverEtat = 1 / material.refractiveIndex;
	}
	else
	{
		etaiOverEtat = material.refractiveIndex;
		localNormal *= -1;
	}
	float cosTheta = dot(-inDirection, localNormal);
	vec3 refractionDirection = calculateRefractionDirection(inDirection, localNormal, cosTheta, etaiOverEtat, reflectionDirection);

	return selectVector(reflectionDirection, refractionDirection, int(frontFacing) * schlick(cosTheta, etaiOverEtat));
}

// point length one, centered at the origin
vec2 getSphereUV(vec3 point)
{
	float theta = acos(-point.y);
	float phi = atan(-point.z, point.x) + PI;

	return vec2(phi / (2 * PI), theta / PI);
}

// returns point along ray intersection occurs
HitInfo hitSphere(int sphereIndex, vec3 rayOrigin, vec3 rayDirection)
{
	HitInfo result;
	result.hit = false;

	vec3 center = spheres[sphereIndex].center.xyz;
	float radius = spheres[sphereIndex].radius;
	vec3 oc = rayOrigin - center;
	float a = dot(rayDirection, rayDirection);
	float halfB = dot(oc, rayDirection);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = halfB*halfB - a*c;
	if (discriminant >= 0)
	{
		//return (-halfB + sqrt(discriminant)) / a;
		float solution1 = (-halfB - sqrt(discriminant)) / a;
		float solution2 = (-halfB + sqrt(discriminant)) / a;
		if (solution1 > 0)
		{
			result.hit = true;
			result.frontFacing = true;
			result.hitPoint = rayAt(rayOrigin, rayDirection, solution1);
			result.normal = normalize(result.hitPoint - spheres[sphereIndex].center.xyz);
			result.material = spheres[sphereIndex].material;
		}
		else if (solution2 > 0)
		{
			result.hit = true;
			result.frontFacing = false;
			result.hitPoint = rayAt(rayOrigin, rayDirection, solution2);
			result.normal = normalize(result.hitPoint - spheres[sphereIndex].center.xyz);
			result.material = spheres[sphereIndex].material;
		}
	}

	result.scatterDirection = getNewDirection(spheres[sphereIndex].material, rayDirection, result.normal, dot(result.normal, rayDirection) < 0);

	result.uv = getSphereUV(result.normal);
	//result.uv = vec2(0.4f,0.1f);

	result.PDF = max(0, dot(result.scatterDirection, result.normal) / PI);

	return result;
}

// returns index of closest hit sphere or -1 if no hits
HitInfo hitSpheres(vec3 rayOrigin, vec3 rayDirection)
{
	HitInfo closest;
	closest.hit = false;
	closest.hitPoint = vec3(MAX,MAX,MAX);
	float closestT = -1;

	for (int i = 0; i < SPHERES_LENGTH; i++)
	{
		HitInfo tempHitInfo = hitSphere(i, rayOrigin, rayDirection);
			if (tempHitInfo.hit)
			{
				float tempT = dot(tempHitInfo.hitPoint, rayDirection);
				if (closestT == -1 || closestT > tempT)
				{
					closest = tempHitInfo;
					closestT = tempT;
				}
			}
	}

	return closest;

	/*

	pushStack(0); // start traversal by adding root to stack
	while(!stackEmpty(G_BVH_STACK))
	{
		int nodeIdx = popStack();
		BVHNode node = BVHNodes[nodeIdx];
		if (node.primitiveIndex != -1)
		{
			//return 1; // remove
			float tempT = hitSphere(node.primitiveIndex, rayOrigin, rayDirection);
			if (tempT > 0)
			{
				if (tempT > 0 && (closestT == -1 || closestT > tempT))
				{
					indexOfClosestSphere = node.primitiveIndex;
					closestT = tempT;
				}
			}
		}
		else if (hitAABB(node, rayOrigin, rayDirection))
		{
			pushStack(getRightChild(nodeIdx));
			pushStack(getLeftChild(nodeIdx));
		}
	}
	return indexOfClosestSphere;
	
	*/
}




//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
HitInfo hitTriangle(int triangleIdx, vec3 origin, vec3 dir)
{
	Triangle tri = triangles[triangleIdx];

	vec3 v0v1 = vertPositions[tri.v1].xyz - vertPositions[tri.v0].xyz;
	vec3 v0v2 = vertPositions[tri.v2].xyz - vertPositions[tri.v0].xyz;
	vec3 pvec = cross(dir, v0v2);
	float determinant = dot(v0v1, pvec);
	float invertedDeterminant = 1 / determinant;

	vec3 tvec = origin - vertPositions[tri.v0].xyz;
	float u = dot(tvec, pvec) * invertedDeterminant;
	vec3 qvec = cross(tvec, v0v1);

	float v = dot(dir, qvec) * invertedDeterminant;
	float t = dot(v0v2, qvec) * invertedDeterminant;
	float w = (1 - v - u);

	vec3 normal = -cross(v0v2, v0v1);
	normal = normalize(normal);
	vec3 interpolatedNormal = normalize((w * vertNormals[tri.n0].xyz) + (u * vertNormals[tri.n1].xyz) + (v * vertNormals[tri.n2].xyz));

	vec3 scatterDirection = getNewDirection(triangles[triangleIdx].material, dir, interpolatedNormal, dot(normal, dir) < 0);
	

	return  HitInfo(
			u > 0 && v > 0 && u + v < 1.0f &&
			t > 0.001f,
			dot(dir, normal) < 0,
			//&& dot(normal, interpolatedNormal) > 0,
			rayAt(origin, dir, t),
			interpolatedNormal,
			//normal,
			//interpolatedNormal - normal,
			triangles[triangleIdx].material,
			scatterDirection,
			(w * vertUVCoords[tri.t0].xy) + (u * vertUVCoords[tri.t1].xy) + (v * vertUVCoords[tri.t2].xy),
			max(0, dot(scatterDirection, interpolatedNormal) / PI));
}


// Returns hitInfo of closest triangle touched
HitInfo hitTriangles(vec3 rayOrigin, vec3 rayDirection)
{
	HitInfo closest;
	closest.hit = false;
	closest.hitPoint = vec3(MAX,MAX,MAX);
	float closestT = -1;

	pushStack(0); // start traversal by adding root to stack
	while(!stackEmpty(G_BVH_STACK))
	{
		int nodeIdx = popStack();
		BVHNode node = BVHNodes[nodeIdx];
		if (node.primitiveIndex != -1)
		{
			HitInfo tempHitInfo = hitTriangle(node.primitiveIndex, rayOrigin, rayDirection);
			if (tempHitInfo.hit)
			{
				float tempT = dot(tempHitInfo.hitPoint, rayDirection);
				if (closestT == -1 || closestT > tempT)
				{
					closest = tempHitInfo;
					closestT = tempT;
				}
		}
		}
		if (hitAABB(node, rayOrigin, rayDirection))
		{
			pushStack(getRightChild(nodeIdx));
			pushStack(getLeftChild(nodeIdx));
		}
	}

	return closest;

	/*
	HitInfo closest;
	closest.hit = false;
	closest.hitPoint = vec3(MAX,MAX,MAX);
	float closestT = -1;
	for(int i = 0; i < numTriangles; i++)
	{
		HitInfo tempHitInfo = hitTriangle(i, rayOrigin, rayDirection);
		if (tempHitInfo.hit)
		{
			float tempT = dot(tempHitInfo.hitPoint, rayDirection);
			if (closestT == -1 || closestT > tempT)
			{
				closest = tempHitInfo;
				closestT = tempT;
			}
		}
	}

	return closest;
	*/
}

vec3 getColorBoxes(vec3 origin, vec3 dir)
{
	vec3 colorAccumulator = vec3(0,0,0);
	int numIterations = 0;


	pushStack(0); // start traversal by adding root to stack
	while(!stackEmpty(G_BVH_STACK))
	{
		int nodeIdx = popStack();
		BVHNode node = BVHNodes[nodeIdx];
		if (node.primitiveIndex != -1)
		{
			HitInfo tempInfo = hitTriangle(node.primitiveIndex, origin, dir);
			if (tempInfo.hit)
			{
				colorAccumulator += vec3(1,1,1);
				numIterations++;
			}
		}
		if (hitAABB(node, origin, dir))
		{
			colorAccumulator += vec3(rand(vec2(nodeIdx, nodeIdx)), rand(vec2(nodeIdx * nodeIdx, nodeIdx)), rand(vec2(nodeIdx, nodeIdx * nodeIdx)));
			numIterations++;
			pushStack(getRightChild(nodeIdx));
			pushStack(getLeftChild(nodeIdx));
		}
	}

	return colorAccumulator / numIterations;
}

// returns hitInfo of closest primitive hit
HitInfo hitScene(vec3 rayOrigin, vec3 rayDirection)
{
	//return hitTriangles(rayOrigin, rayDirection);
	HitInfo result;
	result.hit = false;
	HitInfo spheresHitInfo = hitSpheres(rayOrigin, rayDirection);
	HitInfo trianglesHitInfo = hitTriangles(rayOrigin, rayDirection);

	float spheresT = MAX;
	if (spheresHitInfo.hit)
	{
		spheresT = dot(spheresHitInfo.hitPoint, rayDirection);
	}
	float trianglesT = MAX;
	if (trianglesHitInfo.hit)
	{
		trianglesT = dot(trianglesHitInfo.hitPoint, rayDirection);
	}
	if (spheresHitInfo.hit && (spheresT <= trianglesT))
	{
		result = spheresHitInfo;
	}
	else if (trianglesHitInfo.hit && (spheresT > trianglesT))
	{
		result = trianglesHitInfo;
	}

	return result;
}

float scatteringPDF(vec3 outDir, vec3 normal)
{
	return max(0, dot(outDir, normal) / PI);
}


// TODO using this results in a scene that's too dark? Evidently, the rays that go towards the light are weighed too low. The return value should be closer to 0
float getLightPDF(vec3 rayOrigin, vec3 lightNormal)
{
	vec3 onLight = vec3(getRandInRange(lightBVHNode.corner1.x, lightBVHNode.corner2.x), getRandInRange(lightBVHNode.corner1.y, lightBVHNode.corner2.y), getRandInRange(lightBVHNode.corner1.z, lightBVHNode.corner2.z));
	vec3 toLight = onLight - rayOrigin;
	float distanceSquared = dot(toLight, toLight);
	toLight = normalize(toLight);

	return distanceSquared / (lightArea);
}

vec3 getDirectionToLight(vec3 rayOrigin, vec3 surfaceNormal)
{
	vec3 onLight = vec3(getRandInRange(lightBVHNode.corner1.x, lightBVHNode.corner2.x), getRandInRange(lightBVHNode.corner1.y, lightBVHNode.corner2.y), getRandInRange(lightBVHNode.corner1.z, lightBVHNode.corner2.z));
	vec3 toLight = onLight - rayOrigin;
	return toLight;
}

vec3 getColor(vec3 origin, vec3 dir)
{
	vec3 emissionStack[MAX_DEPTH];
	vec3 attenuationStack[MAX_DEPTH];
	float samplingPDFStack[MAX_DEPTH];
	float scatteringPDFStack[MAX_DEPTH];

	for(int i = 0; i < MAX_DEPTH; i++)
	{
		emissionStack[i] = vec3(0,0,0);
		attenuationStack[i] = vec3(1,1,1);
		samplingPDFStack[i] = 1;
		scatteringPDFStack[i] = 1;
	}


	bool miss = false;
	vec3 normal;
	float t = 0;
	for(int i = 0; !miss && i < MAX_DEPTH; i++)
	{
		HitInfo hitInfo = hitScene(origin, dir);
		if(hitInfo.hit)
		{
			if(hitInfo.material.specularity > 0 || hitInfo.material.refractiveIndex > 0)
			{
				dir = hitInfo.scatterDirection;
				origin = hitInfo.hitPoint + dir * 0.01f; // make sure the new ray isn't stuck in the object

				samplingPDFStack[i] = 1;
				emissionStack[i] = hitInfo.material.emitted.xyz * int(hitInfo.frontFacing);
				attenuationStack[i] = hitInfo.material.color.xyz;// * texture(texSampler, hitInfo.uv).xyz;
				scatteringPDFStack[i] = 1;
			}
			else
			{
				if (getRand() > 0.5f)
				{
					dir = hitInfo.scatterDirection;
				}
				else
				{
					dir = getDirectionToLight(hitInfo.hitPoint, hitInfo.normal);
				}

				origin = hitInfo.hitPoint + dir * 0.01f; // make sure the new ray isn't stuck in the object

				//samplingPDFStack[i] = getLightPDF(origin, hitInfo.normal);

				samplingPDFStack[i] = 0.5f * getLightPDF(origin, hitInfo.normal) + 0.5f * hitInfo.PDF;

				emissionStack[i] = hitInfo.material.emitted.xyz * int(hitInfo.frontFacing);
				attenuationStack[i] = hitInfo.material.color.xyz;// * texture(texSampler, hitInfo.uv).xyz;
				//samplingPDFStack[i] = hitInfo.PDF;
				scatteringPDFStack[i] = scatteringPDF(dir, hitInfo.normal);

			
				}
				
			}

		else
			{
				//return dir;
				miss = true;
				emissionStack[i] = vec3(0,0,0);
				attenuationStack[i] = vec3(1,1,1);
			}
	}

	//return vec3(samplingPDFStack[0], samplingPDFStack[0], samplingPDFStack[0]);

	vec3 colorAccumulator = inColor;
	for(int i = int(MAX_DEPTH) - 1; i >= 0; i--)
	{
		colorAccumulator = colorAccumulator * attenuationStack[i] * scatteringPDFStack[i] / samplingPDFStack[i] + emissionStack[i];
	}

	return colorAccumulator;
}



void main()
{
	//outColor = outColor = texture(texSampler, gl_FragCoord.xy / vec2(camera.pixelWidth, camera.pixelHeight));
	//outColor = vec4(1,1,1,1);
	//outColor = vec4(1,1,1,1) * numTriangles / 888;
	//outColor = camera.position;
	//outColor = vec4(1,1,1,1) * spheres[1].radius;
	//outColor = lightBVHNode.corner1;
	//return;

	//G_RAND_VAR = time; 
	//G_RAND_VAR = 38401;
	G_RAND_VAR = 1;
	//G_RAND_VAR = 1 /*(time % 100)*/ * rand(vec2(int(gl_FragCoord.x) % 10, int(gl_FragCoord.y) % 10));

	float aperture = 0.0f;

	vec2 screenSize = vec2(camera.pixelWidth, camera.pixelHeight);

	//g = seedRand(gl_FragCoord.xy * time);
	//g = seedRand(vec2(time, time));
	g = seedRand(gl_FragCoord.xy * RNGSeed);
	//g = uint(gl_FragCoord.x * gl_FragCoord.y);
	//float shade = rand(gl_FragCoord.xy);
	//outColor = vec4(getRand(), getRand(), getRand(), 1);
	//outColor = vec4(shade, shade, shade, 1);
	//return;
	

	vec3 origin = camera.position.xyz;

	
	
	vec3 colorAccumulator = vec3(0,0,0);

	for(int i = 0; i < SAMPLES_PER_PIXEL; i++)
	{
		// cameraPosition and nextCameraPosition represent the positions when the shutter opens and closes respectively. Randomly choose any point in that time period to simulate motion blur
		//float shutterTime = getRand();
		//shutterTime = 1.0f;
		//origin = mix(camera.position.xyz, camera2.position.xyz, shutterTime);
		//vec3 up = mix(camera.up.xyz, camera2.up.xyz, shutterTime);
		//vec3 right = mix(camera.right.xyz, camera2.right.xyz, shutterTime);
		//vec3 lowerLeftCorner = mix(camera.lowerLeftCorner.xyz, camera2.lowerLeftCorner.xyz, shutterTime);

		origin - camera.position.xyz;
		vec3 up = camera.up.xyz;
		vec3 right = camera.right.xyz;
		vec3 lowerLeftCorner = camera.lowerLeftCorner.xyz;


		// add random offset to sample for anti-aliasing
		vec2 offset = vec2(getRand() * 2 - 1, getRand() * 2 - 1);
		//vec2 offset = aperture * vec2(getRand() * 2 - 1, getRand() * 2 - 1); // this causes jittering?????????
		float ox = aperture * (getRand() * 2 - 1);
		float oy = aperture * (getRand() * 2 - 1);


		vec2 uv = (gl_FragCoord.xy + offset) / screenSize.xy;
		uv.y = 1 - uv.y; // Vulkan uses right-handed coord system (???)

		origin += vec3(ox, oy, 0); // add offset to origin for depth of field
		
		vec3 rayDir = lowerLeftCorner + (uv.x * right * camera.focalPlaneWidth) + (uv.y * up * camera.focalPlaneHeight) - origin;
		rayDir = normalize(rayDir);

		colorAccumulator += getColor(origin, rayDir);
	}

	outColor = vec4(colorAccumulator / SAMPLES_PER_PIXEL, 1.0f);
}
