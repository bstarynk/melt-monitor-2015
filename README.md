
# MELT monitor

This *MELT monitor* is an alpha stage *experimental* GPLv3 software (hosted on
[github](http://github.com/bstarynk/melt-monitor-2015))
by [Basile Starynkevitch](http://starynkevitch.net/Basile/), from France.

It is conceptually related to [GCC MELT](http://gcc-melt.org/), a
Lispy like domain specific language to customize the
[GCC](http://gcc.gnu.org/) compiler, implemented mostly by a GPLv3
meta-plugin (the *MELT plugin* for GCC) that I also developed and that
is FSF copyrighted, like GCC is. Read
[GCC MELT sheet](http://gcc-melt.org/gcc-melt-sheet.pdf) (two pages)
for more (and,
[this French page](http://gcc-melt.org/gcc-melt-feuillet.pdf) (single
page, in French)....)


## The MELT system

The *MELT system* (still partially a dream) refers to a mix of this
*MELT monitor* and [GCC MELT](http://gcc-melt.org/) customized
compilations (including some future *MELT* extensions for *GCC* not
yet developed; these extensions will communicate with this *MELT
monitor* using some textual ad-hoc protocol on Unix sockets). The
long-time goal of the *MELT system* is to provide a software
development environment and static analysis tool for code compiled by
GCC (putative free-software equivalent of non-sound static code
analysis tools like Coverity, ParaSoft, etc...) and be able to help
exploring and understanding and navigating source code of many
software components at once (in our wildest dreams, all the free
software packaged in a Debian distribution and compiled with GCC, and
some large proprietary software provided by funding partners).

You can use the
[gcc-melt GoogleGroup](https://groups.google.com/forum/#!forum/gcc-melt)
to discuss technical matters related to the *MELT system* (so both
*MELT plugin* and *MELT monitor*)

## Goals of the *MELT monitor*

The goals of the *MELT monitor* includes notably

+ provide a web interface (HTML5 compliant, for recent Firefox) to the
*MELT system* (it took years for me to admit that people want a web
interface, or at least a GUI one, and that most software developers
-notably those working in industry- don't use the command line
anymore, like I am used to do....). And with a Web interface, even
non-Linux users could use it (although the *MELT system* targets only
Linux)... That interface would also be useful to *browse* (but not
edit!) a large collection of 

+ Provide a web interface to edit some *abstract syntax tree* (ASTs)
  of some future DSL, to search into *MELT system* & *GCC* compiled
  software, and give him the ability to document things in a *Wiki*
  way. We don't want the user to type some DSL syntax (like the Lispy
  *MELT DSL* inside our *MELT plugin*) as text, we want to assist him
  in constructing ASTs; Old 1980s
  [MENTOR](https://hal.inria.fr/inria-00076535) system from INRIA (or
  some Smalltalk systems) is inspirational.

+ provide a persistent machinery to the *MELT system*, that is a
  long-term store usable by MELT extensions to keep, enhance, and
  provide static analysis information. In other words, we need a
  cluster of MELT monitors which checkpoints periodically its state
  and which we can later restart smoothly from that persistent state:
  *conceptually* the *MELT system* should run during an entire
  software project using it (like your version control system does),
  because it is part of the entreprise know-how. Hence,
  [dynamic software updating](https://en.wikipedia.org/wiki/Dynamic_software_updating)
  of the *MELT system* is a must.

+ provide a high-level declarative language framework for static
  analysis, above the *MELT plugin*, and for the *MELT system* ; we
  want some high-level rule based language (e.g. to express coding
  rules) ...

+ be able to take advantage of multi-core systems by using several
threads and work in parallel

+ if possible, take advantage and be able to run on a small cluster or
  cloud (of e.g. a few dozens of Linux servers), so *MELT monitors*
  should be able to run concurrently and communicate ...

Not all of them are implemented in 2015. My roadmap includes, like I
did in the *MELT plugin*, development of bootstrapped specific
languages with a metaprogramming (and metaknowledge, inspired by
[J.Pitrat's work and vision](http://bootstrappingartificialintelligence.fr/)
approach). The *MELT monitor* will have some DSL and more and more of
its code will become self-generated.


## Current state

(I am advancing very slowly; things are difficult; this needs a Linux
system to be run and used)

### External dependencies

The following software components are needed (or are very likely to be
needed later, I am listing -for reference- also some packages that are
very likely to be useful in the future and that I have looked
into). When packaged in Debian(/Sid), I just name the Debian
package. See the `Makefile`

+ a recent *GCC* compiler (so `gcc-5` Debian package), in particular
  because even when running both *MELT plugin* and *MELT monitor* are
  generating code.

+ the [GCCJIT](http://gcc.gnu.org/onlinedocs/jit/) library, so
  `libgccjit-5-dev` Debian package

+ as for the *MELT plugin*, all the source dependencies of the latest *GCC*
(so `aptitude build-dep gcc-5`)

+ the *MELT plugin* itself

+ [Boehm's garbage collector](http://www.hboehm.info/gc/) thru
`libgc-dev` Debian package; I am aware of GC techniques, but coding a
multi-threaded GC is too much work (I did spend months debugging the
single-thread GC in the *MELT plugin*, and before that I coded
[Qish](http://starynkevitch.net/Basile/qishintro.html) and some
proprietary GC inside *Fluctuat* so I consider myself experimented in
[garbage collection](http://gchandbook.org/) techniques.). Ravenbrook
[Memory Pool System](http://www.ravenbrook.com/project/mps/) might be
considered as an alternative, at least after bootstrapping is
completed.

+ the `pkg-config` utility (and `pkg-config` Debian package)

+ The [Glib](http://developer.gnome.org/glib/stable/) from GTK is a
little bit used. so `libglib2.0-dev` Debian package.

+ [sqlite](http://sqlite.org/) might be used (for persistency), so `libsqlite3-dev`
Debian package.

+ [codemirror](http://codemirror.net/) is embedded under `webroot/`,
  to nicely show some C code in a web page.

+ [redis](http://redis.io/) might be used (for persistency), so `redis-server`,
`redis-tools`, `libhiredis-dev` Debian packages

+ [PostGreSQL](http://postgresql.org/) might be used (for
  persistency), so probably `libpq-dev`

+ [0mq](http://zeromq.org/) might and probably will be used for
  messaging purposes, so probably `libzmq5-dev` or `libzmq3-dev` or
  `libzmq-dev` Debian package.

+ [jansson](http://www.digip.org/jansson/) library for
  [JSON](http://json.org/) is needed (notably for AJAX), so
  `libjansson-dev` Debian package.

+ [libonion](http://www.coralbits.com/libonion/) is an HTTP server
  library that is absolutely required, but it is not packaged in
  Debian. So compile its latest source from
  [David Moreno `onion` github](https://github.com/davidmoreno/onion);
  I did contribute some small patches to it (sometimes, when I need a
  patch not yet accepted in `libonion` by David Moreno, you'll need to
  compile my github
  [fork of onion](https://github.com/bstarynk/onion)...).

+ I am using some HTML5 things (e.g. JQuery, etc...) but I am copying
them under my `webroot/` subdirectory, which also contains some of my
own code in my own source files (or generated files) there.

### General features

This list only things that are (at least partly) implemented.

+ A dynamic typing framework (semantically similar to what Scheme or
Javascript have). Most values are immutable (boxed strings, boxed
numbers, sets or tuples of items, nodes), but *items* are mutable
values (similar to "objects" or "structures" in Scheme or
Javascript). We should later make that reflective, and generate all
the C code related to that.

+ A node value has an item connective and a sequence of son values (a
  bit like Prolog terms). It might have some fixed metadata (a
  metaitem and a metarank), which is ignored for comparison, equality,
  hashing. It is immutable.

+ Items are externally identified by unique item names (internally,
  they are identified by their address). An item name has a radix
  (C-identifier like, but double or initial or terminal underscores
  are forbidden) and an optional suffix preceded with two
  underscores. For example, `the_agenda` or `comment` is an item name
  (without suffix). And `web_state__0dmo5BBWK0mbtv` is an item name of
  radix `web_state` and suffix `0dmo5BBWK0mbtv` (this is an 80 bit
  positive random number -probably world-wide unique- nearly
  represented in base 60). Notice that item names are on purpose
  acceptable identifiers in generated C and in generated
  Javascript. Items have attributes (a map or "dictionnary" of some
  item associated to some non-nil value) and may have some unique
  payload owned by the item. Each item has its mutex.

+ A persistent machinery, so the entire state of the *MELT monitor*,
  including its agenda, can be checkpointed and restarted on disk
  (persisted preferably in textual form for MELT monitor data,
  friendly to `git` version control, and for user data such as static
  analysis information, in REDIS store or PostGreSQL databases). That
  code should also be generated but it is not yet.

+ An agenda mechanism: `the_agenda` predefined item contains as its
  payload a queue of tasklet items. A fixed number (typically 3 to 10)
  of working threads are repeatedly fetching some tasklet from the
  agenda and running it. Of course tasklets are added (either in
  front, or at the end) to `the_agenda`'s queue.

+ A very incomplete web interface. I'm struggling learning more HTML5
  techniques, and most of my recent questions on
  [StackOverflow](http://stackoverflow.com/users/841108/basile-starynkevitch)
  are related to this. I'm understanding more and more that
  metaprogramming techniques are practically essential.

### Source files

Here is a description of the source files that I have wrote. Other
files available on
[github](http://github.com/bstarynk/melt-monitor-2015) are required,
but are either copied from elsewhere or generated. Some *generated*
files are mentioned, however. In the long term, most (ideally, all!)
source files should be generated, but I have not yet
[bootstrapped](https://en.wikipedia.org/wiki/Bootstrapping_%28compilers%29)
my MELT monitor (like I mostly did for the *MELT plugin*), but it is a
[major](http://bootstrappingartificialintelligence.fr/WordPress3/2014/06/the-meta-bug-curse-of-the-bootstrap/)
subgoal (notably because that would ease adding new data structures in
the MELT monitor).

+ `meltmoni.h` is the only (hand-written) header; it is including many
other system or third-party libraries header, and some `_mom*.h`
generated headers (in particular, the generated `_mom_predef.h` is
listing predefined items).


+ `mi19937ar.c` is a near-copy of some MIT licensed
  [Mersenne Twister pseudo-random number generator](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html)
  by Makoto Matsumoto and Takuji Nishimura; I modified some function
  names by prefixing them with `momrand` and making them more
  thread-friendly.
  
  + `agenda.c` is the agenda mechanism

+ `primes.c` contain many primes, and `mom_prime_above (int64_t)` and
  `mom_prime_below (int64_t)` functions. Prime numbers are e.g. useful
  as hash table sizes, etc.

+ `canvedit.c`, `webroot/canvedit.js`, `webroot/canvedit.css`,
  `webroot/canbedit.html` is a (probably aborted) tentative of using
  HTML5 canvases for web interface.

+ `global.mom` -and perhaps other `*.mom` files, is the persistent
state in textual form, parsed with some stack machine algorithms.

+ `hashed.c` implements various hash tables, notably hashed sets,
hashed associations, hashed maps, ... payloads.

+ `hweb.c` uses `libonion` to implement the web interface. Files
  (notably some `jquery*.js`) under `webroot/` are statically
  served. Dynamic content (and AJAX) is related to the `web_state` and
  `web_handlers` items.

+ `item.c` has item related code.

+ `main.c` is the main program and utilities.

+ `microedit.c`, `webroot/microedit.css`, `webroot/microedit.html`,
  `webroot/microedit.js` is a micro editor (HTML5 contenteditable!) to
  be able to edit values and use them.

+ `modules/` should contain generated C code.

+ `_mom_predef.h` is generated and contains the predefined items.

+ `state.c` has the persistence machinery (loading and saving state
  notably in `global.mom` textual file). We'll need to persist to some
  [REDIS](http://redis.io) store or some
  [PostgreSQL](http://postgresql.org/) database as soon as we'll be
  running in a distributed context (cloud or cluster).

+ `value.c` handle immutable values

### Running that

It is *alpha-stage* software (and that is the third time I'm coding
some MELT monitor). You won't be able to run it usefully in 2015. But
if you want to help (e.g. to answer some questions I am asking on some
forums) or see what is running, build it, then run `./monimelt --help`
to get some command line help.  Later, try something like `./monimelt
-Drun,web -W localhost.localdomain:8086/` in the terminal and use your
browser on URLs like `http://localhost.localdomain:8086/hackc.html` to
be able to type some C code and run it, or
`http://localhost.localdomain:8086/microedit.html` to try the micro
editor (very alpha, not working yet).

To simply dump the state and test the persistency machinery, try
`./monimelt -d` (perhaps also `-Dload,dump`). Modified files are
backed up (e.g. old `global.mom` is backed up as `global.mom~`, and
likewise for `_mom_predef.h~`)

Killing the monitor with `SIGTERM` might trigger a dump (of the
persistent state). Killing it with `SIGQUIT` or `SIGKILL` won't.

# Help needed

This is currently an informal research work. Please help me to find
funding for it (e.g. collaborative R&D projects where this work could
fit).  Otherwise, I might be asked to stop working on this. I could be
interested in having my *MELT system* working on your proprietary
software source code (with some funding available), as long as all my
own (Basile Starynkevitch's) work remains free software, GPLv3
compatible. Notice that GCC runtime license strongly and legally
discourage compiling *proprietary* code with *proprietary* GCC
extensions.

Alternatively (but unlikely, because it is so specific), contribute
some code to this. But since eventually this will be integrated into
*MELT* -as part of GCC- (with a copyright transfered to FSF) you
should follow the
[Contribute to GCC](https://gcc.gnu.org/contribute.html) in particular
its *demanding* legal prerequisites.

# Contact information

[`basile` at `starynkevitch` dot `net`](mailto:basile@starynkevitch.net)
(the mailbox I read more often) for technical and scientific
discussions or questions.

[`basile` dot `starynkevitch` at `cea` dot `fr`](mailto:basile.starynkevitch@cea.fr)
(the mailbox I read at office hours only) for funding or business
related discussions: collaborative R&D consortiums, industrial
contracts

I ([Basile Starynkevitch](http://starynkevitch.net/Basile/)) am
French, living near Paris, working at
[CEA, LIST](http://www-list.cea.fr), the IT part (nearly a thousand
persons) of [CEA](http://ww.cea.fr/) (a 15000 persons, 4.4 billion
euros/year budget, French government owned applied research agency,
see
[Commissariat a l'Energie Atomique et aux energies alternatives](https://en.wikipedia.org/wiki/Commissariat_%C3%A0_l%27%C3%A9nergie_atomique_et_aux_%C3%A9nergies_alternatives)
wikipage). FWIW, the [Frama-C](http://frama-c.com/),
[Papyrus](http://www.eclipse.org/papyrus),
[Unisim](http://unisim-vp.org/), *Gatel* (and of course this MELT
monitor and the [MELT plugin](http://gcc-melt.org/download.html)...)
tools are developed in the same DILS department (100+ persons) where I
work.
