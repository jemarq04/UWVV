function _listsettings {
  local filename="local.cfg"
  [[ ! -f $filename ]] && return
  if [[ ${#COMP_WORDS[@]} -eq 2 ]]; then
    compopt -o default
    COMPREPLY=()
  elif [[ ${#COMP_WORDS[@]} -eq 3 ]]; then
    local settings_list=( $(grep "^\s*\[.*\]\s*$" $filename) )
    local settings_list=(${settings_list[@]##*\[})
    local settings_list=(${settings_list[@]%%\]*})
    local settings_list=(${settings_list[@]/DEFAULT})
    COMPREPLY=( $(compgen -W "${settings_list[*]}" -- $2) )
  fi
}
complete -F _listsettings crabSubmit.sh
