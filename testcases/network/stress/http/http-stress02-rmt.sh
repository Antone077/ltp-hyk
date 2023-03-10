#!/bin/sh

# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TCID="http-stress02-rmt"
TST_TOTAL=1

. test.sh

tst_require_cmds awk curl

if [ $# -ne 5 ]; then
	tst_brkm TBROK "Usage: $0 server_addr fname fsize duration connections"
fi

server_ipaddr="$1"
filename="$2"
filesize="$3"
duration="$4"
connections="$5"

echo $server_ipaddr | grep -F ':' > /dev/null
if [ $? -eq 0 ]; then
	server_ipaddr='['$server_ipaddr']'
fi

start_epoc=$(date +%s)
while true ; do
	# Exit when the specified seconds have passed.
	current_epoc=$(date +%s)
	elapse_epoc=$(($current_epoc - $start_epoc))

	[ $elapse_epoc -ge $duration ] && break

	# Do not request the file over the specified quantity
	http_num=$(jobs | wc -l)
	if [ $http_num -ge $connections ]; then
		sleep 1
		continue;
	fi

	curl --noproxy '*' -s -g "http://$server_ipaddr/$filename" \
		-o /dev/null &
done

killall -qw curl

out=$(curl --noproxy '*' -sS -g "http://$server_ipaddr/$filename" -o /dev/null \
	-w "time=%{time_total} size=%{size_download} speed=%{speed_download}")

tst_resm TINFO "stat: $out"
recv_filesize=$(echo "$out" | awk '{print $2}')

if [ $recv_filesize != "size=$filesize" ]; then
	tst_resm TINFO "expected file size '$filesize'"
	tst_brkm TBROK "Failed to get http://$server_ipaddr/$filename"
fi

tst_exit
