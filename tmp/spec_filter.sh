
#
# spec_filter.sh
#
# Thu Nov 27 14:32:12 JST 2014
#
# outputs success lines in green
# and failure lines in red
#

while read LINE; do

  # green
  echo $LINE | GREP_COLORS="mt=01;32" grep -E " 0 failures" || :

  # yellow
  echo $LINE | GREP_COLORS="mt=01;33" grep -E " [0-9]+ pending" || :

  # red
  echo $LINE | grep -E "[1-9][0-9]* failures" || :
  echo $LINE | grep -E "^make spec L=[0-9]+" || :
  echo $LINE | grep -E "definitely lost: [1-9][0-9]* bytes" || :
  #
  if [ ! -z "$1" ]; then
    echo $LINE | grep -E " [1-9][,0-9]* bytes in " || :
    echo $LINE | grep -E " at 0x" || :
    echo $LINE | grep -E " by 0x" || :
  fi

  echo $LINE | grep -E -i "segmentation fault" || :
  echo $LINE | grep -E -i "signal [0-9]+" || :

  echo $LINE | grep -E ": warning: " || :
done

