t = []
foreach i : ['bb', 'empty', 'insn', 'mem', 'syscall', 'memtrace']
  t += shared_module(i, files(i + '.c'),
                     include_directories: ['../../include/qemu','../../include/starfive'],
                     dependencies: glib)
endforeach
alias_target('test-plugins', t)
