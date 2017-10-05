Prerequisites
=============

install Valgrind massif and memcheck
`sudo apt-get valgrind`

install massif visualizer
`sudo apt-get massif-visualizer`

Massif
======

launch the command
`rake test_massif`

a graph opens
if the slope is not null, launch the memory check with memcheck
see section below

Memcheck
========

launch the command
`rake test_memcheck`
