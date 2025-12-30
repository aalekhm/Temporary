/* 3DS2OgreXML */
#include <string.h>

#include <fstream.h>
#include <iostream.h>

#include "types.h"
#include "vect.h"
#include "xmlmesh.h"
#include "material.h"

#ifdef __APPLE__
#define _SWAP_BYTES_ 1
#endif

// -------------------------------------------------------------

char *filedata;

#ifdef _SWAP_BYTES_

void swapword (char *word) {
	char a, b;
	
	a = *word;
	b = *(word+1);
	
	*(word+1) = a;
	*word = b;
}

void swapdword (char *dword) {
	char a, b, c, d;
	
	a = *dword;
	b = *(dword+1);
	c = *(dword+2);
	d = *(dword+3);
	
	*dword = d;
	*(dword+1) = c;
	*(dword+2) = b;
	 *(dword+3) = a;
}

#endif

// -------------------------------------------------------------

class Chunk {
	private:
		unsigned short tag;
		long size;
		char *data;

		Chunk *curSubChunk;
		long curpos;
		char *curdata;
	public:
		Chunk (char *indata) {
			data = indata;
			memcpy((char*)&tag, data, 2);
			memcpy((char*)&size, data + 2, 4);
			
			#ifdef _SWAP_BYTES_
				swapword ((char*)&tag);
				swapdword ((char*)&size);
			#endif
			
			data += 6;

			curSubChunk = NULL;

			curpos = 0;
			curdata = data;
		}

		~Chunk (void) {}

		unsigned short getTag (void) { return (tag); }

		long getSize (void) { return (size); }
		
		char* getData (void) { return (data); }
		
		Chunk* getSubChunk (void) { return (curSubChunk); }
		
		Chunk* nextSubChunk (void) {
			if (curpos < (size-6)) {
				if (curSubChunk) {
					curdata += curSubChunk->size;
					curpos += curSubChunk->size;
					delete curSubChunk;
				}
				
				curSubChunk = new Chunk (curdata);
			}
			else
				curSubChunk = NULL;
				
			return curSubChunk;
		}
		
		void getShort (unsigned short *ret) {
			memcpy((char*)ret, curdata, 2);
			
			#ifdef _SWAP_BYTES_
				swapword ((char*)ret);
			#endif
			
			curdata += 2;
			curpos += 2;
		}
		
		void getChar (unsigned char *ret) {
			*ret = *curdata;
			curdata += 1;
			curpos += 1;
		}

		void getFloat (float *ret) {
			memcpy((char*)ret, curdata, 4);
			
			#ifdef _SWAP_BYTES_
				swapdword ((char*)ret);
			#endif
			
			curdata += 4;
			curpos += 4;
		}

		void getString (char *ret) {
			strcpy (ret, curdata);
			curdata += strlen (ret) + 1;
			curpos += strlen (ret) + 1;
		}

};

// -------------------------------------------------------------

void parse3ds (void);
void parsemeshdata (Chunk *meshdatachunk);
void parsenamedobject (Chunk *objectchunk);
void parsetriangleobject (Chunk *triobjectchunk);
void CalculateVertexNormals (Mesh *m);
void parsefacearray (Chunk *arraychunk, Mesh *mesh);
void parsecolor (Chunk *colchunk, Colour *col);
float parsepercentage (Chunk *perchunk);
void parsematerial (Chunk *matchunk);

// --------------------------------------------------------------

void CalculateVertexNormals (Mesh *m) {
	Vector *fnormals, *vnormals;
	Vector vtot;
	face *f;
	Vector v1, v2, v3, ab, ac;
	short i, j;
	float count;

	fnormals = new Vector[m->getNumFaces()];

	for (i= 0; i < m->getNumFaces(); ++i) {
		f = m->getFace (i);

		vect_init (v1, m->getVertex(f->v1)->x, m->getVertex(f->v1)->y, m->getVertex(f->v1)->z);
		vect_init (v2, m->getVertex(f->v2)->x, m->getVertex(f->v2)->y, m->getVertex(f->v2)->z);
		vect_init (v3, m->getVertex(f->v3)->x, m->getVertex(f->v3)->y, m->getVertex(f->v3)->z);

		vect_sub (ab, v2, v1);
    	vect_sub (ac, v3, v1);
    	vect_cross (fnormals[i], ac, ab);

    	vect_normalize (fnormals[i]);

		//cout << fnormals[i][1] << " " << fnormals[i][2] << " " << fnormals[i][3] << "\n";
	}
	vnormals = new Vector[m->getNumVertexes()];
	for (i = 0; i < m->getNumVertexes(); ++i) {
		// find faces attached to our normal
		count = 0;
		vect_init (vtot, 0.0, 0.0, 0.0);
		for (j = 0; j < m->getNumFaces(); ++j) {
			f = m->getFace (j);
			if ((i == f->v1)||(i == f->v2)||(i == f->v3)) {
				++count;
				vect_init (ab, fnormals[j][0], fnormals[j][1], fnormals[j][2]);
				vect_add (vtot, vtot, ab);
			}
		}

		vect_init (vnormals[i], vtot[0]/count, vtot[1]/count, vtot[2]/count);
		vect_normalize (vnormals[i]);

		//cout << count << " " << vnormals[i][0] << " " << vnormals[i][1] << " " << vnormals[i][2] << "\n";

	}
	
	m->setVertexNormals (vnormals);
}

