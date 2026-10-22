.PHONY: all run clean debug test linux browser browser-run sprites

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

browser:
	emcmake cmake -S . -B build/browser -DCMAKE_BUILD_TYPE=Release
	emmake cmake --build build/browser --target conr

browser-run: browser
	python3 -m http.server 8000 --bind 127.0.0.1 --directory dist/browser

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
	python3 tools/sprites/dungeon.py
	python3 tools/sprites/forest.py
	python3 tools/sprites/mountain.py
	python3 tools/sprites/swamp.py
	python3 tools/sprites/coast.py
	python3 tools/sprites/desert.py
	python3 tools/sprites/dragonspine.py
	python3 tools/sprites/frostfell.py
	python3 tools/sprites/moonveil.py
	python3 tools/sprites/ashen.py
	python3 tools/sprites/glassdeep.py
	python3 tools/sprites/catacombs.py
	python3 tools/sprites/apothecary.py
	python3 tools/sprites/adventurers_guild.py
	python3 tools/sprites/workshop.py
	python3 tools/sprites/town_hall.py
	python3 tools/sprites/local_buildings.py
	python3 tools/sprites/towns.py
	python3 tools/sprites/polar_kraken.py
	python3 tools/sprites/crownroad_gate.py

clean:
	rm -rf build
