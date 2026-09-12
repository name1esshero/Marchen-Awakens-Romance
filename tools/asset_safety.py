"""Protect authored English PNGs and the source paths that select them."""
from pathlib import Path


def has_english_art(path):
    path = Path(path)
    return path.suffix.lower() == '.png' and (
        path.stem.lower().endswith('_en') or
        path.with_stem(path.stem + '_en').exists())


def unlink_generated(path):
    """Cleanup may remove generated files, never localized art or its owner."""
    path = Path(path)
    if has_english_art(path):
        raise ValueError('Refusing to delete English artwork or its source: ' + str(path))
    path.unlink()
