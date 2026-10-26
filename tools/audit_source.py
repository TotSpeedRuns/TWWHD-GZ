"""Fail publication if excluded game files, private paths or credentials appear."""
from pathlib import Path
import argparse, re, subprocess
ROOT=Path(__file__).resolve().parents[1]
DIRECTORIES={'runtime','tools','patcher','creator','setup','experiments','docs','tests','.github'}
ROOT_FILES={'.gitignore','.gitattributes','.dockerignore','Dockerfile','LICENSE','README.md','NOTICE.md','requirements.txt','requirements-docker.txt','build.py','build-docker.sh','build-docker.ps1','CONTRIBUTING.md'}
SUFFIXES={'.py','.c','.cpp','.h','.hpp','.inc','.s','.asm','.in','.json','.md','.txt','.yml','.yaml','.ld','.sh','.ps1'}
def files(root=ROOT):
    for p in root.rglob('*'):
        rel=p.relative_to(root)
        if rel.parts[0] in {'local-assets','build','dist','.git'}or '__pycache__'in rel.parts:continue
        if not p.is_file():continue
        if p.is_symlink():raise ValueError('Symlink in source export: '+str(rel))
        if len(rel.parts)==1:
            if p.name not in ROOT_FILES:raise ValueError('Unreviewed root file: '+str(rel))
        elif rel.parts[0] not in DIRECTORIES:raise ValueError('Unreviewed source directory: '+str(rel))
        if p.suffix.lower()not in SUFFIXES and not p.name.upper().startswith(('LICENSE','COPYING','NOTICE'))and p.name not in ROOT_FILES:raise ValueError('Non-source file: '+str(rel))
        data=p.read_bytes()
        if b'\0'in data:raise ValueError('Binary data: '+str(rel))
        text=data.decode('utf-8-sig')
        patterns=[r'[A-Za-z]:[/\\]Users[/\\]',r'192[.]168[.]\d+[.]\d+',r'-----BEGIN [A-Z ]*PRIVATE KEY-----',r'gh[pousr]_[A-Za-z0-9]{30,}',r'github_pat_[A-Za-z0-9_]{30,}',r'(?i)encryptKeyWith\s*=\s*[\x22\x27][0-9a-f]{32}']
        if any(re.search(pattern,text)for pattern in patterns):raise ValueError('Private data pattern in '+str(rel))
        yield p
def audit():
    selected=list(files());template=(ROOT/'runtime/patch_wwhdgz.asm.in').read_text()
    if not re.search(r'(?m)^\.asset _gz_',template):raise ValueError('Artwork exclusion directives missing')
    return selected
if __name__=='__main__':print('Audited',len(audit()),'source files')
