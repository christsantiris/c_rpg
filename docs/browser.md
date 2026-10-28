# Browser build and GitHub Pages

Castle of No Return uses Emscripten to compile its C/SDL2 game to WebAssembly.
The browser build includes its font, BMP sprites, MP3 music and effects, and Ogg
effects. The native build and existing JSON save format remain available.

The browser draws through SDL's WebGL renderer, like the desktop OpenGL build, so
areas drawn at the 1.5× encounter zoom keep their tiles seamless. If WebGL is
unavailable, the game falls back to SDL's software renderer; it still plays, but
zoomed areas can show thin seam lines between small tile details.

## Build and preview locally
From the Castle of No Return project root, run these two commands in the same terminal:

```sh
source /private/tmp/wolf-browser-emsdk/emsdk_env.sh
make browser-run
```

Open **http://localhost:8000/**. Wait for loading to finish, then click **Play**
or press Enter. Starting with a click or keypress enables browser audio. The
game canvas takes keyboard focus; clicking it restores focus. The page has a
separate **Fullscreen** button, keeping **F** available for ranged attacks.
Press Escape to leave fullscreen; Escape also follows the game's menu rules.
The controls beneath the canvas show the default bindings; use the game's
Controls menu to remap them.

Run the `source` command again whenever you open a new terminal. CMake 3.20+,
Make and Python 3 are also required. The SDK is in a temporary directory; if it
is removed, install the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
version **4.0.23** in a permanent location and update the `source` path.
`make browser` builds without launching the server. The
build uses both `emcmake` and `emmake`, so CI and local builds use Emscripten's
compiler and archiver. Browser intermediates live in `build/browser`, separately
from native builds. No locally installed SDL libraries are needed for this build.

The deployable files are generated in `dist/browser/`:

| File | Purpose |
| --- | --- |
| `index.html` | Launch page, controls, storage setup and game canvas |
| `index.js` | Emscripten runtime |
| `index.wasm` | Compiled game |
| `index.data` | Packaged game assets |

Keep all generated files together. Use an HTTP server for local previews;
double-clicking `index.html` with a `file://` URL does not work. Build output
is ignored by Git and does not need committing. `make clean` removes build
intermediates; `dist/browser/` remains until removed or rebuilt.

## Deploy with GitHub Pages

The repository contains the complete
[Browser Pages workflow](../.github/workflows/browser-pages.yml). It follows
GitHub's [custom Pages workflow](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages)
pattern: build an artifact, then deploy it with Pages permissions.

1. Review and commit the browser changes, then push or merge them into
   **`master`**, this repository's default branch. Generated `build/` and `dist/`
   files stay out of the commit.
2. Open the repository on GitHub. Under **Settings → Pages → Build and
   deployment**, set **Source** to **GitHub Actions**. If organizational Actions
   policies apply, allow the checkout and Pages actions used in the workflow.
3. Open **Actions → Browser Pages**. A push to `master` runs it automatically.
   For the first deployment or a rebuild, choose **Run workflow**, select
   **master**, then run it. If the earlier push failed because Pages was not
   enabled yet, rerun after completing step 2.
4. Wait for both **build** and **deploy** to succeed. The build installs the
   pinned SDK, runs `make browser`, and uploads only `dist/browser`. The deploy
   job uses the `github-pages` environment; approve that environment's deployment
   if your repository has configured protection rules.
5. Open the deployment URL in the workflow or **Settings → Pages**. For this
   repository, the expected URL is **https://christsantiris.github.io/c_rpg/**.
   Play, create a character, save in a slot, reload the page and load the slot.
   Music, movement, inventory, spells, fullscreen and mouse selection should
   also work.

Subsequent pushes to `master` redeploy automatically. Manual runs on another
branch build an artifact but skip publication; only `master` deploys. The
workflow grants read permissions during building and `pages: write` plus
`id-token: write` only to deployment. There is no PAT, `gh-pages` branch or
external game server to configure. If the default branch changes, update both
the workflow's `push.branches` and deploy job's `if` condition.

GitHub Pages serves the project under `/c_rpg/`. Asset requests use relative
paths, so the same files also work at another static host or repository path.
For another host, upload the entire `dist/browser/` directory and serve `.wasm`
as `application/wasm`. No cross-origin isolation headers are required.

## Saves and high scores

The launch page mounts the existing `saves/` directory on Emscripten's
[IDBFS](https://emscripten.org/docs/api_reference/Filesystem-API.html#filesystem-api-idbfs),
loads it before enabling Play, and automatically persists closed files to
IndexedDB. The game keeps its existing save/load code and save migrations.
Save slots, their character key bindings and Hall of Fame data survive reloads
in the same browser profile on the same site. Saving remains a manual in-game
action; closing the tab does not create a save.

Desktop saves and personal high scores are never included in the browser asset
bundle. Browser saves do not automatically transfer between devices, profiles,
local preview and GitHub Pages. Clearing browser site data removes them. If
storage is blocked or cannot be loaded, the launch page reports that saves are
temporary and still allows play. Leave the page open briefly after saving so
the asynchronous storage write can finish.

## Troubleshooting

- **Thin dark lines across zoomed areas:** the browser is using the software
  fallback because WebGL is unavailable. Enable hardware acceleration in the
  browser settings, or check `chrome://gpu` for WebGL support.

- **`emcmake` or `emmake` not found:** activate `emsdk_env.sh` in the terminal
  running Make. Browser builds need the SDK, not just a native compiler.
- **Pages says it cannot find the site:** select GitHub Actions as the Pages
  source, then rerun the workflow on `master`.
- **No deployment after a manual run:** select `master`; other branches only
  build. Check pending `github-pages` environment approvals.
- **Missing `.wasm` or `.data`:** deploy every generated file together. Check
  the workflow's uploaded artifact and the browser's network panel. Use the
  Pages URL including `/c_rpg/`, not the account root.
- **No music or keys:** start with Play and click the canvas to focus it. Check
  the game's music/SFX toggles and the browser's sound permission. Fullscreen
  may be unavailable in embedded pages; the game still works in its canvas.
- **Saves disappear:** use the same site and browser profile, allow site
  storage, and avoid clearing it. A storage warning means saves are temporary.

## Validation

Verified with Emscripten 4.0.23 and automated desktop Chrome checks served
under `/c_rpg/`: startup, audible music samples, character creation, movement,
inventory, journal, spellbook, game keys including F and Tab, mouse selection,
fullscreen, saving and loading through the game, save/high-score persistence
after reload, quit, and denied-storage fallback. The native build and all
5,261 native tests pass, and `actionlint` accepts the workflow. Live Pages
publication still requires the repository setup and push described above.
