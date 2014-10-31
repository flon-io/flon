
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
#include <string.h>
#include <stdlib.h>

//#include "flutil.h"
#include "bocla.h"


int main(int argc, char *argv[])
{
  // TODO: read payload

  char *u = flu_readall("sendgrid.credentials");
  if (u == NULL)
  {
    puts("{\"outcome\":\"couldn't send: couldn't read sengrid.credentials\"}");
    return 1;
  }
  *(strchr(u, '\n')) = '\0'; // trim right
  char *col = strchr(u, ':');
  if (col == NULL)
  {
    puts("{\"outcome\":\"couldn't send: couldn't split sengrid.credentials\"}");
    return 1;
  }
  *col = '\0';
  char *p = col + 1;

  char *s = NULL;
  flu_sbuffer *sb = flu_sbuffer_malloc();

  s = flu_urlencode(u, -1);
  flu_sbprintf(sb, "api_user=%s&", s);
  free(s);

  s = flu_urlencode(p, -1);
  flu_sbprintf(sb, "api_key=%s&", s);
  free(s);

  free(u);

  s = flu_urlencode("jmettraux+flon@gmail.com", -1);
  flu_sbprintf(sb, "to=%s&", s);
  free(s);

  s = flu_urlencode("John Mettraux (flon)", -1);
  flu_sbprintf(sb, "toname=%s&", s);
  free(s);

  s = flu_urlencode("maitre corbeau sur un arbe perche", -1);
  flu_sbprintf(sb, "subject=%s&", s);
  free(s);

  s = flu_urlencode(
    "Maitre corbeau, sur un arbre perche,\n"
    "Tenait en son bec un fromage.\n"
    "Maitre renard, par l'odeur alleche,\n"
    "Lui tint a peu pres ce language :\n"
    "...\n",
    -1);
  flu_sbprintf(sb, "text=%s&", s);
  free(s);

  s = flu_urlencode("jmettraux+flon_sgmail@gmail.com", -1);
  flu_sbprintf(sb, "from=%s", s);
  free(s);

  char *form = flu_sbuffer_to_string(sb);

  fcla_response *res = fcla_post(
    "https://api.sendgrid.com/api/mail.send.json", NULL, form);

  free(form);

  printf("res: %i\n", res->status_code);
  printf("body: >%s<\n", res->body);

  fcla_response_free(res);

  puts("{\"outcome\":\"ok\"}");
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

