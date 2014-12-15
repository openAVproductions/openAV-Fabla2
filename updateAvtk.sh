#!/bin/bash

# A simple little script to pull latest AVTK into the subtree
git subtree pull --prefix=src/ui/avtk --squash avtk-origin master
