#!/bin/sh
# Complete, verified recovery snapshot; errors are fatal and old backups remain.
set -eu
exec python3 "$(dirname "$0")/snapshot.py"
