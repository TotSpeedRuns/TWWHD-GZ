# Docker build

Install Docker with Linux-container support. Docker supplies devkitPPC, Python
and Pillow, so those do not need to be installed on the host. Docker Desktop
must be running; on Windows, use Linux containers. Images are Linux amd64;
ARM hosts need amd64 emulation.

This command builds the **exported v126 runtime and patch payload**. It does not
reconstruct the complete newest GZ release, build GZ Setup, or package the desktop
installer. See [source status](SOURCE_STATUS.md) and [build scope](BUILD.md).

You still need your own unmodified European v0 `code/cking.rpx` and the matching
practice RPX accepted by `runtime/local-assets.json` for local artwork extraction.
Docker does not supply game executables or artwork. Mismatched input hashes are
rejected by the existing extraction/build commands.

Windows PowerShell, from the cloned repository:

```powershell
./build-docker.ps1 -OriginalRpx 'D:/games/original/code/cking.rpx' -PracticeRpx 'D:/games/matching-practice/code/cking.rpx'
```

Linux/macOS with Bash:

```sh
bash build-docker.sh /path/to/original/code/cking.rpx /path/to/matching-practice/code/cking.rpx
```

The source checkout and both input files are mounted read-only. Only the output
directory is writable. Temporary extracted artwork and compiler scratch data
stay in the disposable container. Game files are never copied into image layers
or the Docker build context: `.dockerignore` only allows the Dockerfile and
the pinned Python requirement file. Container builds disable the maintainer's
optional auto-publication setting and do not commit or push changes.

Results are written under a new `build/docker-TIMESTAMP/runtime/` directory,
including `cking.rpx`, the payload and `build-proof.json`. Use PowerShell's
`-OutputDirectory` or the Bash script's third argument to select another output
directory. Its `runtime/` subdirectory must not already exist. Keep these local
generated outputs out of Git.

## Checks without game files

```sh
docker build --platform linux/amd64 -t twwhd-gz-builder .
docker run --rm --platform linux/amd64 --mount "type=bind,source=$PWD,target=/project,readonly" twwhd-gz-builder --check
```

This runs the public source/asset-boundary tests and compiles the freestanding
runtime C components and import assembly with devkitPPC. It does not build a
complete RPX. GitHub Actions runs this check without receiving private game files.
Full runtime output validation requires the local input files above.

The toolchain image is pinned by tag and digest in `Dockerfile`; Pillow is pinned
in `requirements-docker.txt`. This provides a repeatable build setup, without
claiming the container reproduces unpublished source revisions or every release
artifact. See `NOTICE.md` for dependency license scopes.
