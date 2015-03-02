
require 'json'

task = JSON.parse(STDIN.read)

task['task']['state'] = 'offered'
task['task']['event'] = 'offering'
task['task']['from'] = 'trash'

STDOUT.puts(JSON.dump(task))

