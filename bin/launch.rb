
dom = ARGV[0] || 'launch.rb'

id = `tst/bin/flon-flon -i #{dom}`.strip

f = File.open("tst/var/spool/dis/exe_#{id}.json", 'w')
f.write(%[
{
  point: execute
  tree:
    [ sequence, {}, [
      [ invoke, { _0: stamp, color: blue }, [] ]
      [ invoke, { _0: stamp, color: green }, [] ]
      [ invoke, { _0: stamp, color: red }, [] ]
      #[ invoke, { _0: sgmail, subjet: lila }, [] ]
    ] ]
  exid: #{id}
  payload: {
    hello: world
  }
}
])
f.close

