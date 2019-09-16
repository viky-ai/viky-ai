#!/bin/bash

parse_url() {

	# extract the protocol
	proto="$(echo $1 | grep :// | sed -e's,^\(.*://\).*,\1,g')"
	# remove the protocol
	url="$(echo ${1/$proto/})"
	# extract the user (if any)
	user="$(echo $url | grep @ | cut -d@ -f1)"
	# extract the host
	host="$(echo ${url/$user@/} | cut -d/ -f1)"

	echo "$host"
}
