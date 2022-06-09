# ex2_sdr
Software Defined Radio software for Ex-Alta2 on-board computer and AlbertaSat ground station.

## Build and Test on Linux

[Meson](https://mesonbuild.com/index.html) is use to manage build and test functions. The main steps are

```
meson setup build
cd build
ninja
meson test
```

All tests should pass, but the `cc_hd_FEC` may fail from time to time because
of the bit error rate (BER) tests, which push the boundary for SNR as part of
the test suite. That means that for a given test run, the noise statistics may
cause the SNR to be lower than the assumed mean SNR by enough to cause a few
extra bit errors that just push the results into a fail. Re-running that test a
a few times should be more successful, showing the FEC method works.

A useful option is to use valgrind to look for memory leaks. For example, the
cc_hd_FEC test can be checked using

```
meson test --wrap='valgrind -s --leak-check=full --show-leak-kinds=all  --undef-value-errors=no --error-exitcode=1' cc_hd_FEC
```

Likewise, total memory usage can be investigated using
```
meson test --wrap='valgrind --tool=massif' cc_hd_FEC
```

see [valgrind man pages](https://valgrind.org/docs/manual/ms-manual.html) for more information on using massif.


## Build and Test for Hercules

## GNURadio 

/gnuradio contains both flow graphs for deployment and test, along with various test scripts and binary data files for development work. Everything is designed for GNURadio 3.9.4 and the Out of Tree module gr-satellites from Daniel Estevez.