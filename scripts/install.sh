#!/usr/bin/env bash
set -euo pipefail
PREFIX=${1:-/usr/local}
echo "Installing to $PREFIX"
install -Dm755 build/acx "$PREFIX/bin/acx"
install -Dm644 man/acx.1 "$PREFIX/share/man/man1/acx.1" || true
install -Dm644 completions/acx.bash "$PREFIX/share/bash-completion/completions/acx" || true
install -Dm644 completions/_acx "$PREFIX/share/zsh/site-functions/_acx" || true
install -Dm644 completions/acx.fish "$PREFIX/share/fish/vendor_completions.d/acx.fish" || true
echo "Done."
