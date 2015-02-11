
#
# spec_filter.rb
#
# Thu Nov 27 14:32:12 JST 2014 (.sh)
# Wed Feb 11 18:32:03 JST 2015 (.rb)
#
# outputs success lines in green
# and failure lines in red
#

#static char *fgaj_red(int c) { return c ? "[0;31m" : ""; }
#static char *fgaj_green(int c) { return c ? "[0;32m" : ""; }
#static char *fgaj_brown(int c) { return c ? "[0;33m" : ""; }
#static char *fgaj_blue(int c) { return c ? "[0;34m" : ""; }
#static char *fgaj_cyan(int c) { return c ? "[0;36m" : ""; }
#static char *fgaj_white(int c) { return c ? "[0;37m" : ""; }
#static char *fgaj_yellow(int c) { return c ? "[1;33m" : ""; }
#static char *fgaj_clear(int c) { return c ? "[0;0m" : ""; }

def red(s)
  "[0;31m#{s}[0;0m"
end
def green(s)
  "[0;32m#{s}[0;0m"
end
def yellow(s)
  "[0;33m#{s}[0;0m"
end

if ARGV[0]
  #puts
  puts "** #{ARGV[0]}"
  #puts "*"
end

ARGF.each do |l|

  l = l.chop

  if l.match(/ [1-9][0-9]* failures/) then puts red(l); next; end
  if l.match(/ [1-9][0-9]* pending/) then puts yellow(l); next; end
  if l.match(/ 0 failures/) then puts green(l); next; end

  if l.match(/: warning: /) then puts yellow(l); next; end

  if l.match(/^make spec L=[0-9]+/) then puts red(l); next; end
  if l.match(/definitely lost: [1-9][0-9]* bytes/) then puts red(l); next; end

  if l.match(/ lost: [1-9][0-9]* bytes in /) then puts red(l); next; end
  if l.match(/ at 0x /) then puts red(l); next; end
  if l.match(/ by 0x /) then puts red(l); next; end

  if l.match(/segmentation fault/) then puts red(l); next; end
  if l.match(/signal [0-9]+/) then puts red(l); next; end
end

