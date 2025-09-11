# SPDX-License-Identifier: Apache-2.0
# Bash completion for 'acx'
_acx_complete() {
  local cur prev words cword
  _init_completion -n : || return
  local commands=""
  if [[ $cword -eq 1 ]]; then
    COMPREPLY=( $(compgen -W "$commands" -- "$cur") )
    return
  fi
}
complete -F _acx_complete acx
