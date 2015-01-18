
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
          "[ a, { _: g }, [] ], "
          "[ b, { _: h }, [] ] "
        "] ]");
    }

    it "rewrites  a or b or c"
    {
      msg = mrad("a or b or c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ or, {}, [ "
          "[ a, { _: g }, [] ], "
          "[ b, { _: h }, [] ], "
          "[ c, { _: i }, [] ] "
        "] ]");
    }

    it "rewrites  a or b and c"
    {
      msg = mrad("a or b and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ or, {}, [ "
          "[ a, { _: g }, [] ], "
          "[ and, { _: j }, [ "
            "[ b, { _: h }, [] ], "
            "[ c, { _: i }, [] ] "
          "] ] "
        "] ]");
    }

    it "rewrites  a and (b or c)"
    {
      msg = mrad("a and (b or c)");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ and, {}, [ "
          "[ a, { _: g }, [] ], "
          "[ or, { _: j }, [ "
            "[ b, { _: h }, [] ], "
            "[ c, { _: i }, [] ] "
          "] ] "
        "] ]");
    }

    it "rewrites  (a or b) and c"
    {
      msg = mrad("(a or b) and c");

      flon_rewrite_tree(node, msg);

      expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
        "[ and, {}, [ "
          "[ or, { _: i }, [ "
            "[ a, { _: g }, [] ], "
            "[ b, { _: h }, [] ] "
          "] ], "
          "[ c, { _: j }, [] ] "
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
            "[ a, { _: g }, [] ] "
          "] ]");
      }

      it "rewrites  unless a"
      {
        msg = mrad("unless a");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ unless, {}, [ "
            "[ a, { _: g }, [] ] "
          "] ]");
      }

      it "rewrites  if a > b"
      {
        msg = mrad("if a > b");
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ if, {}, [ "
            "[ >, { _: i }, [ "
              "[ a, { _: g }, [] ], "
              "[ b, { _: h }, [] ] "
            "] ] "
          "] ]");
      }

      it "rewrites  if a > b then c d"
      it "rewrites  if a > b then c d else f g"

      it "rewrites  if a > b \\ c d"
      {
        msg = mrad(
          "if a > b\n"
          "  c d"
        );
        //fdja_putdc(fdja_l(msg, "tree"));

        flon_rewrite_tree(node, msg);

        expect(fdja_tod(fdja_l(msg, "tree")) ===f ""
          "[ if, {}, [ "
            "[ >, { _: i }, [ "
              "[ a, { _: g }, [] ], "
              "[ b, { _: h }, [] ] "
            "] ], "
            "[ c, { _0: d }, [] ] "
          "] ]");
      }

      it "rewrites  c d if a > b"
      it "rewrites  c d unless a > b"
    }
  }
}

