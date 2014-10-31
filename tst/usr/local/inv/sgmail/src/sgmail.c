
//
// Copyright (c) 2013-2014, John Mettraux, jmettraux+flon@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Made in Japan.
//

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

//#include "flutil.h"
#include "bocla.h"


int main(int argc, char *argv[])
{
  char *s =
    "api_user=your_sendgrid_username&"
    "api_key=your_sendgrid_password&"
    "to=jmettraux+flon@gmail.com&"
    "toname=JohnFlon&"
    "subject=test0&"
    "text=loreipsumtralala&"
    "from=jmettraux+flon_server@gmail.com";

  fcla_response *res = fcla_post(
    "https://api.sendgrid.com/api/mail.send.json", NULL, s);

  printf("res: %i\n", res->status_code);

  return 0;
}

/*
POST
https://api.sendgrid.com/api/mail.send.json

POST Data
api_user=your_sendgrid_username&api_key=your_sendgrid_password&to[]=destination@example.com&toname[]=Destination&to[]=destination2@example.com&toname[]=Destination2&subject=Example_Subject&text=testingtextbody&from=info@domain.com


POST
https://api.sendgrid.com/api/mail.send.json

POST Data
api_user=your_sendgrid_username&api_key=your_sendgrid_password&to=destination@example.com&toname=Destination&subject=Example_Subject&text=testingtextbody&from=info@domain.com
*/

