#!/bin/sh

g++ -Wall -g onlinelife.cpp -lcurl -o onlinelife `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` #-lpthread
