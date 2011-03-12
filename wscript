import Options
from os import unlink, symlink
from os.path import exists 

srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('node_addon')
    conf.env.append_value('LINKFLAGS', ['-lssh']);

def build(bld):
    obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    obj.target = 'ssh'
    obj.source = [ 'ssh.cc' ]
    obj.cxxflags = ['-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE']

def shutdown():
    if Options.commands['clean']:
        if exists('ssh.node'): unlink('ssh.node')
    else:
        if exists('build/default/ssh.node') and not exists('ssh.node'):
            symlink('build/default/ssh.node', 'ssh.node')

