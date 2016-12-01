// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Modifications by: Shannon TJ 10101385

// Date:    Fall 2016
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 Coordinates;

//first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

float pi = 3.14159265359;

//Initial origin position
uniform float x = 0;
uniform float y = 0;
uniform float z = 0;

//Sets of vertices
uniform float[3] light;
uniform float[12] planeVert;
uniform float[6] planeColor;

uniform float[288] triangleVert;
uniform float[96] triangleColor;

uniform float[12] sphereVert;
uniform float[9] sphereColor;

uniform float[8] planeLight;
uniform float[12] sphereLight;
uniform float[200]	triangleLight;

//Scene 1
uniform int pV = 6;
uniform int tV = 108;
uniform int sV = 4;

vec3 lightVec = vec3(light[0],light[1],light[2]);

vec3 normalPlane = vec3(0,0,0);
vec3 pointPlane = vec3(0,0,0);

vec3 centre = vec3(0,0,0);
float radius = 0;

vec3 point1 = vec3(0,0,0);
vec3 point2 = vec3(0,0,0);
vec3 point3 = vec3(0,0,0);

//Initialize focal length and origin
float focal_length = 1/(tan(pi/6)); 
vec3 Origin = vec3(x,y,z);


//Solve t for a plane intersection
float closePlane(vec3 D, vec3 N, vec3 Q)
{
	
	//Solve for t
	float numerator = dot(N,Q-Origin);
	float denominator = dot(D,N);
	
	//Check for divide by zero
	if(denominator == 0)
		return -1;
		
	else
	{
		float tPlane = numerator/denominator;
		return tPlane;
	}
}

//Solve t for a sphere intersection
float closeSphere(vec3 Dir, vec3 Centre, float radius)
{
	
	float a = dot(Dir,Dir);
	float b = 2*(dot(Origin,Dir) - dot(Centre,Dir));
	float c = (-2*dot(Origin,Centre)) + dot(Origin,Origin) + dot(Centre,Centre) - (radius*radius);
	
	float discrim = (b*b) - (4*a*c);
	
	//Negative discriminant, not a real number
	if(discrim < 0)
		return -1;
	
	//Otherwise...
	else
	{
		float tSphere1 = (-b + sqrt(discrim))/2*a;
		float tSphere2 = (-b - sqrt(discrim))/2*a;	
		
		//Return the smallest t value
		if(tSphere1<tSphere2)
			return tSphere1;
			
		else
			return tSphere2;
	}
}


//Solve t for a triangle intersection
float closeTriangle(vec3 D, vec3 P1, vec3 P2, vec3 P3)
{	
	vec3 e1 = P2 - P1;
	vec3 e2 = P3 - P1;
	vec3 s = Origin-P1;
	
	mat3 dee;
	mat3 see;
	mat3 dse;
	mat3 des; 
	
	dee[0] = -D;
	dee[1] = e1;
	dee[2] = e2;
	
	see[0] = s;
	see[1] = e1;
	see[2] = e2;
	
	dse[0] = -D;
	dse[1] = s;
	dse[2] = e2;
	
	des[0] = -D;
	des[1] = e1;
	des[2] = s;
	
	//Cramer's Rule
	vec3 tuv = (1/determinant(dee))*(vec3(determinant(see),determinant(dse),determinant(des)));
	
	float t = tuv.x;
	float u = tuv.y;
	float v = tuv.z;
	float plus = u+v;
	
	//if t > 0 and u, v, and u+v are within the [0,1] range
	if((t > 0) && (u > 0) && (u < 1) && (v > 0) && (v < 1) && (plus > 0) && (plus < 1))
		return t;
	
	else
		return -1;
}

bool shadowCheck(vec3 Direction, float smallest_t)
{
			//test current point against objects to get shadow intersection
		
				//Get ray intersection point
				vec3 pointHit = Origin + (smallest_t*Direction);
				//Get shadow ray direction + length
				vec3 shadowRay = lightVec - pointHit;
				//Normalize shadow ray direction
				float shadowLength = sqrt(dot(shadowRay,shadowRay));
				shadowRay = normalize(shadowRay);
				
				Origin = pointHit;
				
				float s;
				int j = 0;
				
				for(int i = 0; i < pV; i=i+6)	
				{
					normalPlane = vec3(planeVert[i], planeVert[i+1], planeVert[i+2]);
					pointPlane = vec3(planeVert[i+3], planeVert[i+4], planeVert[i+5]);
					
					s = closePlane(shadowRay, normalPlane, pointPlane);
					if(s > 0.001 && s < shadowLength)
						return true;
				}
				
				j = 0;
				for(int i = 0; i < sV; i=i+4)
				{
					
					centre = vec3(sphereVert[i], sphereVert[i+1], sphereVert[i+2]);
					radius = sphereVert[i+3];	
					
					s = closeSphere(shadowRay, centre, radius);
					if(s > 0.001 && s < shadowLength)
						return true;
				}	
				
				j = 0;
				
				for(int i = 0; i < tV; i=i+9)
				{
					point1 = vec3(triangleVert[i], triangleVert[i+1], triangleVert[i+2]);
					point2 = vec3(triangleVert[i+3], triangleVert[i+4], triangleVert[i+5]);
					point3 = vec3(triangleVert[i+6], triangleVert[i+7], triangleVert[i+8]);
					
					s = closeTriangle(shadowRay, point1, point2, point3);
					if(s > 0.001 && s < shadowLength)
						return true;
				}
					
				return false;
					
}

