# Documentation publishing

The source branch retains editable assets, translation mappings, generators,
and build code. Generated gallery HTML and `reports/` are ignored locally.
The wiki holds format/editing documentation and report navigation. GitHub Pages
serves interactive galleries and report snapshots from the separate `gh-pages`
branch; `.nojekyll` keeps asset names unchanged.

## Prepare and review

On a fresh clone, run `make docs-fetch` once. This clones the published branch
under `build/published-docs` and restores only missing report/gallery files.
It never replaces current reports or editable sources. Historical proofs are
snapshots, so they are fetched, not recreated as though they were new results.

Run the relevant audit commands when you want new evidence. `make galleries`
only regenerates viewing HTML; it does not extract assets or run ROM audits.
Then run `make site`. It stages allowed files from `graphics/`, `sound/`,
`text/`, and `reports/` in `build/site`, checks local HTML links and dynamic
animation frame paths, includes the real map-editor demonstration from
`docs/media/`, and writes `publication.json` with per-file hashes.
The ROM, toolchain, credentials, backups, and Git metadata are never staged.
Generated wiki pages go in `build/wiki`.

Preview using `python3 -m http.server 8000 --directory build/site`.
Staged pages are viewing/download copies; changes to those copies do not edit
the local decomp. The site does not host the map editor's saving backend.

## Publish

Clone `https://github.com/name1esshero/Marchen-Awakens-Romance.git` with
`--single-branch --branch gh-pages` into a separate checkout. Copy the reviewed
`build/site/` contents into it, preserving its `.git` and any historical
snapshots not regenerated locally. Inspect the diff, commit, and push normally;
do not force-push or switch the decomp's working branch.

Clone `https://github.com/name1esshero/Marchen-Awakens-Romance.wiki.git`
separately. Merge the reviewed `build/wiki/` pages into it, retaining manually
written additions to existing pages, then commit and push. Alternatively,
pass that wiki checkout to `tools/publish_docs.py --wiki PATH` while staging:
the existing Home text before the generated navigation marker is preserved.
Other generated page names and `_Sidebar.md` are replaced, so review their diff.

After GitHub reports the Pages deployment successful, check the live HTML,
images, audio, and report links before removing any local-only historical data.
`make clean` removes `build/site` and the temporary documentation checkouts;
it does not delete ignored local report snapshots.
