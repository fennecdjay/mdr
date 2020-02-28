#!/usr/bin/bash

set -Eeu

readonly dir="result"

if [ -t 1 ] || [ -n "$GITHUB_ACTIONS" ]
then
  GREEN="\033[32m"
  RED="\033[31m"
  CLEAR="\033[0m"
  BOLD="\033[1m"
else
  GREEN=""
  RED=""
  CLEAR=""
  BOLD=""
fi

[ -d "$dir" ] || mkdir "$dir"

ok() {
  printf "${GREEN}OK${CLEAR} ${BOLD}% 3i${CLEAR} %s\n" "$1" "$2"
  return 0
}

not_ok() {
  printf "${RED}NOT OK${CLEAR} ${BOLD}% 3i${CLEAR} %s\n" "$1" "$2"
  return 1
}

file_test() {
  number=$1
  file=$2
  filename="$(basename $file)"
  err="tests/err/${filename:${#filename}-3}err"
  md="tests/result/${filename:${#filename}-1}"
  err_result="${file}.err"

  ./mdr "$2" 2> "$err_result"

  if [ -f "$md" ]
  then if [ -n "$(diff "$md" "$md_result")" ]
  then
    not_ok "$number" "$file"
    return 1
  fi
  elif [ -f "$err" ] && [[ $(cat "$err_result") != *"$(cat "$err")"* ]]
  then
    not_ok "$number" "$file"
    return 1
  fi

  ok "$number" "$file"
  return 0
}

TOTAL=$((2 + $(find tests/*.mdr | wc -l)))
index=1
echo "$index..$TOTAL"
if [ "$(./mdr 2>&1)" = "usage: mdr <files>" ]
then ok "$index" "usage"
else not_ok "$index" "usage"
fi

for file in tests/non_existant_file.mdr $(find tests/*.mdr)
do
  index=$((index + 1))
  file_test "$index" "$file"
done
