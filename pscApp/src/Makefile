TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += psc

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install psc.dbd into <top>/dbd
DBD += psc.dbd

# specify all source files to be compiled and added to the library
psc_SRCS += ethernet.c

psc_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

