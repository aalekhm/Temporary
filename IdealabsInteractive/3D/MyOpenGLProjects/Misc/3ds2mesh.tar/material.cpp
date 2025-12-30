/* material.cpp */

#include <fstream.h>
#include <iostream.h>

#include "material.h"
#include "types.h"

ofstream matfile;

void setup_material_file (char *filename) {

	matfile.open (filename);

}

void close_material_file (void) {

	matfile.close ();
}

void write_material (Material *mat) {
	//write the material
	matfile << "material " << mat->getName() << "\n";
	matfile << "{\n";
	matfile << "\ttechnique\n";
	matfile << "\t{\n";
	matfile << "\t\tpass\n";
	matfile << "\t\t{\n";
	
	matfile << "\t\t\tambient " << mat->getAmbient()->red << " " << mat->getAmbient()->green << " " << mat->getAmbient()->blue << "\n";
	//matfile << "\t\t\tdiffuse " << mat->getDiffuse()->red << " " << mat->getDiffuse()->green << " " << mat->getDiffuse()->blue << "\n";

	if (strcmp(mat->getTextureName(), "NONE")) {
		matfile << "\n\t\t\ttexture_unit\n\t\t\t{\n";
		matfile << "\t\t\t\ttexture " << mat->getTextureName() << "\n";
		matfile << "\t\t\t}\n";
	}

	//end
	matfile << "\t\t}\n\t}\n}\n\n";
}
