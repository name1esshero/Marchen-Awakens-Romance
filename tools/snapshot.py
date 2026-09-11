#!/usr/bin/env python3
"""Create and verify a complete project recovery archive, excluding build output.

Includes editable assets/text, assembly/data, analysis, bundled agbcc and ROM.
Never deletes earlier snapshots. A separate empty directory is safest for restore.
"""
import hashlib
import json
import os
from pathlib import Path
import tarfile
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parent.parent
EXCLUDE_ROOT = {'backups', 'backup from before agbcc', 'build', 'mar.gba', '.git', '.agents', '.codex'}
EXCLUDE_ANY = {'__pycache__', '.git', '.pytest_cache'}


def digest(stream):
    return hashlib.file_digest(stream, 'sha256').hexdigest()


def main():
    stamp = datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S-%fZ')
    dest = ROOT / 'backups' / ('clean-' + stamp)
    dest.mkdir(parents=True)
    archive = dest / 'project.tar.gz'
    partial = dest / 'project.tar.gz.partial'
    records = {}
    with tarfile.open(partial, 'w:gz', compresslevel=6) as tar:
        for directory, dirs, files in os.walk(ROOT, followlinks=False):
            relative = Path(directory).relative_to(ROOT)
            dirs[:] = sorted(d for d in dirs if d not in EXCLUDE_ANY and
                             not (relative == Path('.') and d in EXCLUDE_ROOT))
            names = sorted(dirs + files)
            for name in names:
                if name in EXCLUDE_ANY or (relative == Path('.') and name in EXCLUDE_ROOT):
                    continue
                if name.endswith('.pyc'):
                    continue
                path = Path(directory) / name
                key = path.relative_to(ROOT).as_posix()
                info = tar.gettarinfo(str(path), arcname=key)
                # tar stores permission/special bits, not stat's file-type bits.
                info.mode &= 0o7777
                record = {'mode': info.mode}
                if info.isfile():
                    with path.open('rb') as source:
                        record.update(type='file', size=info.size, sha256=digest(source))
                        source.seek(0)
                        tar.addfile(info, source)
                elif info.issym():
                    record.update(type='symlink', target=info.linkname)
                    tar.addfile(info)
                elif info.isdir():
                    record.update(type='directory')
                    tar.addfile(info)
                else:
                    raise ValueError(f'Unsupported file type: {path}')
                records[key] = record
    print('Archive written; verifying all member hashes and modes', flush=True)
    seen = set()
    with tarfile.open(partial, 'r:gz') as tar:
        for member in tar:
            record = records[member.name]
            assert member.mode == record['mode'], member.name
            if member.isfile():
                with tar.extractfile(member) as data:
                    assert digest(data) == record['sha256'], member.name
            elif member.issym():
                assert member.linkname == record['target'], member.name
            seen.add(member.name)
    assert seen == records.keys(), 'Archive member list differs'
    partial.rename(archive)
    with archive.open('rb') as stream:
        checksum = digest(stream)
    (dest / 'SHA256SUMS').write_text(f'{checksum}  project.tar.gz\n')
    (dest / 'manifest.json').write_text(json.dumps(records, indent=2) + '\n')
    (dest / 'RESTORE.txt').write_text(
        'Verified project snapshot (UTC ' + stamp + ').\n'
        'Includes original ROM, agbcc, current assembly/data, editable graphics,\n'
        'translations, analysis, tools, tests and reports. Excludes prior backups,\n'
        'build/, mar.gba, caches and local agent/Git configuration.\n\n'
        'Restore into a new EMPTY directory to avoid stale files:\n'
        '  cd /path/to/this/backup\n'
        '  sha256sum -c SHA256SUMS\n'
        '  mkdir /path/to/restored-project\n'
        '  tar -xzf project.tar.gz -C /path/to/restored-project\n'
        '  cd /path/to/restored-project\n'
        '  make -j4 compare\n\n'
        'Requires make, Python 3 with project dependencies, and arm-none-eabi\n'
        'binutils/cpp on PATH (see README.md). Bundled agbcc modes are preserved.\n'
        'manifest.json records every archived file hash, mode and symlink target.\n')
    print(f'Snapshot verified: {dest.relative_to(ROOT)} ({len(records)} entries, {archive.stat().st_size} bytes)', flush=True)


if __name__ == '__main__':
    main()
