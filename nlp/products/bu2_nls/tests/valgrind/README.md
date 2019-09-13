Prerequisites
=============

install Valgrind massif and memcheck
`sudo apt-get install valgrind`

install massif visualizer
`sudo apt-get install massif-visualizer`

install kcachegrind for callgrind
`sudo apt-get install kcachegrind`

Massif
======

launch the command
`bundle exec rake test_massif`

a graph opens
if the slope is not null, launch the memory check with memcheck
see section below

Memcheck
========

launch the command
`bundle exec rake test_memcheck`

callgrind
=========

launch the command
`bundle exec rake test_callgrind`
