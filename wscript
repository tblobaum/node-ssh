import Options
import os, re
from os.path import exists 

srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('node_addon')
    
    libs = re.split(r'\s+', os.popen(
        'pkg-config --libs libssh'
    ).readline().strip())
    conf.env.append_value('LINKFLAGS', libs)
    
    if not conf.find_program('pkg-config') :
        conf.fatal('pkg-config not found')
    
    if os.system('pkg-config --exists libssh') != 0 :
        conf.fatal('libssh pkg-config package (libssh.pc) not found')

def build(bld):
    sshd = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    sshd.target = 'sshd'
    sshd.source = [ 'src/init.cc', 'src/sshd.cc', 'src/client.cc' ]
    sshd.cxxflags = [ '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE' ]
    sshd.cxxflags.append(
        os.popen('pkg-config --cflags libssh').readline().strip()
    )
    
    sshd.uselib = 'SSH'
