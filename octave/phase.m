% phase.m
% David Rowe August 2009
% experiments with phase for sinusoidal codecs

function phase(samname, F0, randphase, png)
  Wo=2*pi*F0/8000;
  P=2*pi/Wo;
  L = floor(pi/Wo);
  N = 16000;
  A = 10000/L;
  phi = zeros(1,L);
  for m=1:L
    phi(m) = m*Wo*50.125;
  end
  if (randphase == 1) 
    rand("seed",0);
    phi = rand(L,1)*2*pi;
  endif
  s = zeros(1,N);

  for m=1:L
    s = s + A*cos(m*Wo*(0:(N-1)) + phi(m));
  endfor

  figure(1);
  clg;
  plot(s(1:250));

  fs=fopen(samname,"wb");
  fwrite(fs,s,"short");
  fclose(fs);

  if png
      % small image to fit blog

      __gnuplot_set__ terminal png size 450,300
      ss = sprintf("__gnuplot_set__ output \"%s.png\"", samname);
      eval(ss)
      replot;

      % for some reason I need this to stop large plot getting wiped
      __gnuplot_set__ output "/dev/null"
  endif

endfunction

