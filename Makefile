.PHONY: all run clean debug

all:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run: all
	./build/conr

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	./build/conr

clean:
	rm -rf build