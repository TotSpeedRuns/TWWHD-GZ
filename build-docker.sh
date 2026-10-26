#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: bash build-docker.sh ORIGINAL_RPX MATCHING_PRACTICE_RPX [OUTPUT_DIRECTORY]" >&2
    exit 2
fi
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
absolute_file() {
    [[ -f "$1" ]] || { echo "Input file does not exist: $1" >&2; exit 2; }
    printf '%s/%s\n' "$(cd -- "$(dirname -- "$1")" && pwd)" "$(basename -- "$1")"
}
original="$(absolute_file "$1")"
practice="$(absolute_file "$2")"
output="${3:-$repo_root/build/docker-$(date -u +%Y%m%dT%H%M%SZ)}"
mkdir -p "$output"
output="$(cd -- "$output" && pwd)"
docker build --platform linux/amd64 -t twwhd-gz-builder "$repo_root"
docker run --rm --platform linux/amd64 --user "$(id -u):$(id -g)" \
    --mount "type=bind,source=$repo_root,target=/project,readonly" \
    --mount "type=bind,source=$original,target=/inputs/original.rpx,readonly" \
    --mount "type=bind,source=$practice,target=/inputs/practice.rpx,readonly" \
    --mount "type=bind,source=$output,target=/output" twwhd-gz-builder
