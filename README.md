
# MELT monitor

This *MELT monitor* is an alpha stage *experimental* GPLv3 software (hosted on
[github](http://github.com/bstarynk/melt-monitor-2015))
by [Basile Starynkevitch](http://starynkevitch.net/Basile/), from France.

It is conceptually related to [GCC MELT](http://gcc-melt.org/), a
Lispy like domain specific language to customize the
[GCC](http://gcc.gnu.org/) compiler, implemented mostly by a GPLv3
meta-plugin (the *MELT plugin* for GCC) that I also developed and that
is FSF copyrighted, like GCC is.

The *MELT system* refers to a mix of this *MELT monitor* and
[GCC MELT](http://gcc-melt.org/) customized compilations (including
some future *MELT* extensions for *GCC* not yet developed; these
extensions will communicate with this *MELT monitor* using some
textual ad-hoc protocol on Unix sockets). The long-time goal of the
*MELT system* is to provide a software development environment and
static analysis tool for code compiled by GCC (putative free-software
equivalent of non-sound static code analysis tools like Coverity,
ParaSoft, etc...).

You can use the
[gcc-melt GoogleGroup](https://groups.google.com/forum/#!forum/gcc-melt)
to discuss technical matters related to the *MELT system* (so both
*MELT plugin* and *MELT monitor*)

## Goals of the *MELT monitor*

The goals of the *MELT monitor* includes notably

+ provide a web interface to the *MELT system* (it took years for me
  to admit that people want a web interface, or at least a GUI one,
  and that most software developers don't use the command line
  anymore, like I am used to do....). And with a Web interface, even
  non-Linux users could use it (although the *MELT system* targets
  only Linux)...

+ provide a persistent machinery to the *MELT system*, that is a
  long-term store usable by MELT extensions to keep static analysis
  information.

+ provide a high-level declarative language framework for static analysis,
  above the *MELT plugin*, and for the *MELT system*

+ be able to take advantage of multi-core systems by using several
  threads and work in parallel

Not all of them are implemented in 2015. My roadmap includes, like I
did in the *MELT plugin*, development of bootstrapped specific
languages with a metaprogramming (and metaknowledge) approach. The
*MELT monitor* will have some DSL and more and more of its code will
become self-generated.


## Current state

(I am advancing very slowly; things are difficult; this needs a Linux
system to be run and used)

### External dependencies

The following software components are needed. When packaged in
Debian(/Sid), I just name the Debian package. See the `Makefile`

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
multi-threaded GC is too much work.

+ the `pkg-config` utility (and `pkg-config` Debian package)

+ The [Glib](http://developer.gnome.org/glib/stable/) from GTK is a
little bit used. so `libglib2.0-dev` Debian package.

+ [sqlite](http://sqlite.org/) might be used, so `libsqlite3-dev`
Debian package.

+ [jansson](http://www.digip.org/jansson/) library for
  [JSON](http://json.org/) is needed, so `libjansson-dev` Debian
  package.

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

This list only things that are partly implemented.

+ A dynamic typing framework (semantically similar to what Scheme or
Javascript have). Most values are immutable (boxed strings, boxed
numbers, sets or tuples of items, nodes), but *items* are mutable
values (similar to "objects" in Scheme or Javascript)

+ A persistent machinery

+ An agenda mechanism

### Source files

Here is a description of the source files that I have wrote. Other
files available on
[github](http://github.com/bstarynk/melt-monitor-2015) are required,
but are either copied from elsewhere or generated. Some *generated*
files are mentioned, however.

+ `meltmoni.h` is the only (hand-written) header; it is including many
other system or third-party libraries header, and some `_mom*.h`
generated headers.


+ `mi19937ar.c` is a near-copy of some MIT licensed
  [Mersenne Twister pseudo-random number generator](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html)
  by Makoto Matsumoto and Takuji Nishimura; I modified some function
  names by prefixing them with `momrand` and making them more
  thread-friendly.
  

# Help needed

This is currently a research project. Please help me to find funding
for it (e.g. collaborative R&D projects where this work could fit).
Otherwise, I might be asked to stop working on this.

Alternatively (but unlikely, because it is so specific), contribute
some code to this. But since eventually this will be integrated into
*MELT* -as part of GCC- (with a copyright transfered to FSF) you
should follow the
[Contribute to GCC](https://gcc.gnu.org/contribute.html) in particular
its *demanding* legal prerequisites.
