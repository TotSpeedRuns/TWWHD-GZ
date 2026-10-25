"""Validate, commit and push the curated source tree after a completed build."""
from pathlib import Path
import argparse, json, subprocess, sys, tempfile
ROOT=Path(__file__).resolve().parents[1]
from audit_source import audit
def run(args,**kw):return subprocess.run(args,check=True,cwd=ROOT,**kw)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path);p.add_argument('--revision');p.add_argument('--push',action='store_true');p.add_argument('--message',help='Commit description for an independently reviewed component sync');a=p.parse_args()
    if bool(a.candidate)!=bool(a.revision):raise ValueError('Supply candidate and revision together')
    if a.candidate:
        from export_revision import export
        export(a.candidate,a.revision)
    selected=audit();run([sys.executable,'-m','unittest','discover','-s','tests'])
    revision=json.loads((ROOT/'docs/revision.json').read_text())['revision']
    # Stage only the reviewed export; ignored ROMs/assets/builds cannot enter.
    scratch=ROOT/'build';scratch.mkdir(exist_ok=True)
    with tempfile.NamedTemporaryFile(mode='wb',delete=False,dir=scratch)as f:
        path=Path(f.name);f.write(b'\0'.join(p.relative_to(ROOT).as_posix().encode()for p in selected)+b'\0')
    try:run(['git','add','--pathspec-from-file='+str(path),'--pathspec-file-nul'])
    finally:path.unlink()
    # Refuse deletion of an already tracked source unless explicitly reviewed.
    deleted=run(['git','ls-files','--deleted'],capture_output=True,text=True).stdout.strip()
    if deleted:raise ValueError('Review removed source files before publishing: '+deleted)
    if subprocess.run(['git','diff','--cached','--quiet'],cwd=ROOT).returncode:
        run(['git','commit','-m',a.message or 'Update reviewed source for '+revision])
    if a.push:
        remote=run(['git','remote','get-url','origin'],capture_output=True,text=True).stdout.strip()
        if remote!='https://github.com/TotSpeedRuns/TWWHD-GZ.git':raise ValueError('Unexpected publication destination')
        run(['git','push','--set-upstream','origin','main'])
    print('Source publication checks complete for',revision)
if __name__=='__main__':main()
