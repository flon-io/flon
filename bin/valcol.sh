#!/usr/bin/env bash

while read LINE; do

  relos=' definitely lost: [1-9]'
  reval='^(--[0-9]+--|==[0-9]+==)'
  retal='^==> '

  if [[ $LINE =~ $relos ]]; then
    echo -n $'\e[1;31m'; echo $LINE; echo -n $'\e[0m'
  elif [[ $LINE =~ $reval ]]; then
    echo -n $'\e[1;30m'; echo $LINE; echo -n $'\e[0m'
  elif [[ $LINE =~ $retal ]]; then
    echo -n $'\e[32m'; echo "==>"
    echo -n $'\e[32m'; echo $LINE
    echo -n $'\e[32m'; echo "==>"
    echo -n $'\e[0m'
  else
    echo $LINE
  fi
done

