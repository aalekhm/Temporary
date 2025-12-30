CC := g++

OBJS := 3dsparse.o \
		xmlmesh.o \
		vect.o \
		material.o

3ds2mesh: $(OBJS)
	$(CC) -o 3ds2mesh $(OBJS)

3dparse.o: 3dparse.cpp types.h
	$(CC) -c 3dparse.cpp

xmlmesh.o: xmlmesh.cpp types.h xmlmesh.h
	$(CC) -c xmlmesh.cpp

vect.o: vect.cpp vect.h
	$(CC) -c vect.cpp

material.o: material.cpp types.h material.h
	$(CC) -c material.cpp

