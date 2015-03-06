
//
// specifying flon
//
// Fri Mar  6 13:38:53 JST 2015
//

#include "fl_ids.h"
#include "feu_helpers.h"


context "bin/flon"
{
  before each
  {
    char *s = NULL;
    fdja_value *v = NULL;
  }
  after each
  {
    free(s);
    fdja_free(v);
  }

  describe "launch"
  {
    it "places a launch msg in var/spool/dis/"
    {
      flu_writeall("l.flon",
        "# domain\n"
        "z.test.flon.launch\n"
        "\n"
        "# tree\n"
        "sequence\n"
        "  trace '$(type) - $(brand) - $(v.category)'\n"
        "\n"
        "# payload\n"
        "type: car\n"
        "brand: renault\n"
        "\n"
        "# vars\n"
        "category: family\n"
        "\n"
      );

      s = flu_pline("../tst/bin/flon launch l.flon");
      expect(s != NULL);

      flu_system("rm -f l.flon");

      v = fdja_parse_f("../tst/var/spool/dis/exe_%s.json", s);

      //fdja_putdc(v);
      expect(fdja_ls(v, "point", NULL) ===f "execute");
      expect(fdja_ls(v, "exid", NULL) ===f s);
      expect(fdja_ld(v, "payload", NULL) ===f "{ type: car, brand: renault }");
      expect(fdja_ld(v, "vars", NULL) ===f "{ category: family }");
    }
  }
}

