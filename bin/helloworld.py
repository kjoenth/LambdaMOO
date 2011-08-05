#!/usr/bin/python
import sys

def main():
	print 'hello world 1'
	print 'hello world 2'
	print 'hello world 3'
	print 'hello world 4'

if __name__ == '__main__':
    theArguments=sys.argv
    theObject=theArguments[1]
    theVerb=theArguments[2]
    theArguments=theArguments[3:]
    print theObject
    print theVerb
    print theArguments
    theObject()