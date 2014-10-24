
//
// specifying fl_exe_nodes.c
//
// Mon Oct 20 16:58:10 JST 2014
//

//#include <stdio.h>

#include "fl_executor.h"


context "fl_exe_nodes"
{
  before each
  {
    fdja_value *exe = fdja_c(
      "exid: xtest.pn-u0-20141020.0754.chipeduzuba\n"
      "nodes: {\n"
        "0: { tree:\n"
        "  [ sequence, {}, [\n"
        "    [ invoke, { _0: stamp, color: red }, [] ]\n"
        "    [ concurrence, {}, [\n"
        "      [ invoke, { _0: stamp, color: green }, [] ]\n"
        "      [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "    ] ]\n"
        "  ] ]\n"
        "}\n"
        "0_0: {}\n"
        "0_1: {}\n"
        "0_1_0: {}\n"
        "0_1_0-1: {}\n"
        "9_1_9: { parent: 9_0 }\n" // special
      "}");
  }
  after each
  {
    if (exe) fdja_free(exe);
  }

  describe "flon_node_tree()"
  {
    it "looks up the root tree"
    {
      fdja_value *t = flon_node_tree(exe, "0");

      expect(t != NULL);

      expect(fdja_to_json(t) ^==f ""
        "[\"sequence\",{},[");
    }

    it "looks up a sub tree"
    {
      fdja_value *t = flon_node_tree(exe, "0_1_0");

      expect(t != NULL);

      expect(fdja_to_json(t) ^==f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"green\"},[]]");
    }
  }

  describe "flon_node_parent_nid()"
  {
    it "returns NULL for node 0"
    {
      expect(flon_node_parent_nid(exe, "0") == NULL);
    }

    it "returns the parent nid"
    {
      expect(flon_node_parent_nid(exe, "0_1_0") ===f "0_1");
    }

    it "ignores the 'counter' part of the nid"
    {
      expect(flon_node_parent_nid(exe, "0_1_0-1") ===f "0_1");
    }

    it "returns the 'parent' if present"
    {
      expect(flon_node_parent_nid(exe, "9_1_9") ===f "9_0");
    }

    it "returns NULL if it doesn't find the node"
    {
      expect(flon_node_parent_nid(exe, "9") == NULL);
    }
  }
}

