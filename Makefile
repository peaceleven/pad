SOURCES = CameraDelegate.h types.h OCRer.h OCRer.cpp CameraManager.h CameraManager.cpp
OBJECTS = OCRer.o CameraManager.o
DEP_FILES = .deps/*.p
CXX_FLAGS = -g -Wall
LIB_FLAGS = -ltesseract -lv4lconvert -lm -lopencv_core -lopencv_highgui -lopencv_imgproc
EXECUTABLE = pad
TEST = padtest

CXX = c++

all: $(EXECUTABLE) $(TEST)

$(TEST): $(OBJECTS) padtest.o
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LIB_FLAGS)

$(EXECUTABLE): $(OBJECTS) padmain.o
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LIB_FLAGS)

-include $(DEP_FILES)

%.o: %.cpp
	$(CXX) -Wp,-MMD,.deps/$(*F).p -o $@ $< -c $(CXX_FLAGS)

clean:
	-rm $(EXECUTABLE) $(OBJECTS) $(DEP_FILES)
