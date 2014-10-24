
//
// specifying flon-dispatcher
//
// Fri Oct 24 12:45:24 JST 2014
//

//#include "flutil.h"
//#include "gajeta.h"
//#include "fl_ids.h"
//#include "fl_common.h"
//#include "fl_dispatcher.h"


context "instruction:"
{
  before all
  {
    dispatcher_start();
  }
  after all
  {
    dispatcher_stop();
  }

  describe "sequence"
  {
    it "flips burgers"
    {
      puts("flipping burgers...");
    }
  }
}

