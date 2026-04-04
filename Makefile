.PHONY: all run clean debug test

all:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run: all
	./build/conr

dmg:
	cmake --build build --config Release
	bash package/macos/build_dmg.sh

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	./build/conr

test:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target test_runner
	./build/test_runner

clean:
	rm -rf build