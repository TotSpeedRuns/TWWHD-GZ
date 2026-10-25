"""Sync explicit, hash-pinned authored components without exporting game data.

The input manifest stays local. Only reviewed destination names and source
hashes enter the repository. This does not replace the integrated v126 builder
or label an incomplete component snapshot as a newer full release.
"""
from pathlib import Path, PurePosixPath
import argparse, hashlib, json, re, subprocess, sys
from audit_source import files, audit
ROOT=Path(__file__).resolve().parents[1]
PREFIX=PurePosixPath('experiments/maintained-components')

def destination(name):
    p=PurePosixPath(name)
    if p.is_absolute() or '..'in p.parts or '\\'in name or ':'in name or not p.is_relative_to(PREFIX):
        raise ValueError('Destination outside maintained components: '+name)
    if p.suffix not in {'.c','.h','.s','.inc','.md','.py','.json'}:
        raise ValueError('Not an authored source destination: '+name)
    target=ROOT.joinpath(*p.parts)
    if not target.resolve().is_relative_to(ROOT.resolve()):
        raise ValueError('Destination resolves outside source repository')
    return target

def sync(manifest):
    if not re.fullmatch(r'[A-Za-z0-9.-]{1,64}',manifest['revision']):
        raise ValueError('Invalid component revision')
    prepared=[];seen=set()
    for row in manifest['files']:
        target=destination(row['destination'])
        if target in seen:raise ValueError('Duplicate destination')
        seen.add(target);source=Path(row['source'])
        if source.is_symlink()or not source.is_file():raise ValueError('Source must be a regular file')
        data=source.read_bytes();digest=hashlib.sha256(data).hexdigest()
        if digest!=row['sha256']:raise ValueError('Source changed before sync: '+source.name)
        if b'\0'in data:raise ValueError('Binary source refused')
        text=data.decode('utf-8-sig')
        if re.search(r'[A-Za-z]:[/\\]Users[/\\]|192[.]168[.]\d+[.]\d+|-----BEGIN [A-Z ]*PRIVATE KEY-----|github_pat_[A-Za-z0-9_]{30,}|gh[pousr]_[A-Za-z0-9]{30,}',text):
            raise ValueError('Private data pattern in component')
        prepared.append((target,data,digest))
    if not prepared:raise ValueError('No source components selected')
    # All sources, destinations and hashes are checked before any copy.
    for target,data,digest in prepared:
        target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    selected=list(files(ROOT))
    if not all(p in selected for p,_,_ in prepared):raise ValueError('Component outside audited source selection')
    report=dict(schema='maintained-source-components/1',revision=manifest['revision'],
        status='Reviewed components; full integrated build export remains separately versioned.',
        files=[dict(path=p.relative_to(ROOT).as_posix(),sha256=d,bytes=len(b))for p,b,d in prepared],
        excludes=['Game executables','Game artwork','Raw user captures','Save files','Keys','Private diagnostics'])
    (ROOT/'docs/maintained-components.json').write_text(json.dumps(report,indent=2)+'\n')
    audit();return report

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',type=Path,required=True);p.add_argument('--publish',action='store_true');a=p.parse_args()
    report=sync(json.loads(a.manifest.read_text(encoding='utf-8-sig')))
    if a.publish:subprocess.run([sys.executable,ROOT/'tools/publish.py','--push','--message','Update maintained components for '+report['revision']],check=True,cwd=ROOT)
    print(json.dumps(dict(revision=report['revision'],files=len(report['files']),published=a.publish)))
if __name__=='__main__':main()
