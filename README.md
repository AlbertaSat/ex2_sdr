# ex2_obc_sdr
Software Defined Radio software for Ex-Alta2 on-board computer

## Tags

   * v_linux_0.8 : happy path for CSP tx/rx with FEC stubbed
   
## Build source

Check `meson.build` to see which source files are actually included. There are
likely source files that are present but not compiled. This should be cleaned up
when merged with the develop branch.

Check `unit_tests/meson.build` to see which unit test sources are compiled.

## Build and Test on Linux

[Meson](https://mesonbuild.com/index.html) is use to manage build and test functions. The main steps are

```
meson setup build
cd build
ninja
meson test
```

    
## Build and Test for Hercules

foo

