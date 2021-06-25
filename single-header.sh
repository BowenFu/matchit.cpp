#!/bin/sh

awk 1 develop/header.txt develop/matchit/*.h develop/footer.txt > include/matchit.h
