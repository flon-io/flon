
//
// specifying flon-invoker
//
// Fri Oct  3 14:21:21 JST 2014
//

#include <errno.h>

#include "flutil.h"
#include "fl_common.h"


context "common"
{
  before each
  {
    flon_configure_j(fdja_c(
      "unit: {\n"
      "  id: u96\n"
      "  group_id: g7\n"
      "}\n"
      "\n"
      "invoker: {\n"
      "  max_processes: 2\n"
      "  xyz: nada\n"
      "}\n"
      "executor: {\n"
      "  p0: ../tst\n"
      "  p1: /var\n"
      "}\n"
    ));
  }

  describe "flon_conf()"
  {
    it "returns a fdja_value"
    {
      expect(flon_conf("invoker.max_processes")->type == 'n');
    }

    it "returns NULL when not found"
    {
      expect(flon_conf("nada") == NULL);
    }
  }

  describe "flon_conf_path()"
  {
    it "returns an absolute path"
    {
      expect(flon_conf_path("executor.p9", "x") $==f "/tmp/x");
      expect(flon_conf_path("executor.p9", "/x") === "/x");
      expect(flon_conf_path("executor.p9", NULL) === NULL);

      expect(flon_conf_path("executor.p0", NULL) $==f "/flon/tst");
      expect(flon_conf_path("executor.p1", NULL) ===f "/var");
    }
  }

  describe "flon_configure()"
  {
    it "parses from {root}/etc/flon.json"
    {
      flon_configure("../tst");

      expect(flon_conf_path("_root", NULL) $==f "/flon/tst");
    }
  }

  describe "flon_generate_id()"
  {
    it "returns an id"
    {
      char *id = flon_generate_id();

      //printf("id: >%s<\n", id);
      expect(id != NULL);
      expect(id ^== "u96_g7_");

      free(id);
    }
  }

  describe "flon_isdir()"
  {
    it "returns 1 if the path points to a dir"
    {
      expect(flon_isdir("../src") == 1);
    }
    it "returns 0 if the path points to something that is not a dir"
    {
      expect(flon_isdir("../src/flutil.c") == 0);
      expect(errno == 0);
    }
    it "returns 0 if the path points to something that is not present"
    {
      expect(flon_isdir("nada") == 0);
      expect(errno == 2);
    }
  }

  describe "flon_basename()"
  {
    it "returns the basename given a path"
    {
      expect(flon_basename("/x/y/z.txt", NULL) ===f "z.txt");
    }

    it "accepts a .suffix for the returned basename"
    {
      expect(flon_basename("/x/y/z.json", ".txt") ===f "z.txt");
    }
  }

  describe "flon_move()"
  {
    it "moves a file to a dir"
    {
      expect(flu_writeall("a.txt", "alright") == 1);

      flon_move("./a.txt", "../tst/var/spool/rejected/");

      expect(flu_readall("../tst/var/spool/rejected/a.txt") ===f "alright");
      expect(unlink("../tst/var/spool/rejected/a.txt") == 0);
    }

    it "renames a file"
    {
      expect(flu_writeall("a.txt", "alright") == 1);
    }
  }
}

