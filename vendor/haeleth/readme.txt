http://dev.haeleth.net/xkanon.shtml

xkanon

 
Site

Home 
Development 
Links 
Development

Home 
Subversion 
Projects

ONScripter 
RLdev 
Kpac 
Majiro stuff 
xclannad patches 
xkanon patches 
Kurokoge 
xkanon

This is a support page for users of Jagarl's xkanon interpreter with English games.

The stock xkanon interpreter has a number of minor incompatibilities with English text: Arkazam fixed most of those. In addition to this, it does not implement (or implements incompatibly) a number of AVG32 features which I have used in Kanon, but which are not used by many commercial titles: I have implemented and fixed most of these in turn.

The interpreter available from this page is my own fork of xkanon. It is based on Jagarl's official 2005/5/15 release, merged with an unreleased version of the interpreter sources from Arkazam's Xkanon-bundle. This fork is the only version of xkanon which will be formally supported for use with my Kanon translation.

Since this version of xkanon has been modified from the original source code, please send bug reports to me in the first instance, not to Jagarl, unless you are absolutely certain that the problem was not introduced by something I've done.

xkanon for English games

Development sources

The current development version is available via anonymous Subversion from http://svn.haeleth.net/pub/xkanon.

Tarball of the above

412 kb ¡ª 2005.12.1 ¡ª version b21p-current

This is the complete xkanon source code, prepatched with my modifications to support "Kanon: a translation". English documentation is included: see README.en.

This tarball is not automatically updated to reflect work in progress. Only use this if for some reason you cannot access the Subversion repository.

Diff against the main xkanon source

The stable version, in the form of a patch to Jagarl's 2005/5/15 release.
