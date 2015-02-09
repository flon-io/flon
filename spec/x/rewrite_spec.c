
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

    context "'>':"
    {
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

      it "rewrites  a b > c d"
      {
        msg = mrad("a b > c d");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ >, {}, 1, [ "
            "[ a, { _0: b }, 1, [] ], "
            "[ c, { _0: d }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f ">");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }
    }

    context "'and', 'or':"
    {
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
            "[ b, { _0: and, _1: c }, 1, [] ] "
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
            "[ b, { _0: or, _1: c }, 1, [] ] "
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
            "[ a, { _0: or, _1: b }, 1, [] ], "
            "[ c, {}, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "and");
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
            "[ trace, { _0: b, _1: or, _2: trace, _3: c }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "or");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }
    }

    context "$(cmp):"
    {
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
    }

    context "head 'if':"
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
            "[ a, { _0: >, _1: b }, 1, [] ] "
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
          "[ ife, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 1, [] ], "
            "[ c, { _0: d }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "ife");
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
          "[ ife, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 1, [] ], "
            "[ c, { _0: d }, 1, [] ], "
            "[ e, { _0: f }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "ife");
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
            "[ a, { _0: >, _1: b }, 1, [] ], "
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

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 2, [] ], "
            "[ c, { _0: d }, 3, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  if true"
      {
        msg = mrad(
          "if true\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ if, {}, 1, [ "
            "[ val, { _0: true }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "if");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  elif true"
      {
        msg = mrad(
          "elif true\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ elif, {}, 1, [ "
            "[ val, { _0: true }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "elif");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  elsif true"
      {
        msg = mrad(
          "elsif true\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ elsif, {}, 1, [ "
            "[ val, { _0: true }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "elsif");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }
    }

    context "tail 'if':"
    {
      it "rewrites  c d if a > b"
      {
        msg = mrad(
          "c d if a > b\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ ife, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 1, [] ], "
            "[ c, { _0: d }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "ife");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  sequence if a > b \\ c \\ d"
      {
        msg = mrad(
          "c if a > b\n"
          "  e f\n"
          "  g h\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ ife, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 1, [] ], "
            "[ c, {}, 1, [ "
              "[ e, { _0: f }, 2, [] ], "
              "[ g, { _0: h }, 3, [] ] "
            "] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "ife");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  c d unless a > b"
      {
        msg = mrad(
          "c d unless a > b\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ unlesse, {}, 1, [ "
            "[ a, { _0: >, _1: b }, 1, [] ], "
            "[ c, { _0: d }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "unlesse");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }
    }

    context "'else if':"
    {
      it "flips burgers"
    }

    context "'set':"
    {
      it "doesn't rewrite  set"
      {
        msg = mrad(
          "set\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ set, {}, 1, [], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "set");
        expect(fdja_ld(node, "tree", NULL) ===f NULL);
      }

      it "doesn't rewrite  set k\\v"
      {
        msg = mrad(
          "set k\n"
          "  v\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ set, { _0: k }, 1, [ "
            "[ v, {}, 2, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "set");
        expect(fdja_ld(node, "tree", NULL) ===f NULL);
      }

      it "rewrites  set k: v"
      {
        msg = mrad(
          "set k: v\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ set, { _0: k }, 1, [ "
            "[ v, {}, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "set");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  set k0: v0, k1: v1"
      {
        msg = mrad(
          "set k0: v0, k1: v1\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ sequence, {}, 1, [ "
            "[ set, { _0: k0 }, 1, [ "
              "[ v0, {}, 1, [] ] "
            "] ], "
            "[ set, { _0: k1 }, 1, [ "
              "[ v1, {}, 1, [] ] "
            "] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "sequence");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  set k0: v0, k1: (a + 1), k2: v2"
      {
        msg = mrad(
          "set k0: v0, k1: (a + 1), k2: v2\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ sequence, {}, 1, [ "
            "[ set, { _0: k0 }, 1, [ "
              "[ v0, {}, 1, [] ] "
            "] ], "
            "[ set, { _0: k1 }, 1, [ "
              "[ a, { _0: +, _1: 1 }, 1, [] ] "
            "] ], "
            "[ set, { _0: k2 }, 1, [ "
              "[ v2, {}, 1, [] ] "
            "] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "sequence");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  set k: 1"
      {
        msg = mrad(
          "set k: 1\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ set, { _0: k }, 1, [ "
            "[ val, { _0: 1 }, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "set");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }

      it "rewrites  set k: blue"
      {
        msg = mrad(
          "set k: blue\n"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_ld(msg, "tree") ===f ""
          "[ set, { _0: k }, 1, [ "
            "[ blue, {}, 1, [] ] "
          "], sx ]");

        expect(fdja_ls(node, "inst", NULL) ===f "set");
        expect(fdja_ld(node, "tree", NULL) ===F fdja_ld(msg, "tree"));
      }
    }
  }
}

