
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
    fdja_value *t = fdja_parse_radial(rdz_strdup(s));
    fdja_set(r, "tree", t);
    //fdja_putdc(t);

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
    it "rewrites  a > b"
    {
      msg = mrad("a > b");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ >, {}, [ "
          "[ a, {}, [] ], "
          "[ b, {}, [] ] "
        "] ]");
    }

    it "rewrites  a or b or c"
    {
      msg = mrad("a or b or c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ or, {}, [ "
          "[ a, {}, [] ], "
          "[ b, {}, [] ], "
          "[ c, {}, [] ] "
        "] ]");
    }

    it "rewrites  a or b and c"
    {
      msg = mrad("a or b and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ or, {}, [ "
          "[ a, {}, [] ], "
          "[ and, {}, [ "
            "[ b, {}, [] ], "
            "[ c, {}, [] ] "
          "] ] "
        "] ]");
    }

    it "rewrites  a and (b or c)"
    {
      msg = mrad("a and (b or c)");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ and, {}, [ "
          "[ a, {}, [] ], "
          "[ or, {}, [ "
            "[ b, {}, [] ], "
            "[ c, {}, [] ] "
          "] ] "
        "] ]");
    }

    it "rewrites  (a or b) and c"
    {
      msg = mrad("(a or b) and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ and, {}, [ "
          "[ or, {}, [ "
            "[ a, {}, [] ], "
            "[ b, {}, [] ] "
          "] ], "
          "[ c, {}, [] ] "
        "] ]");
    }

    context "with 'if' or 'unless'"
    {
      it "rewrites  if a"
      {
        msg = mrad("if a");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ if, {}, [ "
            "[ a, {}, [] ] "
          "] ]");
      }

      it "rewrites  unless a"
      {
        msg = mrad("unless a");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ unless, {}, [ "
            "[ a, {}, [] ] "
          "] ]");
      }

      it "rewrites  if a > b"
      {
        msg = mrad("if a > b");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ if, {}, [ "
            "[ >, {}, [ "
              "[ a, {}, [] ], "
              "[ b, {}, [] ] "
            "] ] "
          "] ]");
      }

      it "rewrites  call x if a > b"
    }
  }
}

