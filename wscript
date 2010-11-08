import Options
from os import unlink, symlink, popen

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

  pkg_config = conf.find_program('pkg-config', var='PKG_CONFIG', mandatory=True)

  pixbuf_libs = popen("%s --libs gdk-pixbuf-2.0" % pkg_config).readline().strip().split()
  pixbuf_cflags = popen("%s --cflags gdk-pixbuf-2.0" % pkg_config).readline().strip().split()

  conf.env['CXXFLAGS_PIXBUF'] = pixbuf_cflags
  conf.env['LINKFLAGS_PIXBUF'] = pixbuf_libs

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'pixbuf'
  obj.source = 'Pixbuf.cc'
  obj.uselib = ['PIXBUF']
