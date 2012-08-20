SOURCES = padmain.cpp CameraDelegate.h types.h OCRer.h OCRer.cpp CameraManager.h CameraManager.cpp
OBJECTS = padmain.o OCRer.o CameraManager.o
DEP_FILES = .deps/*.p
CXX_FLAGS = -g -Wall
LIB_FLAGS = -ltesseract -lv4lconvert
EXECUTABLE = pad

CXX = c++

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LIB_FLAGS)

-include $(DEP_FILES)

%.o: %.cpp
	$(CXX) -Wp,-MMD,.deps/$(*F).p -o $@ $< -c $(CXX_FLAGS)

clean:
	-rm $(EXECUTABLE) $(OBJECTS) $(DEP_FILES)
