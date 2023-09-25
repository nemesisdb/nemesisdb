#!/bin/sh


cdCmd="cd client ; bash --login"

cmd="ssh -t fc@20.108.174.55 '${cdCmd}'"
gnome-terminal -e "${cmd}"

cmd="ssh -t fc@20.77.72.255 '${cdCmd}'"
gnome-terminal -e "${cmd}"

cmd="ssh -t fc@20.254.101.174 '${cdCmd}'"
gnome-terminal -e "${cmd}"

cmd="ssh -t fc@51.104.223.37 '${cdCmd}'"
gnome-terminal -e "${cmd}"

cmd="ssh -t fc@20.0.21.78 '${cdCmd}'"
gnome-terminal -e "${cmd}"

