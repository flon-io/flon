
#
# a flon computing service
#
# Mon Oct 20 15:25:37 JST 2014
#

#require 'pp'
require 'json'

task = JSON.parse(STDIN.read)

numbers = task['numbers']

task['total'] =
  if (numbers && numbers.class == Array)
    numbers.reduce(&:+)
  else
    0
  end

puts(JSON.dump(task))

t = Time.now
STDERR.puts("#{t} .#{t.usec} xxx #{Process.pid} compute.rb over.")

