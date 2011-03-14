import Options
import os
import re
from os.path import exists 

srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('node_addon')
    
    if not conf.find_program('pkg-config') :
        conf.fatal('pkg-config not found')
    
    if os.system('pkg-config --exists libssh') != 0 :
        conf.fatal('libssh pkg-config package (libssh.pc) not found')

def build(bld):
    obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    obj.target = 'ssh'
    obj.source = [ 'ssh.cc' ]
    obj.cxxflags = [ '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE' ]
    
    obj.cxxflags.append(os.popen(
        'pkg-config --cflags libssh'
    ).readline().strip())
    
    obj.cxxflags.append(os.popen(
        'pkg-config --libs libssh'
    ).readline().strip())

def shutdown():
    if Options.commands['clean']:
        if exists('ssh.node'): os.unlink('ssh.node')
    else:
        if exists('build/default/ssh.node') and not exists('ssh.node'):
            os.symlink('build/default/ssh.node', 'ssh.node')
