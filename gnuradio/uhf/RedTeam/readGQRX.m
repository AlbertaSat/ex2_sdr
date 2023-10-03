% Reads a "raw" IQ file generate dby GQRX (running in Linux) and plots the
% spectrum.  Format of raw file is 32-bit float I, Q, ...
%
%
% W. Newhall KB2BRD
 
clear all
 
% User input
filename = '/media/Michael/SAT_rcv_20230801_1809.raw';
Ns = 1024;
Fs_MHz = 2.4;
% Modify limits of plot below if desired.
 
% Read file
fid = fopen(filename, 'r');
s_dat = fread(fid, [2, Ns], 'float32').';
fclose( fid );
 
% Format signal data into complex signal
s = s_dat(:,1) + 1j*s_dat(:,2);
t_us = (0:Ns-1)/Fs_MHz;
 
S_dB = fftshift(20*log10(abs(fft(s))));
f_MHz = (0:(Ns-1))/Ns * Fs_MHz - Fs_MHz/2;
 
figure(1)
plot(f_MHz, S_dB)
xlabel( 'Frequency (MHz)' )
ylabel( 'Normalized Magnitude (dB)' )
xlim( [min(f_MHz), max(f_MHz)] )
ylim( [-30, 50] )
title('Spectrum of Recorded Signal')
grid on
 
figure(2)
plot( t_us, [real(s), imag(s)] );
xlabel( 'Time (us)' )
ylabel( 'Normalized Amplitude' )
%xlim( [min(t_us), max(t_us)] )
xlim( [0,50])
title('Time-Domain Recorded Signal')
grid on
legend('I', 'Q')