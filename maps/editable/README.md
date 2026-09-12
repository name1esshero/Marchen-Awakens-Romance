# Editable map overrides

`make map-editor` saves KMP tile planes and raw attributes here as JSON.
Each document records its source hash, dimensions, plane offsets and words.
The build applies these after the existing assembled-image layouts. Unspecified
KMP headers and bytes remain intact. Removing an override restores the layout
from the existing source files. Directory dependencies make removal rebuild.

These JSON files are editable source inputs and should be committed.
