# XV6-RISCV On K210
Run xv6-riscv on k210 board  
[English](../README.md) | [中文](./README_cn.md)     

## See [Tasks](https://github.com/abrasumente233/osterm)

## Different from [xv6-k210](..)
+ For `/proc`  file system
  + add another version of implementation of `ls` command. 
  + By reading the dirent information in the ecache in the /proc directory one by one, the process directory information is passed back to the user mode for printing.
    + add function `procfs_enext`  in [fat32.c](./kernel/fat32.c) and modify function `dirnext` in [file.c](./kernel/file.c)

+ For `ps` command
  + add  another version of implementation of `ps` command.
  + implement the `ps` command based on the `/proc` file system implementation above.
  + see [pstest](./xv6-user/pstest.c)

