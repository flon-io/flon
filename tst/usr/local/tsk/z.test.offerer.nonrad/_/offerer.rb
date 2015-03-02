
require 'json'

task = JSON.parse(STDIN.read)

task['task']['for'] = 'stamp'

STDOUT.puts(JSON.dump(task))

