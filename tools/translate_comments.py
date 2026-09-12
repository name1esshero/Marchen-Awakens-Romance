#!/usr/bin/env python3
"""Apply reviewed exact-string translations; never replace Japanese ROM text.

Names are transliterations, not claims about official localized spellings.
ASCII literals are marked separately and never counted as translated dialogue.
"""
import collections
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent.parent


def main():
    glossary = json.loads((ROOT / 'text/translation/english.json').read_text())
    scoped_path = ROOT / 'text/translation/scripts.json'
    scoped = json.loads(scoped_path.read_text()) if scoped_path.exists() else {}
    counts = collections.Counter()
    for path in sorted((ROOT / 'text').glob('*.txt')) + sorted((ROOT / 'text/nfp').glob('*.txt')):
        # Short dialogue fragments depend on their scene. Keep these local
        # instead of assigning their English wording to every exact match.
        lookup = dict(glossary)
        if path.parent.name == 'nfp':
            lookup.update(scoped.get(path.name.removesuffix('.txt'), {}))
        lines = path.read_text(encoding='utf-8').splitlines(keepends=True)
        for i, line in enumerate(lines):
            if not line.startswith('@'):
                continue
            body = line.rstrip('\n').partition('  //')[0]
            original = body.split(' ', 1)[1]
            # Preserve formatting tokens verbatim in the original. Removing
            # these from a comment lookup does not assign them engine meanings.
            clean = re.sub(r' ?C[0-9A-F]{4} ?', '', original).strip()
            if path.parent.name == 'nfp' and not clean:
                lines[i] = body + '  // FORMAT: Blank text / color controls only; no translatable characters.\n'
                counts['formatting_records'] += 1
                continue
            name = re.fullmatch(r'［([^］]+)］', clean)
            translated = lookup.get(name[1] if name else clean)
            if translated and name:
                translated = '[' + translated + ']'
            if not translated:
                for punctuation in ('！！', '！', '…'):
                    if clean.endswith(punctuation) and clean[:-len(punctuation)] in lookup:
                        translated = lookup[clean[:-len(punctuation)]] + {'！！':'!!','！':'!','…':'...'}[punctuation]
                        break
            if translated:
                lines[i] = body + '  // EN: ' + translated + '\n'
                counts['english_comments'] += 1
            elif path.parent.name == 'nfp' and clean and clean.isascii():
                lines[i] = body + '  // LITERAL: ' + clean + ' (usage not inferred)\n'
                counts['ascii_literals'] += 1
        out = ''.join(lines)
        if out != path.read_text(encoding='utf-8'):
            path.write_text(out, encoding='utf-8')
    from sync_legacy_text import sync
    print(dict(counts))
    print(sync(ROOT))


if __name__ == '__main__':
    main()
