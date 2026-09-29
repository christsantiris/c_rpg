.PHONY: all run clean debug test linux sprites

all:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run: all
	./build/conr

dmg:
	cmake --build build --config Release
	bash package/macos/build_dmg.sh

linux:
	bash package/linux/build_linux.sh

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	./build/conr $(if $(WEAPON),--weapon "$(WEAPON)") $(if $(GOLD),--gold "$(GOLD)") $(if $(SCROLLS),--scrolls "$(SCROLLS)")

test:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target test_runner
	./build/test_runner

sprites:
	python3 tools/sprites/castle.py
	python3 tools/sprites/tavern.py
	python3 tools/sprites/inn.py
	python3 tools/sprites/blacksmith.py
	python3 tools/sprites/alchemist.py
	python3 tools/sprites/harbor.py
	python3 tools/sprites/healer.py
	python3 tools/sprites/witch.py
	python3 tools/sprites/labyrinth.py
	python3 tools/sprites/apothecary.py
	python3 tools/sprites/crownroad_gate.py

clean:
	rm -rf build
