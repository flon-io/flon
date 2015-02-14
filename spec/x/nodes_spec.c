
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
    fdja_free(execution);
    execution = fdja_c(
      "exid: xtest.pn-u0-20141020.0754.chipeduzuba\n"
      "nodes: {\n"
        "0: { tree:\n"
        "  [ sequence, {}, 1, [\n"
        "    [ invoke, { _0: stamp, color: red }, 2, [] ]\n"
        "    [ concurrence, {}, 3, [\n"
        "      [ invoke, { _0: stamp, color: green }, 4, [] ]\n"
        "      [ invoke, { _0: stamp, color: blue }, 5, [] ]\n"
        "      [ invoke, { _0: stamp, color: red }, 6, [] ]\n"
        "      [ or, { _0: a, _1: b }, 7, [] ]\n"
        "    ] ]\n"
        "  ] ]\n"
        "}\n"
        "0_0: {}\n"
        "0_1: {}\n"
        "0_1_0: {}\n"
        "0_1_0-1: {}\n"
        "0_1_2: { tree: [ sequence, {}, -1, [] ] }\n"
        "0_1_3: { tree: [ or, {}, 7, [ [ a, {}, 7, [] ], [ b, {}, 7, [] ] ] ] }\n"

        "9_1_9: { parent: 9_0 }\n" // special

        "1_0-1: { tree: [ or, {}, 7, [ [ a, {}, 7, [] ], [ b, {}, 7, [] ] ] ] }\n"
      "}");
  }
  after each
  {
    fdja_free(execution); execution = NULL;
  }

  describe "flon_node_tree()"
  {
    it "looks up the root tree"
    {
      fdja_value *t = flon_node_tree("0");

      expect(fdja_tod(t) ^==f ""
        "[ sequence, {}, 1, [");
    }

    it "looks up a sub tree"
    {
      fdja_value *t = flon_node_tree("0_1_0");

      expect(fdja_tod(t) ===f ""
        "[ invoke, { _0: stamp, color: green }, 4, [] ]");
    }

    it "looks up and returns updated trees"
    {
      fdja_value *t = flon_node_tree("0_1_2");

      expect(fdja_tod(t) ===f ""
        "[ sequence, {}, -1, [] ]");
    }

    it "looks up and returns rewritten trees"
    {
      fdja_value *t = flon_node_tree("0_1_3_0");

      expect(fdja_tod(t) ===f ""
        "[ a, {}, 7, [] ]");
    }

    it "looks up and returns rewritten trees (-{counter})"
    {
      fdja_value *t = flon_node_tree("0_1_3_0-77");

      expect(fdja_tod(t) ===f ""
        "[ a, {}, 7, [] ]");
    }

    it "looks up and returns rewritten trees (-{counter} 2)"
    {
      fdja_value *t = flon_node_tree("1_0_0-1");

      expect(fdja_tod(t) ===f ""
        "[ a, {}, 7, [] ]");
    }
  }

  describe "flon_parent_nid()"
  {
    it "returns NULL for node 0"
    {
      expect(flon_parent_nid("0") == NULL);
    }

    it "returns the parent nid"
    {
      expect(flon_parent_nid("0_1_0") ===f "0_1");
    }

    it "ignores the 'counter' part of the nid"
    {
      expect(flon_parent_nid("0_1_0-1") ===f "0_1");
    }

    it "returns the 'parent' if present"
    {
      expect(flon_parent_nid("9_1_9") ===f "9_0");
    }

    it "returns NULL if it doesn't find the node"
    {
      expect(flon_parent_nid("9") == NULL);
    }
  }
}