void parsefacearray (Chunk *arraychunk, Mesh *mesh) {
	face *faces;
	unsigned short nfac;
	short i;

	arraychunk->getShort (&nfac);

	faces = new face[nfac];

	for (i = 0; i < nfac; ++i) {
		arraychunk->getShort (&faces[i].v1);
		arraychunk->getShort (&faces[i].v2);
		arraychunk->getShort (&faces[i].v3);
		arraychunk->getShort (&faces[i].flags);
	}

	//parse subchunks
	mesh->setMaterialName("NONE");
	Chunk *subchunk = arraychunk->nextSubChunk ();
	while (subchunk != NULL) {
		switch (subchunk->getTag()) {
			case 0x4130:
				char matname[50];
				subchunk->getString(matname);
				mesh->setMaterialName(matname);
				break;
		}
		subchunk = arraychunk->nextSubChunk ();
	}

	mesh->setFaces (faces, nfac);
}

void parsetriangleobject (Chunk *triobjectchunk) {
	Mesh mesh;
	short i;
	vertex *vert;
	face *faces;
	texvertex *texvert;
	unsigned short nvert, ntvert;

	cout << "Parsing...\n";

	Chunk *subchunk = triobjectchunk->nextSubChunk ();

	while (subchunk != NULL) {
		//cout << hex << subchunk->getTag() << "\n";
		switch (subchunk->getTag()) {
			case 0x4110: // parse point array
				subchunk->getShort (&nvert);

				vert = new vertex[nvert];

				for (i = 0; i < nvert; ++i) {
					subchunk->getFloat (&vert[i].x);
					subchunk->getFloat (&vert[i].y);
					subchunk->getFloat (&vert[i].z);

					//cout << vert[i].x << " " << vert[i].y << " " << vert[i].z << "\n";
				}

				mesh.setVertexes (vert, nvert);
				break;
			case 0x4160:
				break;
			case 0x4120:
				unsigned short nfac;
				parsefacearray (subchunk, &mesh);
				break;
			case 0x4140: // uv array
				subchunk->getShort (&ntvert);

				if (nvert != ntvert)
					cout << "Possible errors in UV mapping\n";

				texvert = new texvertex[ntvert];

				for (i = 0; i < ntvert; ++i) {
					subchunk->getFloat (&texvert[i].x);
					subchunk->getFloat (&texvert[i].y);
				}
				
				mesh.setTexVertexes (texvert);
				break;
		}

		subchunk = triobjectchunk->nextSubChunk ();
	}

	CalculateVertexNormals (&mesh);

	cout << "Writing...\n";
	write_mesh (&mesh);
	cout << "Done.\n";
}

void parsecolor (Chunk *colchunk, Colour *col) {
	float r,g,b;
	unsigned char cr, cg, cb;

	//cout << hex << colchunk->getTag() << "\n";
	switch (colchunk->getTag()) {
		case 0x0010:
			colchunk->getFloat (&r);
			colchunk->getFloat (&g);
			colchunk->getFloat (&b);
			break;
		case 0x0011:
			colchunk->getChar (&cr);
			colchunk->getChar (&cg);
			colchunk->getChar (&cb);
			r = (float)cr / 255.0;
			g = (float)cg / 255.0;
			b = (float)cb / 255.0;
			break;
	}

	col->setRGB(r,g,b);
}

float parsepercentage (Chunk *perchunk) {
	float perc;
	unsigned short sperc;

	//cout << hex << colchunk->getTag() << "\n";
	switch (perchunk->getTag()) {
		case 0x0030:
			perchunk->getShort (&sperc);
			perc = (float)sperc/100.0;
			break;
		case 0x0031:
			perchunk->getFloat (&perc);
			break;
	}

	return perc;
}

