Import('RTT_ROOT')
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src = Glob('qxwz_sdk/*.c')

LIBPATH = [cwd + '/qxwz_lib']
LIBS = ['QXSI_RTCM32_RTK_1.2.2.09c45bdb.20190920_STM32-KEIL']

if GetDepend(['PKG_USING_QXWZ_EXAMPLE']):
    src += Glob('samples/*.c')

path = [cwd + '/qxwz_sdk']

group = DefineGroup('qxwz', src, depend = ['PKG_USING_QXWZ'], CPPPATH = path, LIBS = LIBS, LIBPATH=LIBPATH)

Return('group')
