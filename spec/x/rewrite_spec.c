
//
// specifying flon-executor
//
// Fri Jan 16 12:54:48 JST 2015
//

#include "djan.h"
#include "fl_executor.h"


context "flon-executor"
{
  fdja_value *mrad(char *s)
  {
    fdja_value *r = fdja_v("{ point: execute }");
    fdja_value *t = fdja_parse_radial(rdz_strdup(s), "sx");
    fdja_set(r, "tree", t);
    //fdja_putdc(t);

    return r;
  }
  fdja_value *mradp(char *s, fdja_value *p)
  {
    fdja_value *r = mrad(s);
    fdja_set(r, "payload", p);

    return r;
  }

  before each
  {
    fdja_free(execution); execution = fdja_v("{}");

    fdja_value *node = fdja_v("{ nid: 0_1, vars: {} }");
    fdja_value *msg = NULL;
  }
  after each
  {
    fdja_free(execution); execution = NULL;
    fdja_free(node);
    fdja_free(msg);
  }

  describe "flon_rewrite_tree()"
  {
    it "doesn't set 'tree' when there is no rewrite"
    {
      msg = mrad(
        ">\n"
        "  a\n"
        "  b\n"
      );

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ >, {}, 1, [ "
          "[ a, {}, 2, [] ], "
          "[ b, {}, 3, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f ">");
      expect(fdja_ld(node, "tree", NULL) === NULL);
    }

    it "rewrites  a > b"
    {
      msg = mrad("a > b");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ >, {}, 1, [ "
          "[ a, {}, 1, [] ], "
          "[ b, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f ">");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  > a b"
    {
      msg = mrad("> a b");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ >, {}, 1, [ "
          "[ a, {}, 1, [] ], "
          "[ b, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f ">");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  a or b or c"
    {
      msg = mrad("a or b or c");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ or, {}, 1, [ "
          "[ a, {}, 1, [] ], "
          "[ b, {}, 1, [] ], "
          "[ c, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f "or");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  a or b and c"
    {
      msg = mrad("a or b and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ or, {}, 1, [ "
          "[ a, {}, 1, [] ], "
          "[ and, {}, 1, [ "
            "[ b, {}, 1, [] ], "
            "[ c, {}, 1, [] ] "
          "] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f "or");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  a and (b or c)"
    {
      msg = mrad("a and (b or c)");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ and, {}, 1, [ "
          "[ a, {}, 1, [] ], "
          "[ or, {}, 1, [ "
            "[ b, {}, 1, [] ], "
            "[ c, {}, 1, [] ] "
          "] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f "and");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  (a or b) and c"
    {
      msg = mrad("(a or b) and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ and, {}, 1, [ "
          "[ or, {}, 1, [ "
            "[ a, {}, 1, [] ], "
            "[ b, {}, 1, [] ] "
          "] ], "
          "[ c, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f "and");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  $(cmp) x y"
    {
      msg = mradp("$(cmp) x y", fdja_v("{ cmp:  > }"));

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ >, {}, 1, [ "
          "[ x, {}, 1, [] ], "
          "[ y, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f ">");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  x $(cmp) y"
    {
      msg = mradp("x $(cmp) y", fdja_v("{ cmp:  > }"));

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ >, {}, 1, [ "
          "[ x, {}, 1, [] ], "
          "[ y, {}, 1, [] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f ">");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    it "rewrites  trace a or (trace b or trace c)"
    {
      msg = mrad("trace a or (trace b or trace c)");
      //fdja_putdc(fdja_l(msg, "tree"));

      flon_rewrite_tree(node, msg);

      expect(fdja_ld(msg, "tree") ===f ""
        "[ or, {}, 1, [ "
          "[ trace, { _0: a }, 1, [] ], "
          "[ or, {}, 1, [ "
            "[ trace, { _0: b }, 1, [] ], "
            "[ trace, { _0: c }, 1, [] ] "
          "] ] "
        "], sx ]");

      expect(fdja_ls(node, "inst", NULL) ===f "or");
      expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
    }

    context "with 'if' or 'unless'"
    {
      it "rewrites  if a"
      {
        msg = mrad("if a");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ a, {}, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  unless a"
      {
        msg = mrad("unless a");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ unless, {}, 1, [ "
            "[ a, {}, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "unless");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  if a > b"
      {
        msg = mrad("if a > b");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ >, {}, 1, [ "
              "[ a, {}, 1, [] ], "
              "[ b, {}, 1, [] ] "
            "] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  if a > b then c d"
      {
        msg = mrad(
          "if a > b then c d\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ >, {}, 1, [ "
              "[ a, {}, 1, [] ], "
              "[ b, {}, 1, [] ] "
            "] ], "
            "[ c, { _0: d }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  if a > b then c d else f g"
      {
        msg = mrad(
          "if a > b then c d else e f\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ >, {}, 1, [ "
              "[ a, {}, 1, [] ], "
              "[ b, {}, 1, [] ] "
            "] ], "
            "[ c, { _0: d }, 1, [] ], "
            "[ e, { _0: f }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  if a > b \\ c d"
      {
        msg = mrad(
          "if a > b\n"
          "  c d"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ >, {}, 1, [ "
              "[ a, {}, 1, [] ], "
              "[ b, {}, 1, [] ] "
            "] ], "
            "[ c, { _0: d }, 2, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "doesn't rewrite  if \\ a > b"
      {
        msg = mrad(
          "if \n"
          "  a > b\n"
          "  c d\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        //expect(fdja_ld(msg, "tree") ===f ""
        //  "[ if, {}, 1, [ "
        //    "[ >, {}, 2, [ "
        //      "[ a, {}, 2, [] ], "
        //      "[ b, {}, 2, [] ] "
        //    "] ], "
        //    "[ c, { _0: d }, 3, [] ] "
        //  "], sx ]");
        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 2, [] ], "
            "[ c, { _0: d }, 3, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  c d if a > b"
      it "rewrites  c d unless a > b"
    }
  }
}

