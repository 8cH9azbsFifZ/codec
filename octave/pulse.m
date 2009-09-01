% pulse.m
% David Rowe August 2009
% experiments with human pulse perception for sinusoidal codecs
% lets try some pulses in noise

function pulse(samname, F0, snr)

  N = 16000;
  s = zeros(1,N);
  Wo  = 2*pi*F0/8000;
  L  = floor(pi/Wo);
  A  = 10000;
  phi = zeros(1,L);
  for m=1:L
    s = s + (A/L)*cos(m*Wo*(1:N) + phi(m));
  endfor
  randn("seed", 0);
  s = s + A*(10 .^(-snr/20))*randn(1,N);
  plot(s(1:250));

  fs=fopen(samname,"wb");
  fwrite(fs,s,"short");
  fclose(fs);
endfunction

