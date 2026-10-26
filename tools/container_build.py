"""Build the exported v126 runtime using read-only, locally mounted game inputs."""
from pathlib import Path
import argparse, os, subprocess, sys, tempfile

ROOT = Path(__file__).resolve().parents[1]

def run(*args, env=None):
    subprocess.run([str(arg) for arg in args], cwd=ROOT, env=env, check=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-rpx', type=Path, default=Path('/inputs/original.rpx'))
    parser.add_argument('--practice-rpx', type=Path, default=Path('/inputs/practice.rpx'))
    parser.add_argument('--output', type=Path, default=Path('/output/runtime'))
    parser.add_argument('--check', action='store_true', help='Source tests and native compile checks; no game files needed')
    args = parser.parse_args()
    toolchain = Path(os.environ['DEVKITPPC'])/'bin'
    with tempfile.TemporaryDirectory(prefix='gz-container-') as scratch:
        scratch = Path(scratch)
        env = dict(os.environ, GZ_TEST_TMP=str(scratch), PYTHONDONTWRITEBYTECODE='1',
                   GIT_CONFIG_COUNT='1', GIT_CONFIG_KEY_0='gz.autopublish', GIT_CONFIG_VALUE_0='false')
        if args.check:
            run(sys.executable, ROOT/'tools/audit_source.py', env=env)
            run(sys.executable, '-m', 'unittest', 'discover', '-s', ROOT/'tests', env=env)
            from build import FLAGS
            for name in ('morth_trick', 'morth_setup', 'morth_setup_data'):
                run(toolchain/'powerpc-eabi-gcc', *FLAGS, '-c', ROOT/'runtime'/(name+'.c'), '-o', scratch/(name+'.o'), env=env)
            run(toolchain/'powerpc-eabi-gcc', '-mcpu=750', '-c', '-x', 'assembler', ROOT/'runtime/imports.s', '-o', scratch/'imports.o', env=env)
            print('Source tests and native component compiles passed; full RPX build requires local inputs.')
            return
        for label, path in (('original', args.original_rpx), ('practice', args.practice_rpx)):
            if not path.is_file():
                parser.error(f'Mount your own {label} RPX read-only at {path}; see docs/DOCKER.md')
        if args.output.exists():
            parser.error('Output must not already exist. Use a fresh output directory.')
        assets = scratch/'assets'
        run(sys.executable, ROOT/'tools/local_assets.py', '--practice-rpx', args.practice_rpx,
            '--destination', assets, env=env)
        run(sys.executable, ROOT/'build.py', '--original-rpx', args.original_rpx,
            '--toolchain', toolchain, '--assets', assets, '--output', args.output, env=env)

if __name__ == '__main__':
    # Allow importing the existing root build configuration from this tools script.
    sys.path.insert(0, str(ROOT))
    main()
