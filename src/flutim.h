
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

// https://github.com/flon-io/flutil

// flutim.h

#ifndef FLON_FLUTIM_H
#define FLON_FLUTIM_H

#include <time.h>


/* Returns the seconds/nanoseconds since the Epoch.
 */
struct timespec *flu_now();

/* Returns the count of seconds since the Epoch.
 * If level is set to 'm', it will return milliseconds.
 * If level is set to 'u', it will return microseconds.
 * If level is set to 'n', it will return nanoseconds.
 */
long long flu_gets(char level);

/* Sleeps for a given amount of milliseconds.
 * Returns how many milliseconds still have to be slepts (interrupted).
 */
long long flu_msleep(long long milliseconds);

/* Sleeps for a given amount of milliseconds.
 * If interrupted, sleeps again until the required milliseconds have all been
 * slept through. Returns the how many milliseconds it actually slept.
 */
long long flu_do_msleep(long long milliseconds);

/* Formats the given time into a string.
 *
 * 'z' --> "2014-11-01T16:34:01Z"
 * 'h' --> "20141101.1634"
 * 's' --> "20141101.163401"
 * 'm' --> "20141101.163401.001"  // milliseconds
 * 'u' --> "20141101.163401.000001"  // microseconds
 * 'n' --> "20141101.163401.000000001"  // nanoseconds
 *
 * 'r' --> "Fri, 30 Oct 2014 16:34:01 UTC"
 * 'g' --> "Fri, 30 Oct 2014 16:34:01 GMT"
 *
 * If the tm arg is NULL, the function will grab the time thanks to
 * clock_gettime(CLOCK_REALTIME, &ts).
 *
 * Warning, 'g' and 'r' set the locale LC_TIME to en_US temporarily.
 * Not thread-safe.
 */
char *flu_tstamp(struct timespec *ts, int utc, char format);

/* Parses a timestamp, takes a utc hint.
 *
 * /!\ not thread-safe, sets and resets the "TZ" env variable /!\
 */
struct timespec *flu_parse_tstamp(char *s, int utc);

/* Does t1 - t0, over seconds and nanoseconds.
 */
struct timespec *flu_tdiff(struct timespec *t1, struct timespec *t0);

/* Use to print the output of flu_tdiff().
 */
char *flu_ts_to_s(struct timespec *ts, char format);

/* Given a string like "10h55s" returns a timespec instance.
 * Returns NULL when it fails to parse.
 */
struct timespec *flu_parse_ts(const char *s);

/* Like flu_parse_ts() but returns seconds (not a full timespec, so no
 * nanoseconds).
 * When it cannot parse, it sets errno to EINVAL and returns 0.
 */
long long flu_parse_t(const char *s);

#endif // FLON_FLUTIM_H

