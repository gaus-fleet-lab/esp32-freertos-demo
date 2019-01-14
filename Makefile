#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := gaus-demo

#Disable certificate check.  DO NOT USE IN PRODUCTION!
CFLAGS=-DGAUS_NO_CA_CHECK

include $(IDF_PATH)/make/project.mk