vec3 reflectCheck(vec3 Direction, vec3 Normal, float smallest_t)
{
	//test current point against objects to get reflection color
		
	//Get ray intersection point
	vec3 pointHit = Origin + (smallest_t*Direction);
	//Get reflect ray direction + length
	vec3 reflectRay = Direction - 2*(dot(Direction,Normal))*Normal;
	//Normalize reflect ray direction
	float reflectLength = sqrt(dot(reflectRay,reflectRay));
	reflectRay = normalize(reflectRay);
				
	Origin = pointHit;	
	
	vec3 closestColor = vec3(0,0,0);
	
	//reflectivity value between 0 and 1, how much an object's color contributes to the ray
	//ref = 1 means perfectly reflective
	//ref = 0 means no reflective
	//where a and b are objects...
	//(1-A.reflect)A.color + A.reflect*B.color
	//(1-A.reflect)A.color + A.reflect*(1-B.reflect)B.color + B.reflect(C.color)
	
				float s;
				int bounce = 10;
				int j = 0;
				
				
				//while bounce < 10...
				
				for(int i = 0; i < pV; i=i+6)	
				{
					normalPlane = vec3(planeVert[i], planeVert[i+1], planeVert[i+2]);
					pointPlane = vec3(planeVert[i+3], planeVert[i+4], planeVert[i+5]);
					
					s = closePlane(reflectRay, normalPlane, pointPlane);
					if(s > 0.001 && s < reflectLength)
						{
							closestColor = vec3(planeColor[j], planeColor[j+1], planeColor[j+2]);
							return closestColor;
						}
					j = j+3;
				}
				
				for(int i = 0; i < sV; i=i+4)
				{
					
					centre = vec3(sphereVert[i], sphereVert[i+1], sphereVert[i+2]);
					radius = sphereVert[i+3];	
					
					s = closeSphere(reflectRay, centre, radius);
					if(s > 0.001 && s < reflectLength)
						{
							closestColor = vec3(sphereColor[j], sphereColor[j+1], sphereColor[j+2]);
							return closestColor;
						}
					j = j+3;
				}	
				
				for(int i = 0; i < tV; i=i+9)
				{
					point1 = vec3(triangleVert[i], triangleVert[i+1], triangleVert[i+2]);
					point2 = vec3(triangleVert[i+3], triangleVert[i+4], triangleVert[i+5]);
					point3 = vec3(triangleVert[i+6], triangleVert[i+7], triangleVert[i+8]);
					
					s = closeTriangle(reflectRay, point1, point2, point3);
					if(s > 0.001 && s < reflectLength)
						{
							closestColor = vec3(triangleColor[j], triangleColor[j+1], triangleColor[j+2]);
							return closestColor;
						}
					j = j+3;
				}
	
	return closestColor;
	
}

