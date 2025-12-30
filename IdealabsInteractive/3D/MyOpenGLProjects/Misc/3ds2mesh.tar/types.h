/* types.h */

#ifndef TREDS_TYPES
#define TREDS_TYPES

#include "vect.h"

#include <string.h>

typedef struct {
	float x, y, z;
} vertex;

typedef struct {
	float x, y;
} texvertex;

typedef struct {
	unsigned short v1, v2, v3, flags;
} face;

class Colour {
	public:
		float red, green, blue;

	Colour (float r, float g, float b) {
		setRGB (r,g,b);
	}

	Colour () {
	}
	
	void setRGB (float r, float g, float b) {
		red = r;
		green = g;
		blue = b;
	}
};

class Mesh {
	private:
		short numvert;
		vertex *vertexes;
		Vector *vnormals;
		texvertex *texvertexes;

		short numfaces;
		face *faces;
		
		char matname[50];

	public:
		Mesh (void) {
			numvert = 0;
			vertexes = NULL;
			texvertexes = NULL;

			numfaces = 0;
			faces = NULL;
		}

		~Mesh (void) {
			if (vertexes)
				delete vertexes;
			if (faces)
				delete faces;
		}

		void setVertexes (vertex *varray, short nvert) {
			vertexes = varray;
			numvert = nvert;
		}
		
		void setTexVertexes (texvertex *tarray) { texvertexes = tarray; }

		void setFaces (face *farray, short nfac) {
			faces = farray;
			numfaces = nfac;
		}
		
		void setVertexNormals (Vector *norm) { vnormals = norm; }

		short getNumFaces (void) { return numfaces; }

		short getNumVertexes (void) { return numvert; }

		face* getFace (short i) { return &faces[i]; }

		vertex* getVertex (short i) { return &vertexes[i]; }

		texvertex* getTexVertexes (void) { return texvertexes; }

		Vector* getNormal(short i) { return &vnormals[i]; }

		void setMaterialName (char *name) { strcpy(matname, name); }

		char *getMaterialName (void) { return matname; }

};

class Material {
	private:
		char name[32];

		Colour ambient;
		Colour diffuse;
		
		//tex data
		float texstrenght;
		char texname[32];
	public:
		Material (void) {
			ambient.setRGB (1.0, 1.0, 1.0);
			diffuse.setRGB (1.0, 1.0, 1.0);

			setTextureName ("NONE");
		}
		
		void setName (char *na) { strcpy(name, na); }

		char *getName (void) { return name; }

		void setDiffuse (Colour dif) { diffuse.setRGB (dif.red, dif.green, dif.blue); }
		
		Colour* getDiffuse (void) { return &diffuse; }

		Colour* getAmbient (void) { return &ambient; }
		
		void setAmbient (Colour amb) { diffuse.setRGB (amb.red, amb.green, amb.blue); }

		void setTextureStrenght ( float ts ) { texstrenght = ts; }

		void setTextureName ( char *tn ) { strcpy(texname, tn); }
		
		char *getTextureName (void) { return texname; }
};

#endif
