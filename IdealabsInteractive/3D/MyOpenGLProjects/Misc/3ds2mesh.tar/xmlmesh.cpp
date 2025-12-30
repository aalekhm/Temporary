/* xmlmesh.cpp */

#include <fstream.h>

#include "xmlmesh.h"
#include "types.h"

ofstream xmlfile;

void setup_mesh_file (char *filename) {

	xmlfile.open (filename);

	xmlfile << "<mesh>\n";
	xmlfile << "\t<submeshes>\n";
}

void close_mesh_file (void) {
	xmlfile << "\t</submeshes>\n";
	xmlfile << "</mesh>\n";

	xmlfile.close ();
}

void write_mesh (Mesh *mesh) {
	if (mesh->getNumFaces() == 0)
		return;
		
	//start
	xmlfile << "\t\t<submesh material=\"" << mesh->getMaterialName() << "\" ";
	xmlfile << "usesharedvertices=\"false\" ";
	xmlfile << "use32bitindexes=\"false\" ";
	xmlfile << "operationtype=\"triangle_list\">\n";

	//write face data
	short i;
	face *f;
	xmlfile << "\t\t\t<faces count=\"" << mesh->getNumFaces() << "\">\n";
	for (i = 0; i < mesh->getNumFaces(); ++i) {
		f = mesh->getFace(i);

		xmlfile << "\t\t\t\t";
		xmlfile << "<face v1=\"" << f->v1 << "\" ";
		xmlfile << "v2=\"" << f->v2 << "\" ";
		xmlfile << "v3=\"" << f->v3 << "\" ";
		xmlfile << "/>\n";
	}
	xmlfile << "\t\t\t</faces>\n";
	
	// write vertex data
	vertex *v;
	xmlfile << "\t\t\t<geometry vertexcount=\"" << mesh->getNumVertexes() << "\">\n";
	// position data
	xmlfile << "\t\t\t\t<vertexbuffer positions=\"true\">\n";
	for (i = 0; i < mesh->getNumVertexes(); ++i) {
		v = mesh->getVertex(i);

		xmlfile << "\t\t\t\t\t<vertex>\n";

		xmlfile << "\t\t\t\t\t\t";
		xmlfile << "<position x=\"" << v->x << "\" ";
		xmlfile << "y=\"" << v->y << "\" ";
		xmlfile << "z=\"" << v->z << "\" ";
		xmlfile << "/>\n";

		xmlfile << "\t\t\t\t\t</vertex>\n";
	}
	xmlfile << "\t\t\t\t</vertexbuffer>\n";
	//normal data
	xmlfile << "\t\t\t\t<vertexbuffer normals=\"true\">\n";
	for (i = 0; i < mesh->getNumVertexes(); ++i) {
		Vector v;

        vect_copy (v, *mesh->getNormal(i));

		xmlfile << "\t\t\t\t\t<vertex>\n";

		xmlfile << "\t\t\t\t\t\t";
		xmlfile << "<normal x=\"" << v[0] << "\" ";
		xmlfile << "y=\"" << v[1] << "\" ";
		xmlfile << "z=\"" << v[2] << "\" ";
		xmlfile << "/>\n";

		xmlfile << "\t\t\t\t\t</vertex>\n";
	}
	xmlfile << "\t\t\t\t</vertexbuffer>\n";
	// texture coords (if exist)
	if (mesh->getTexVertexes () != NULL) {
		xmlfile << "\t\t\t\t<vertexbuffer texture_coord_dimensions_0=\"2\" texture_coords=\"1\">\n";
		for (i = 0; i < mesh->getNumVertexes(); ++i) {

			xmlfile << "\t\t\t\t\t<vertex>\n";

			xmlfile << "\t\t\t\t\t\t";
			xmlfile << "<texcoord u=\"" << mesh->getTexVertexes ()[i].x << "\" ";
			xmlfile << "v=\"" << mesh->getTexVertexes ()[i].y << "\" ";
			xmlfile << "/>\n";

			xmlfile << "\t\t\t\t\t</vertex>\n";
		}

		xmlfile << "\t\t\t\t</vertexbuffer>\n";
	}
	xmlfile << "\t\t\t</geometry>\n";

	//end
	xmlfile << "\t\t</submesh>\n";
}