vec3 closestShape(vec3 Direction)
{
	//Default color is black
	vec3 closestColor = vec3(0,0,0);
	float t = 0;
	float smallest_t = 1000000;

	bool shadow = false;
	bool plane = false;
	
	vec3 Normal;
	
	int b = 0;
	int d = 0;
	int f = 0;
	
	int x = 0;
	int y = 0;
	int z = 0;
	
		//test if plane is closest
		for(int a = 0; a < pV; a = a+6)
		{
			normalPlane = vec3(planeVert[a], planeVert[a+1], planeVert[a+2]);
			pointPlane = vec3(planeVert[a+3], planeVert[a+4], planeVert[a+5]);
			t = closePlane(Direction, normalPlane, pointPlane);
			
			//If t intersects and t is smaller than current smallest t, change the smallest t value
			if(t > 0 && t < smallest_t)
			{
				//Get original color
				smallest_t = t;
				closestColor = vec3(planeColor[b], planeColor[b+1], planeColor[b+2]);	
				
				float cA = planeLight[x];
				float cL = planeLight[x+1];
				float cP = planeLight[x+2];
				float p = planeLight[x+3];
				
				//Lighting equation
				vec3 pointHit = Origin + (smallest_t*Direction);
				vec3 l = lightVec - pointHit;
				l = normalize(l);
				
				vec3 h = normalize(-pointHit)+l;
				h = normalize(h);
				
				Normal = normalPlane;
				
				closestColor = closestColor*(cA + cL*max(0,dot(normalPlane,l))) + (cP*closestColor*max(0,pow(dot(h,normalPlane),p)));		
				
				//Set plane to true so shadow is not cast in scene 3
				if(pointPlane == vec3(0,0,-20.0f))
					plane = true;
					
			}

			b = b+3;
			x = x+4;
		}

		//test if sphere is closest
		for(int c = 0; c < sV; c = c+4)
		{
			centre = vec3(sphereVert[c], sphereVert[c+1], sphereVert[c+2]);
			radius = sphereVert[c+3];
			t = closeSphere(Direction, centre, radius);

			if(t > 0 && t < smallest_t)
			{
				//Get original color
				smallest_t = t;
				closestColor = vec3(sphereColor[d], sphereColor[d+1], sphereColor[d+2]);
				
			    float cA = sphereLight[y];
				float cL = sphereLight[y+1];
				float cP = sphereLight[y+2];
				float p = sphereLight[y+3];
		
				//Lighting equation
				vec3 pointHit = Origin + (smallest_t*Direction);
				vec3 l = lightVec - pointHit;
				l = normalize(l);
										
				vec3 normalSphere = pointHit - centre;
				normalSphere = normalize(normalSphere);
				
				vec3 h = normalize(-pointHit)+l;
				h = normalize(h);
	
			    Normal = normalSphere;
					
				closestColor = closestColor*(cA + cL*max(0,dot(normalSphere,l))) + (cP*closestColor*max(0,pow(dot(h,normalSphere),p)));
			}
			
			d = d+3;
			y = y+4;
		}
		
		//test if triangle is closest
		for(int e = 0; e < tV; e = e+9)
		{
			point1 = vec3(triangleVert[e], triangleVert[e+1], triangleVert[e+2]);
			point2 = vec3(triangleVert[e+3], triangleVert[e+4], triangleVert[e+5]);
			point3 = vec3(triangleVert[e+6], triangleVert[e+7], triangleVert[e+8]);
			t = closeTriangle(Direction, point1, point2, point3);

			if(t > 0 && t < smallest_t)
			{
				//Get original color
				smallest_t = t; 
				closestColor = vec3(triangleColor[f], triangleColor[f+1], triangleColor[f+2]);
				
				//Get ambient color, light intensity, phong exponent
			    float cA = triangleLight[z];
				float cL = triangleLight[z+1];
				float cP = triangleLight[z+2];
				float p = triangleLight[z+3];
				
				//Lighting equation
				vec3 pointHit = Origin + (smallest_t*Direction);
				vec3 l = lightVec - pointHit;
				l = normalize(l);				
				
				vec3 side1 = point2 - point1;
				vec3 side2 = point3 - point1;
				
				vec3 h = normalize(-pointHit)+l;
				h = normalize(h);
				
				vec3 normalTriangle = cross(side1,side2);
				normalTriangle = normalize(normalTriangle);
				
				Normal = normalTriangle;
					
				closestColor = closestColor*(cA + cL*max(0,dot(normalTriangle,l))) + (cP*closestColor*max(0,pow(sqrt(dot(h,normalTriangle)),p)));
			}

			f = f+3;
			z = z+4;
		}					
		
				//vec3 reflect = reflectCheck(Direction, Normal, smallest_t);
				//reflect = reflect*0.01f;
				//closestColor = closestColor + reflect;
				
				shadow = shadowCheck(Direction, smallest_t);	
				if(shadow && plane == false)
					closestColor = closestColor-0.3f;	
				
				Origin = vec3(x,y,z);
	
	return closestColor;
}


void main(void)
{
	//Direction vector
	vec3 Direction = vec3(Coordinates.x,Coordinates.y,-focal_length);
	
	//Normalize direction vector
	Direction = normalize(Direction);
			
	//Look at every shape, find smallest value of t
	//Get color belonging to the closest shape
	vec3 newColor = closestShape(Direction);
		
    //Write colour output 
    FragmentColour = vec4(newColor,0);
}
