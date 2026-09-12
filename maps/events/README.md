# Editable script arguments

`make map-editor` can save existing constant native-call arguments here.
Keys are CODE-relative offsets of signed 32-bit immediate values. Source
hashes and FUNC references constrain edits to the original script revision.
Only verified contiguous integer-push sequences are exposed; dynamic calls,
script insertion/removal, and unclassified variable tables are not editable.

The build applies these changes after rebuilding text, refuses compression
allocation overflow, and never changes the default ROM when no edits exist.
Removing an override restores the arguments from the script source.
These JSON files are source inputs, not generated reports.
