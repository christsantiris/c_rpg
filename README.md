# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

## Table of Contents
- [Overview](#overview)
- [Screenshot](#screenshot)
- [Building and Running](#building-and-running)
  - [Compile the Game](#compile-the-game)
  - [Run the Game](#run-the-game)
  - [Clean the Build](#clean-the-build)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Installers](#installers)
- [Roadmap](#roadmap)

## Overview
*The Castle of No Return* is a text-based roguelike game built with C. The game features turn-based combat and a retro art style.

## Screenshot
Below are screenshots of the game in action:

<img width="2560" height="1428" alt="image" src="https://github.com/user-attachments/assets/82afe40e-d893-486c-a3eb-42e1037b8408" />
<img width="2562" height="1426" alt="image" src="https://github.com/user-attachments/assets/54defc0a-0c11-4941-b706-fc27f0c62ccb" />


## Compile the game
To compile the app run `make run` in the root directory.

## Clean the build
Run `make clean` to destroy the compiled game and start over if you make your own changes

## Clean the tests
Run `make test` to run unit tests

## Dependencies
cmake sdl2 sdl2_ttf sdl2_mixer

## Contributing
I would gladly welcome any pull requests to help improve the game

## License
MIT

## Installers
For MacOS run `chmod +x package/macos/build_dmg.sh` then `make dmg`
and drag the file into applications and double click!

## Roadmap
- windows installer
- linux installer
- expand sound effects (stairs unlock, attack, magic)
- Add other areas (mountains, forest, marsh)
- Quests
- NPCs
- Secret/Locked Rooms
- Minimap
