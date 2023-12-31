
# Welcome to Defence-Force ~~SVN~~ GIT repository

## Please behave

If you don't know anything about ~~Subversion~~ GIT, you can learn more about it here:

<!--
<ul>
<li><a href="http://subversion.tigris.org">The Subversion website</a></li>
<li><a href="http://svnbook.red-bean.com">The official SubVersion book</a></li>
</ul>

If you want to install a subversion client, here is a selection for the various OSes:
<ul>
<li>Windows: <a href="http://tortoisesvn.tigris.org/">Tortoise SVN</a> (<a href="http://tortoisesvn.net/downloads">binaries</a>)</li>
<li>BeOS: <a href="http://bebits.com/app/3962">SVN binaries</a></li>
<li>Haiku: <code>installoptionalpackage -a subversion</code></li>
<li>GNU/Linux:</li>
<ul>
<li>Debian-based, Ubuntu: <a href="apt:subversion">subversion package</a>, or <code>apt-get install subversion</code></li>
<li>RPM-based (RedHat, Mandriva, ...): <code>rpm -ivh subversion</code></li>
</ul>
<li>MacOS: <a href="http://subversion.tigris.org/">Subversion</a> (<a href="http://subversion.tigris.org/getting.html#osx">binaries</a>), <a href="http://scplugin.tigris.org/">SCPlugin</a> (<a href="http://scplugin.tigris.org/servlets/ProjectDocumentList">binaries</a>, <a href="http://scplugin.tigris.org/servlets/ProjectProcess?pageID=rgnEkt">installation</a>)</li>
</ul>

If you want to check out the repository with the command-line client:<br>
<code>svn co http://miniserve.defence-force.org/svn</code>
<br>
<br>

If you want to be added as a user so you can modify, please <a href="mailto:webmaster@defence-force.org">contact the webmaster</a>.
<br>
-->

Here is the structure of the whole repository at the moment:

<pre>
\---pc
    +---shared_libraries
    |   +---freeimage
    |   \---unittestcpp
    +---euphoric_tools
    |   +---4k8
    |   +---Bin2Tap
    |   +---dsk
    |   +---old2mfm
    |   +---tap2dsk
    |   \---txt2bas
    \---osdk
        +---main
            +---bas2tap
            +---bin2txt
            +---common
            +---compiler
            +---DskTool
            +---filepack
            +---header
            +---link65
            +---macrosplitter
            +---makedisk
            +---MemMap
            +---old2mfm
            +---opt65
            +---pictconv
            +---SampleTweaker
            +---tap2cd
            +---tap2dsk
            +---TapTool
            +---xa
            \---Ym2Mym
