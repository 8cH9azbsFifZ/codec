% pulse.m
% David Rowe August 2009
% experiments with phase for sinusoidal codecs
%   - lets see how pulse waveforms are perceived at different F0
%   - experiment suggested by O'Shaughnessy, "Speech Communication", page 145
%   - in low F0 alternative phase pulses are perceived as 2F0

function pulse(samname, F0, alternate)
  Wo = 2*pi*F0/8000;
  P  = 2*pi/Wo
  L  = floor(pi/Wo);
  N  = 16000;
  A  = 1000;
  phi = zeros(1,L);
  if (alternate == 1) 
    % shift phases of some harmonics to introduce -ve pulses
    % note magnitude spectrum unchanged
    phi(2:2:L) = (2:2:L)*pi*(P)*Wo;
  endif
  s = zeros(1,N);

  for m=1:L
    s = s + A*cos(m*Wo*(0:(N-1)) + phi(m));
  endfor

  plot(s(1:250));

  fs=fopen(samname,"wb");
  fwrite(fs,s,"short");
  fclose(fs);
endfunction

