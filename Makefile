all:
	g++ -std=c++11 *.cpp -DGL_GLEXT_PROTOTYPES -lSDL2 -lGL
