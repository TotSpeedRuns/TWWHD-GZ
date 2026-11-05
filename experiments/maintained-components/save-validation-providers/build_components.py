"""Rebuild three exact authored PPC32 objects; no game input or console IO."""
from pathlib import Path
import argparse, hashlib, json, subprocess

HERE = Path(__file__).resolve().parent
CONFIG_SHA = 'f31e86aa53a24a9c4c64e51c259d0499926d23b4d13b779541bb8b978b2d5fd2'

def need(ok, why):
    if not ok:
        raise ValueError(why)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--compiler', type=Path, required=True, help='devkitPPC r49 powerpc-eabi-gcc')
    ap.add_argument('--output', type=Path, required=True, help='New directory outside this component')
    args = ap.parse_args()
    config_path = HERE / 'components.json'
    need(sha(config_path) == CONFIG_SHA, 'Changed component specification')
    config = json.loads(config_path.read_bytes())
    compiler = args.compiler.resolve(strict=True)
    need(compiler.is_file() and sha(compiler) == config['compiler_sha256'], 'Exact qualified devkitPPC compiler required')
    for name, digest in config['source_sha256'].items():
        path = HERE / name
        need(path.is_file() and not path.is_symlink() and sha(path) == digest, 'Changed authored source: ' + name)
    output = args.output.resolve()
    need(not output.exists() and not output.is_relative_to(HERE), 'New output outside source component required')
    output.mkdir(parents=True)
    results = []
    for entry in config['objects']:
        target = output / entry['output']
        command = [str(compiler), *entry['flags'], '-c', str(HERE / entry['source']), '-o', str(target)]
        run = subprocess.run(command, capture_output=True, text=True)
        need(run.returncode == 0, 'Compiler failed: ' + run.stderr)
        need(sha(target) == entry['sha256'], 'Object differs from qualified component: ' + entry['output'])
        results.append(dict(name=entry['output'], bytes=target.stat().st_size, sha256=sha(target)))
    report = dict(schema='authored-save-validation-object-rebuild/1', passed=True,
        objects=results, source_only=True, SDK_or_game_input=False,
        target_code_executed=False, console_IO=False, runtime_image_built=False)
    (output / 'build-result.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps(report))

if __name__ == '__main__':
    main()
