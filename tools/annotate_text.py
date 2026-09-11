#!/usr/bin/env python3
"""Compatibility entry point for reviewed English comments.

The former kana-only romanizer inserted {kanji} placeholders and erased
reviewed translations. It has been retired. Named script annotations and
legacy reference views now use the same exact-string translation sources.
"""
from translate_comments import main

if __name__ == '__main__':
    main()
