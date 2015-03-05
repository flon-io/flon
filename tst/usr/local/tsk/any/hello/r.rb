
# saying hello
#
# Thu Mar  5 13:18:57 JST 2015


require 'json'

task = JSON.parse(STDIN.read)

task['hello'] = task['name']
task.delete('name')

STDOUT.puts(JSON.dump(task))