void parsematerial (Chunk *matchunk) {
	char name[64], texname[64];
	Material mat;
	Colour col;

	//objectchunk->getString (name);

	Chunk *subchunk = matchunk->nextSubChunk ();

	while (subchunk != NULL) {
		//cout << hex << subchunk->getTag() << "\n";
		switch (subchunk->getTag()) {
			case 0xa000: // name
				subchunk->getString (name);
				cout << "MATERIAL \"" << name << "\":\n";
				cout << "Parsing...\n";
				mat.setName (name);
				break;
			/*case 0xa010: // ambient
				parsecolor (subchunk->nextSubChunk (), &col);
				mat.setAmbient (col);
				break;*/
			case 0xa020: // ambient?
				parsecolor (subchunk->nextSubChunk (), &col);
				mat.setAmbient (col);
				break;
			case 0xa200: // texture map
				mat.setTextureStrenght (parsepercentage (subchunk->nextSubChunk()));
				subchunk->nextSubChunk()->getString(texname);
				mat.setTextureName (texname);
				//cout << texname << "\n";
				break;
		}

		subchunk = matchunk->nextSubChunk ();
	}
	
	cout << "Writing...\n";
	write_material (&mat);
	cout << "Done.\n";
}


void parsenamedobject (Chunk *objectchunk) {
	char name[64];

	objectchunk->getString (name);
	cout << "MESH \"" << name << "\"\n";

	Chunk *subchunk = objectchunk->nextSubChunk ();

	while (subchunk != NULL) {
		//cout << hex << subchunk->getTag() << "\n";
		switch (subchunk->getTag()) {
			case 0x4100: // named object
				parsetriangleobject (subchunk);
				break;
		}

		subchunk = objectchunk->nextSubChunk ();
	}
}

void parsemeshdata (Chunk *meshdatachunk) {
	Chunk *subchunk = meshdatachunk->nextSubChunk ();

	while (subchunk != NULL) {
		switch (subchunk->getTag()) {
			case 0xafff: // material entry
				parsematerial (subchunk);
				break;
			case 0x4000: // named object
				parsenamedobject (subchunk);
				break;
		}


		subchunk = meshdatachunk->nextSubChunk ();
	}
}

void parse3ds (void) {
	Chunk mainchunk (filedata);
	Chunk *subchunk = mainchunk.nextSubChunk ();

	while (subchunk != NULL) {

		switch (subchunk->getTag()) {
			case 0x0002:  // 3ds version
				unsigned short version;
				subchunk->getShort (&version);
				//cout << "3DS Version: " << hex << version << dec << '\n';
				break;
			case 0x3D3D:  // mesh data chunk
				parsemeshdata (subchunk);
				break;
		}

		subchunk = mainchunk.nextSubChunk ();
	}
}

int main (int argc, char *argv[])
{
    cout << "\n3D Studio to XML Mesh Converter by Marco Cirant\n\n";
	
	if (argc < 2) {
		cout << "  Usage: 3DS2MESH inputfile.3ds\n\n";
		return 1;
	}

	// create output file names
	char meshfname[32], matfname[32], *ptr;
	strcpy (meshfname, argv[1]);
	ptr = strchr (meshfname, '.');
	*ptr = '\0';
	strcpy (matfname, meshfname);
	strcat (matfname, ".material");
	strcat (meshfname, ".mesh.xml");
	
	// open input and output file
	ifstream infile (argv[1], ios::binary);
	if (infile.bad()) {
		cout << argv[1] << " does not exists\n";
		return 1;
	}

	setup_mesh_file (meshfname);
	setup_material_file (matfname);

	// read file
	short maintag;
	long filesize;
	infile.read ((char*)&maintag, 2);
	if (maintag != 0x4D4D) {
		cout << argv[1] << " is not a 3D Studio file\n";
		return (1);
	}
	infile.read ((char*)&filesize, 4);
	
	#ifdef _SWAP_BYTES_
		swapdword ((char*)&filesize);
	#endif
	
	filesize += 6;
	infile.seekg (0);
	filedata = new char[filesize];
	if (filedata == NULL) {
		cout << "Not enough memory\n";
		return (1);
	}
	infile.read(filedata, filesize);
	
	// DO IT
	parse3ds ();

	delete filedata;

	//close files
	infile.close ();
	close_mesh_file ();

	return 0;
}

